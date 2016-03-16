from collections import defaultdict

import numpy as np

#### File I/O

def read_csv_data(filename):
    return np.genfromtxt(filename, delimiter=',', skip_header=1)[:,1:]

def generate_header(length):
    header = "sensors"
    timestamp = 0.5
    for i in range(length):
        header += "," + str(timestamp)
        timestamp += 0.5
        if timestamp == 24.0:
            timestamp = 0.0
    header += "\n"
    return header

def write_csv_data(filename, data):
    header = generate_header(len(data[0]))
    num_sensors = len(data)
    with open(filename, mode='w') as f:
        f.write(header)
        for sensor_id in range(num_sensors):
            line = str(sensor_id)
            for entry in data[sensor_id]:
                line += "," + ("%.2E" % entry)
            line += "\n"
            f.write(line)

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
    model[index][0] = sum(model[index][2]) / model[index][3]

def rv_calc_var(model, index):
    mean = model[index][0]
    N = model[index][3]
    summa = 0

    for entry in model[index][2]:
        summa += (entry - mean) ** 2
    model[index][1] = summa / N

def populate_sensor_rvs(model, sensor_id, sensor_data):
    initial_timestamp = 0.5
    lowest_timestamp  = 0.0
    highest_timestamp = 23.5
    timestamp_step    = 0.5

    current_timestamp = initial_timestamp
    for entry in sensor_data:
        rv_index = model_index(sensor_id, current_timestamp)
        rv_add_entry(model, rv_index, entry)

        current_timestamp += timestamp_step
        if current_timestamp > highest_timestamp:
            current_timestamp = lowest_timestamp

    for timestamp in np.arange(0.0, 24.0, 0.5):
        rv_index = model_index(sensor_id, timestamp)
        rv_calc_mean(model, rv_index)
        rv_calc_var(model, rv_index)

def train_model(data):
    model = defaultdict(lambda: [0, 0, [], 0])
    sensor_id = 0
    for sensor_readings in data:
        populate_sensor_rvs(model, sensor_id, sensor_readings)
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

def infer_window(model, data, window_size):
    num_sensors = len(data)
    num_timestamps = len(data[0])
    transposed_res = np.zeros((num_timestamps, num_sensors))

    initial_timestamp = 0.5
    lowest_timestamp  = 0.0
    highest_timestamp = 23.5
    timestamp_step    = 0.5

    i = 0
    window_start = 0
    current_timestamp = initial_timestamp
    for sensor_data in data.T:
        whitelist, window_start = generate_window(window_start,
                                                  num_sensors,
                                                  window_size)
        transposed_res[i] = infer_timestamp(model,
                                            sensor_data,
                                            current_timestamp,
                                            whitelist)

        i += 1
        current_timestamp += timestamp_step
        if current_timestamp > highest_timestamp:
            current_timestamp = lowest_timestamp

    return transposed_res.T

#### Running code

budgets = [0, 5, 10, 20, 25]

hum_train_datafile = 'intelLabDataProcessed/intelHumidityTrain.csv'
tmp_train_datafile = 'intelLabDataProcessed/intelTemperatureTrain.csv'

hum_test_datafile = 'intelLabDataProcessed/intelHumidityTest.csv'
tmp_test_datafile = 'intelLabDataProcessed/intelTemperatureTest.csv'

hum_train_data = read_csv_data(hum_train_datafile)
tmp_train_data = read_csv_data(tmp_train_datafile)

hum_test_data = read_csv_data(hum_test_datafile)
tmp_test_data = read_csv_data(tmp_test_datafile)

hum_model = train_model(hum_train_data)
tmp_model = train_model(tmp_train_data)

for b in budgets:
    hum_w_filename = "w%d.csv" % b
    hum_v_filename = "v%d.csv" % b
    hum_w_pred = infer_window(hum_model, hum_test_data, b)
    write_csv_data(hum_w_filename, hum_w_pred)
