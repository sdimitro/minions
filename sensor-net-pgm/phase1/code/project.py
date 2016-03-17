from collections import defaultdict

import numpy as np

#### Helper Functions

def get_num_sensors(readings):
    return len(readings)

def get_num_timestamps(readings):
    return len(readings[0])

def generate_timestamps(num_timestamps):
    res = []

    initial_timestamp = 0.5
    lowest_timestamp  = 0.0
    highest_timestamp = 23.5
    timestamp_step    = 0.5

    current_timestamp = initial_timestamp
    for i in range(num_timestamps):
        res.append(current_timestamp)

        current_timestamp += timestamp_step
        if current_timestamp > highest_timestamp:
            current_timestamp = lowest_timestamp
    return res

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
        for sensor_id in range(get_num_sensors(data)):
            line = str(sensor_id) + ","
            line += ",".join(map(lambda x: "%.2E" % x,
                                 data[sensor_id]))
            line += "\n"
            f.write(line)

def write_mae_data(filename, data):
    lmbd = lambda x: (x + "\t" + str(data[x]))
    with open(filename, mode='w') as f:
        f.write("\n".join(map(lmbd, sorted(data))))
        f.write("\n")

#### Modeling

def model_index(sensor_id, timestamp):
    return "s%d-%.1f" % (sensor_id, timestamp)

def rv_mean(model, index):
    return model[index][0]

def rv_var(model, index):
    return model[index][1]

def rv_add_entry(model, index, entry):
    model[index][2].append(entry)
    model[index][3] += 1

def rv_calc_mean(model, index):
    model[index][0] = np.mean(model[index][2])

def rv_calc_var(model, index):
    model[index][1] = np.var(model[index][2])

def populate_sensor_rvs(model, sensor_id, sensor_data, timestamps):
    i = 0
    for entry in sensor_data:
        rv_index = model_index(sensor_id, timestamps[i])
        rv_add_entry(model, rv_index, entry)
        i += 1

    for timestamp in np.arange(0.0, 24.0, 0.5):
        rv_index = model_index(sensor_id, timestamp)
        rv_calc_mean(model, rv_index)
        rv_calc_var(model, rv_index)

def train_model(data):
    model = defaultdict(lambda: [0, 0, [], 0])
    sensor_id = 0
    timestamps = generate_timestamps(get_num_timestamps(data))
    for sensor_readings in data:
        populate_sensor_rvs(model, sensor_id, sensor_readings, timestamps)
        sensor_id += 1
    return model

#### Inference

def highest_prediction_variance_sensors(model, timestamp, num_sensors, n):
    res = []
    variance_table = {}
    for sensor_id in range(num_sensors):
        rv_index = model_index(sensor_id, timestamp)
        variance_table[sensor_id] = rv_var(model, rv_index)
    sorted_sensor_list = sorted(variance_table,
                                key=variance_table.get,
                                reverse = True)
    return sorted_sensor_list[:n]

def to_be_predicted(sensor_id, whitelist):
    return not sensor_id in whitelist

def generate_window(start, limit, window_size):
    res = []
    ids_left = window_size
    current_id = start
    while ids_left > 0:
        res.append(current_id % limit)
        ids_left -= 1
        current_id += 1
    return res, (current_id % limit)

def infer_timestamp(model, test_data, timestamp, whitelist):
    num_sensors = len(test_data)
    res = np.zeros(num_sensors)
    for sensor in range(num_sensors):
        if to_be_predicted(sensor, whitelist):
            rv_index = model_index(sensor, timestamp)
            res[sensor] = rv_mean(model, rv_index)
        else:
            res[sensor] = test_data[sensor]
    return res

def generate_windows(num_timestamps, num_sensors, window_size):
    windows = []
    window_start = 0
    for i in range(num_timestamps):
        window, window_start = generate_window(window_start,
                                               num_sensors,
                                               window_size)
        windows.append(window)
    return windows

def collect_highest_vars(model, timestamps, num_sensors, n):
    whitelists = []
    for timestamp in timestamps:
        whitelist = highest_prediction_variance_sensors(model,
                                                        timestamp,
                                                        num_sensors,
                                                        n)
        whitelists.append(whitelist)
    return whitelists

def infer_window(model, data, window_size):
    num_sensors = len(data)
    num_timestamps = len(data[0])
    timestamps = generate_timestamps(num_timestamps)
    whitelists = generate_windows(num_timestamps,
                                  num_sensors,
                                  window_size)
    return infer(model, data, timestamps, whitelists)

def infer_var(model, data, n):
    num_sensors = len(data)
    num_timestamps = len(data[0])
    timestamps = generate_timestamps(num_timestamps)
    whitelists = collect_highest_vars(model, timestamps, num_sensors, n)
    return infer(model, data, timestamps, whitelists)

def infer(model, data, timestamps, whitelists):
    transposed_data = data.T
    transposed_res = np.zeros_like(transposed_data)
    i = 0
    for sensor_data in transposed_data:
        transposed_res[i] = infer_timestamp(model,
                                            sensor_data,
                                            timestamps[i],
                                            whitelists[i])

        i += 1
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

hum_model = train_model(hum_train_data)
tmp_model = train_model(tmp_train_data)

# Mean Absolute Error Statistics
hum_mae = {}
tmp_mae = {}

for budget in budgets:
    w_filename = "w%d.csv" % budget
    v_filename = "v%d.csv" % budget

    tmp_w_pred = infer_window(tmp_model, tmp_test_data, budget)
    tmp_v_pred = infer_var(tmp_model, tmp_test_data, budget)
    write_csv_data(tmp_results_folder + w_filename, tmp_w_pred)
    write_csv_data(tmp_results_folder + v_filename, tmp_v_pred)
    tmp_mae[w_filename] = np.mean(np.absolute(tmp_w_pred - tmp_test_data))
    tmp_mae[v_filename] = np.mean(np.absolute(tmp_v_pred - tmp_test_data))

    hum_w_pred = infer_window(hum_model, hum_test_data, budget)
    hum_v_pred = infer_var(hum_model, hum_test_data, budget)
    write_csv_data(hum_results_folder + w_filename, hum_w_pred)
    write_csv_data(hum_results_folder + v_filename, hum_v_pred)
    hum_mae[w_filename] = np.mean(np.absolute(hum_w_pred - hum_test_data))
    hum_mae[v_filename] = np.mean(np.absolute(hum_v_pred - hum_test_data))

write_mae_data(results_dir + 'temperature_mae.txt', tmp_mae)
write_mae_data(results_dir + 'humidity_mae.txt', hum_mae)
