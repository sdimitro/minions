from collections import defaultdict
import itertools

from sklearn import linear_model
import numpy as np

#### Helper Functions

def get_num_timestamps(readings):
    return len(readings[0])

def irange(start, end, step=1):
    return range(start, end + step, step)

def generate_timestamps(num_timestamps):
    lowest_timestamp  = 0.0
    highest_timestamp = 23.5
    timestamp_step    = 0.5
    initial_timestamp = 0.5
    timestamps = [t for t in np.arange(lowest_timestamp,
                                       highest_timestamp + timestamp_step,
                                       timestamp_step)]
    start = timestamps.index(initial_timestamp)
    return [timestamps[i % len(timestamps)]
                      for i in irange(start, num_timestamps)]

#### File I/O

def read_csv_data(filename):
    return np.genfromtxt(filename, delimiter=',', skip_header=1)[:,1:]

def generate_header(length):
    header  = "sensors,"
    header += ",".join(map(str, generate_timestamps(length)))
    header += "\n"
    return header

def write_csv_data(filename, data):
    with open(filename, mode='w') as f:
        f.write(generate_header(get_num_timestamps(data)))
        for sensor_id, sensor_data in enumerate(data):
            line = str(sensor_id) + ","
            line += ",".join(map(lambda x: "%.2E" % x, sensor_data))
            line += "\n"
            f.write(line)

def write_mae_data(filename, data):
    lmbd = lambda x: (x + "\t" + str(data[x]))
    with open(filename, mode='w') as f:
        f.write("\n".join(map(lmbd, sorted(data))))
        f.write("\n")

#### Modeling

class RandomVariable(object):

    def __init__(self, samples=None):
        # Screw you Python and your mutable default arguments!
        self.samples = ([] if samples is None else samples)
        self.mean, self.variance = False, False
        self.b_0, self.b_1, self.residual = False, False, False
        self.observed = False

    def add_entry(self, entry):
        self.samples.append(entry)

    def calc_mean(self):
        self.mean = np.mean(self.samples)

    def calc_var(self):
        self.variance = np.var(self.samples)

    def estimate_learning_params(self, current, next):
        regr = linear_model.LinearRegression()
        regr.fit(np.reshape(current, (-1 , 1)), next)
        self.b_1 = regr.coef_[0]
        self.b_0 = regr.intercept_
        self.residual = np.var(next - (self.b_0 + self.b_1 * current))

    def observe(self, observation):
        self.mean, self.var = observation, 0
        self.samples = []
        self.observed = True

    def data_(self):
        return [self.mean, self.variance,
                self.b_1, self.b_0, self.residual]

    def all_data_(self):
        return [self.mean, self.variance,
                self.samples, len(self.samples),
                self.b_0, self.b_1, self.residual]

class SensorModel(object):

    def get_num_sensors(self, readings):
        return len(readings)
    
    def get_num_timestamps(self, readings):
        return len(readings[0])

    def infer(self, data, whitelists, func):
        transposed_data = data.T
        transposed_res = np.zeros_like(transposed_data)
        for i, sensor_data in enumerate(transposed_data):
            transposed_res[i] = func(sensor_data, whitelists[i])
        return transposed_res.T

    def to_be_observed(self, sensor_id, whitelist):
        return sensor_id in whitelist


class HourLevelModel(SensorModel):

    def estimate_sensor_params(self, sensor_readings):
        rv = RandomVariable(sensor_readings)
        rv.calc_mean()
        rv.calc_var()
        prev = np.roll(sensor_readings, 1)
        next = sensor_readings
        rv.estimate_learning_params(prev, next)
        return rv.data_()

    def infer_timestamp(self, sensor_data, whitelist):
        num_sensors = self.get_num_sensors(sensor_data)
        res = np.zeros(num_sensors)
        for sensor in range(num_sensors):
            if self.to_be_observed(sensor, whitelist):
                res[sensor] = sensor_data[sensor]
                self.prev_state[sensor] = [res[sensor], 0]
            elif not self.prev_state[sensor]:
                res[sensor] = self.sensor_params[sensor][0]
                self.prev_state[sensor] = [res[sensor],
                                           self.sensor_params[sensor][1]]
            else:
                b_0 = self.sensor_params[sensor][2]
                b_1 = self.sensor_params[sensor][3]
                sigma_sq = self.sensor_params[sensor][4]

                prev_mean = self.prev_state[sensor][0]
                prev_sigma_sq = self.prev_state[sensor][1]

                mean = b_0 + b_1 * prev_mean
                var = sigma_sq + (b_1 ** 2) * prev_sigma_sq

                res[sensor] = mean
                self.prev_state[sensor] = [mean, var]
        return res

    def train(self, train_data):
        self.sensor_params = {sensor_id: self.estimate_sensor_params(readings)
                              for sensor_id, readings in enumerate(train_data)}

    def infer_window(self, test_data, budget):
        self.prev_state = defaultdict(lambda: False)
        num_sensors = self.get_num_sensors(test_data)
        num_timestamps = self.get_num_timestamps(test_data)
        a = ActiveInferenceWindow()
        windows = a.generate_windows(num_sensors, budget, num_timestamps)
        return self.infer(test_data, windows, self.infer_timestamp)

    def infer_var(self, test_data, budget):
        self.prev_state = defaultdict(lambda: False)
        transposed_data = test_data.T
        transposed_res = np.zeros_like(transposed_data)
        temp_hsh = {k: v[1] for k, v in self.sensor_params.items()}
        whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]
        for i, sensor_data in enumerate(transposed_data):
            transposed_res[i] = self.infer_timestamp(sensor_data, whitelist)
            temp_hsh = {k: v[1] for k, v in self.prev_state.items()}
            whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]
        return transposed_res.T


