from collections import defaultdict

import numpy as np

#### File I/O

def read_csv_data(filename):
    return np.genfromtxt(filename, delimiter=',', skip_header=1)[:,1:]

def write_csv_data(filename, data):
    generated_header = "TODO"
    # generated_sensor_ids = np.arange(0, 50)
    # add generated row and column to data
    np.savetxt(filename,
               data,
               #np.insert(data, 0, generated_sensor_ids, axis=1),
               fmt='%.2E',
               delimiter=',',
               header=generated_header,
               comments='')

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

#### RUNNING CODE

hum_train_datafile = 'intelLabDataProcessed/intelHumidityTrain.csv'
tmp_train_datafile = 'intelLabDataProcessed/intelTemperatureTrain.csv'

hum_train_data = read_csv_data(hum_train_datafile)
tmp_train_data = read_csv_data(tmp_train_datafile)

hum_model = train_model(hum_train_data)
tmp_model = train_model(tmp_train_data)

#print(hum_model)
#print('============')
#print(tmp_model)
