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

# note: Expects mean to be populated
def rv_calc_var(model, index):
    mean = model[index][0]
    N = model[index][3]
    summa = 0

    for entry in model[index][2]:
        summa += (entry - mean) ** 2
    model[index][1] = summa / N


def train_model(data):
    model = defaultdict(lambda: [0, 0, [], 0])
    return model