class DayLevelModel(HourLevelModel):

    def model_index(self, sensor_id, timestamp):
        return "s%d-%.1f" % (sensor_id, timestamp)

    def rv_mean(self, index):
        return self.model[index].mean

    def rv_var(self, index):
        return self.model[index].variance

    def rv_add_entry(self, index, entry):
        self.model[index].add_entry(entry)

    def rv_calc_mean(self, index):
        self.model[index].calc_mean()

    def rv_calc_var(self, index):
        self.model[index].calc_var()

    def rv_calc_learning_params(self, index, next_index):
        prev = np.array(self.model[index].samples)
        next = np.array(self.model[next_index].samples)
        self.model[index].estimate_learning_params(prev, next)

    def populate_sensor_rvs(self, sensor_id, sensor_data, timestamps):
        for i, entry in enumerate(sensor_data):
            rv_index = self.model_index(sensor_id, timestamps[i])
            self.rv_add_entry(rv_index, entry)

        for timestamp in np.arange(0.0, 24.0, 0.5):
            next_timestamp = (timestamp + 0.5) % 24

            rv_index = self.model_index(sensor_id, timestamp)
            next_rv_index = self.model_index(sensor_id, next_timestamp)

            self.rv_calc_mean(rv_index)
            self.rv_calc_var(rv_index)
            self.rv_calc_learning_params(rv_index, next_rv_index)

    def train(self, train_data):
        # [mean, variance, entries, num_entries, b_0, b_1, residual]
        # self.model = defaultdict(lambda: [0, 0, [], 0, 0, 0, 0])
        self.model = defaultdict(RandomVariable)
        timestamps = generate_timestamps(self.get_num_timestamps(train_data))
        for sensor_id, sensor_readings in enumerate(train_data):
            self.populate_sensor_rvs(sensor_id, sensor_readings, timestamps)
        self.model = {k: v.all_data_() for k, v in self.model.items()}


    def infer_timestamp(self, sensor_data, timestamp, whitelist):
        num_sensors = self.get_num_sensors(sensor_data)
        res = np.zeros(num_sensors)
        for sensor in range(num_sensors):
            index = self.model_index(sensor, timestamp)

            if self.to_be_observed(sensor, whitelist):
                res[sensor] = sensor_data[sensor]
                self.prev_state[sensor] = [res[sensor], 0]

            elif not self.prev_state[sensor]:
                res[sensor] = self.model[index][0]
                self.prev_state[sensor] = [res[sensor],
                                           self.model[index][1]]
            else:
                b_0 = self.model[index][4]
                b_1 = self.model[index][5]
                sigma_sq = self.model[index][6]

                prev_mean = self.prev_state[sensor][0]
                prev_sigma_sq = self.prev_state[sensor][1]

                mean = b_0 + b_1 * prev_mean
                var = sigma_sq + (b_1 ** 2) * prev_sigma_sq

                res[sensor] = mean
                self.prev_state[sensor] = [mean, var]
        return res

    def infer(self, data, timestamps, whitelists):
        transposed_data = data.T
        transposed_res = np.zeros_like(transposed_data)
        for i, sensor_data in enumerate(transposed_data):
            transposed_res[i] = self.infer_timestamp(sensor_data,
                                                     timestamps[i],
                                                     whitelists[i])
        return transposed_res.T


    def infer_window(self, test_data, budget):
        self.prev_state = defaultdict(lambda: False)
        num_sensors = self.get_num_sensors(test_data)
        num_timestamps = self.get_num_timestamps(test_data)
        timestamps = generate_timestamps(num_timestamps)
        a = ActiveInferenceWindow()
        windows = a.generate_windows(num_sensors, budget, num_timestamps)
        return self.infer(test_data, timestamps, windows)

    def infer_var(self, test_data, budget):
        self.prev_state = defaultdict(lambda: False)
        transposed_data = test_data.T
        transposed_res = np.zeros_like(transposed_data)
        num_sensors = self.get_num_sensors(test_data)

        timestamp = 0.5

        temp_hsh = {}
        for sensor in range(num_sensors):
            index = self.model_index(sensor, timestamp)
            temp_hsh[sensor] = self.model[index][1]
        whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]

        for i, sensor_data in enumerate(transposed_data):
            transposed_res[i] = self.infer_timestamp(sensor_data, timestamp, whitelist)
            timestamp = (timestamp + 0.5) % 24
            temp_hsh = {k: v[1] for k, v in self.prev_state.items()}
            whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]
        return transposed_res.T

