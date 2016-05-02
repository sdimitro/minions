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

def calculate_betas(data):
    num_sensors = len(data)
    num_timestamps = len(data[0])
    inversed_data = data.T

    betas = []
    residuals = []
    regrs = []

    for sensor_data in data:
        b = [0.0, []]
        regr = linear_model.LinearRegression()
        regr.fit(inversed_data, np.roll(sensor_data, -1))
        b[0] = regr.intercept_
        b[1] = list(regr.coef_)
        betas.append(b)

    return betas

def calculate_residuals(data, betas):
    residuals = []
    for sensor_id in range(len(data)):
        x_means = []
        for timestamp in range(len(data[0])):
            x_mean = betas[sensor_id][0]
            for other_sensor_id in range(len(data)):
                temp  = data[other_sensor_id][timestamp]
                temp *= betas[sensor_id][1][other_sensor_id]
                x_mean += temp
            x_means.append(x_mean)
        residuals.append(np.var(x_means))
    return residuals

def calculate_misi(data):
    mi = []
    si = []
    for sensor_data in data:
        mi.append(np.mean(sensor_data))
        si.append(np.var(sensor_data))
    return mi, si

class ActiveInferenceWindow(object):
    def generate_windows(self, limit, window_size, n):
        nums = itertools.cycle(range(limit))
        return [[next(nums) for _ in range(window_size)] for _ in range(n)]

class RandomVariable(object):

    def __init__(self):
        self.sensor_id = -1
        self.mi = False
        self.si = False
        self.b_0 = False
        self.betas = []
        self.residual = False
        self.observed = False
        self.starting = False
        self.mean = False
        self.var = False

    def observe(self, observation):
        self.mean, self.var = observation, 0
        self.observed = True

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

    def train(self, train_data):
        betas = calculate_betas(train_data)
        residuals = calculate_residuals(train_data, betas)
        mi, si = calculate_misi(train_data)
        self.sensor_params = {}
        for sensor_id in range(len(train_data)):
            rv = RandomVariable()
            rv.sensor_id = sensor_id
            rv.mi, rv.si = mi[sensor_id], si[sensor_id]
            rv.b_0 = betas[sensor_id][0]
            rv.betas = betas[sensor_id][1]
            rv.residual = residuals[sensor_id]
            self.sensor_params[sensor_id] = rv

    def infer_current_state(self, sensor_rv, sensor_id, prev_snapshot):
        b_0      = sensor_rv.b_0
        betas    = sensor_rv.betas
        sigma_sq = sensor_rv.residual

        prev_means = [prev_snapshot[sid].mean
                      for sid in range(len(prev_snapshot))]
        prev_sigma_sqs = [prev_snapshot[sid].var
                          for sid in range(len(prev_snapshot))]

        mean = b_0
        var  = sigma_sq

        for bi, pm, pv in zip(betas, prev_means, prev_sigma_sqs):
            mean += bi * pm
            var  += (bi ** 2) * pv

        prev_snapshot[sensor_id].mean = mean
        prev_snapshot[sensor_id].var  = var
        return mean

    def infer_timestamp(self, sensor_data, whitelist):
        num_sensors = self.get_num_sensors(sensor_data)
        res = np.zeros(num_sensors)
        for sensor, sensor_rv in self.sensor_params.items():
            if self.to_be_observed(sensor, whitelist):
                res[sensor] = sensor_data[sensor]
                # Throws trash for GC after first timestamp :-(
                # TODO: prev pointers maybe?
                self.prev_snapshot[sensor] = RandomVariable()
                self.prev_snapshot[sensor].observe(sensor_data[sensor])

            elif not self.prev_snapshot[sensor]:
                res[sensor] = sensor_rv.mi
                # Same Object pointer for both
                prev_rv = self.prev_snapshot[sensor] = RandomVariable()
                prev_rv.mean = sensor_rv.mi
                prev_rv.var = sensor_rv.si
                prev_rv.starting = True

            else:
                res[sensor] = self.infer_current_state(sensor_rv, sensor,
                                                       self.prev_snapshot)
        return res

    def infer_window(self, test_data, budget):
        self.prev_snapshot = defaultdict(lambda: False)
        num_sensors = self.get_num_sensors(test_data)
        num_timestamps = self.get_num_timestamps(test_data)
        a = ActiveInferenceWindow()
        windows = a.generate_windows(num_sensors, budget, num_timestamps)
        return self.infer(test_data, windows, self.infer_timestamp)


    def infer_var(self, test_data, budget):
        self.prev_snapshot = defaultdict(lambda: False)
        transposed_data = test_data.T
        transposed_res = np.zeros_like(transposed_data)
        temp_hsh = {k: v.var for k, v in self.sensor_params.items()}
        whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]
        for i, sensor_data in enumerate(transposed_data):
            transposed_res[i] = self.infer_timestamp(sensor_data, whitelist)
            temp_hsh = {k: v.var for k, v in self.prev_snapshot.items()}
            whitelist = sorted(temp_hsh, key=temp_hsh.get, reverse=True)[:budget]
        return transposed_res.T

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

hum_betas = calculate_betas(hum_train_data)
tmp_betas = calculate_betas(tmp_train_data)

hum_residuals = calculate_residuals(hum_train_data, hum_betas)
tmp_residuals = calculate_residuals(tmp_train_data, hum_betas)

hum_mi, hum_si = calculate_misi(hum_train_data)
tmp_mi, tmp_si = calculate_misi(tmp_train_data)

hum_model = HourLevelModel()
tmp_model = HourLevelModel()
 
hum_model.train(hum_train_data)
tmp_model.train(tmp_train_data)

# Mean Absolute Error Statistics
hum_model_mae = {}
tmp_model_mae = {}

for budget in budgets:
    w_filename = "w%d.csv" % budget
    v_filename = "v%d.csv" % budget

    tmp_w_pred = tmp_model.infer_window(tmp_test_data, budget)
    tmp_v_pred = tmp_model.infer_var(tmp_test_data, budget)

    write_csv_data(tmp_results_folder + w_filename, tmp_w_pred)
    write_csv_data(tmp_results_folder + v_filename, tmp_v_pred)

    tmp_model_mae[w_filename] = np.mean(np.absolute(tmp_w_pred - tmp_test_data))
    tmp_model_mae[v_filename] = np.mean(np.absolute(tmp_v_pred - tmp_test_data))

    hum_w_pred = hum_model.infer_window(hum_test_data, budget)
    hum_v_pred = hum_model.infer_var(hum_test_data, budget)

    write_csv_data(hum_results_folder + w_filename, hum_w_pred)
    write_csv_data(hum_results_folder + v_filename, hum_v_pred)

    hum_model_mae[w_filename] = np.mean(np.absolute(hum_w_pred - hum_test_data))
    hum_model_mae[v_filename] = np.mean(np.absolute(hum_v_pred - hum_test_data))

write_mae_data(results_dir + 'temperature_model_mae.txt', tmp_model_mae)
write_mae_data(results_dir + 'humidity_model_mae.txt', hum_model_mae)
