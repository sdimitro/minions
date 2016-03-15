from numpy import genfromtxt

def read_csv_data(filename):
    return genfromtxt(filename, delimiter=',', skip_header=1)[:,1:]