### Inference

class ActiveInferenceWindow(object):

    def generate_windows(self, limit, window_size, n):
        nums = itertools.cycle(range(limit))
        return [[next(nums) for _ in range(window_size)] for _ in range(n)]

#### Running code

budgets = [0, 5, 10, 20, 25]

data_dir = '../data/intelLabDataProcessed/'
hum_train_datafile = data_dir + 'intelHumidityTrain.csv'
tmp_train_datafile = data_dir + 'intelTemperatureTrain.csv'
hum_test_datafile  = data_dir + 'intelHumidityTest.csv'
tmp_test_datafile  = data_dir + 'intelTemperatureTest.csv'

results_dir = '../results/'
hum_results_folder = results_dir + 'humidity/'
tmp_results_folder = results_dir + 'temperature/'

hum_train_data = read_csv_data(hum_train_datafile)
tmp_train_data = read_csv_data(tmp_train_datafile)
hum_test_data  = read_csv_data(hum_test_datafile)
tmp_test_data  = read_csv_data(tmp_test_datafile)

hum_model_1 = HourLevelModel()
hum_model_2 = DayLevelModel()
tmp_model_1 = HourLevelModel()
tmp_model_2 = DayLevelModel()

hum_model_1.train(hum_train_data)
hum_model_2.train(hum_train_data)
tmp_model_1.train(tmp_train_data)
tmp_model_2.train(tmp_train_data)

# Mean Absolute Error Statistics
hum_model_1_mae = {}
hum_model_2_mae = {}

tmp_model_1_mae = {}
tmp_model_2_mae = {}

for budget in budgets:
    hw_filename = "h-w%d.csv" % budget
    hv_filename = "h-v%d.csv" % budget
    dw_filename = "d-w%d.csv" % budget
    dv_filename = "d-v%d.csv" % budget

    tmp_hw_pred = tmp_model_1.infer_window(tmp_test_data, budget)
    tmp_hv_pred = tmp_model_1.infer_var(tmp_test_data, budget)
    tmp_dw_pred = tmp_model_2.infer_window(tmp_test_data, budget)
    tmp_dv_pred = tmp_model_2.infer_var(tmp_test_data, budget)

    write_csv_data(tmp_results_folder + hw_filename, tmp_hw_pred)
    write_csv_data(tmp_results_folder + hv_filename, tmp_hv_pred)
    write_csv_data(tmp_results_folder + dw_filename, tmp_dw_pred)
    write_csv_data(tmp_results_folder + dv_filename, tmp_dv_pred)

    tmp_model_1_mae[hw_filename] = np.mean(np.absolute(tmp_hw_pred - tmp_test_data))
    tmp_model_2_mae[hv_filename] = np.mean(np.absolute(tmp_hv_pred - tmp_test_data))
    tmp_model_1_mae[dw_filename] = np.mean(np.absolute(tmp_dw_pred - tmp_test_data))
    tmp_model_2_mae[dv_filename] = np.mean(np.absolute(tmp_dv_pred - tmp_test_data))

    hum_hw_pred = hum_model_1.infer_window(hum_test_data, budget)
    hum_hv_pred = hum_model_1.infer_var(hum_test_data, budget)
    hum_dw_pred = hum_model_2.infer_window(hum_test_data, budget)
    hum_dv_pred = hum_model_2.infer_var(hum_test_data, budget)

    write_csv_data(hum_results_folder + hw_filename, hum_hw_pred)
    write_csv_data(hum_results_folder + hv_filename, hum_hv_pred)
    write_csv_data(hum_results_folder + dw_filename, hum_dw_pred)
    write_csv_data(hum_results_folder + dv_filename, hum_dv_pred)

    hum_model_1_mae[hw_filename] = np.mean(np.absolute(hum_hw_pred - hum_test_data))
    hum_model_2_mae[hv_filename] = np.mean(np.absolute(hum_hv_pred - hum_test_data))
    hum_model_1_mae[dw_filename] = np.mean(np.absolute(hum_dw_pred - hum_test_data))
    hum_model_2_mae[dv_filename] = np.mean(np.absolute(hum_dv_pred - hum_test_data))

write_mae_data(results_dir + 'temperature_model_1_mae.txt', tmp_model_1_mae)
write_mae_data(results_dir + 'temperature_model_2_mae.txt', tmp_model_2_mae)
write_mae_data(results_dir + 'humidity_model_1_mae.txt', hum_model_1_mae)
write_mae_data(results_dir + 'humidity_model_2_mae.txt', hum_model_2_mae)
