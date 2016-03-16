import numpy as np

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

