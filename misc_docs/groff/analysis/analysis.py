def measure_gflops(inst):
    return inst['vCPU'] * inst['Clock'] * inst['IPC']

def measure_all_gflops(hsh):
    for k, v in hsh.items():
        v['GFLOPS'] = measure_gflops(v)

def measure_cost_per_tflop(hsh):
    for k, v in hsh.items():
        v['$/TFLOP'] = v['Price'] / (v['GFLOPS'] / 1000)

def measure_5yr_cost(hsh):
    hrs_in_5_years = 24 * 365 * 5
    for k, v in hsh.items():
        v['5yr-Cost'] = v['Price'] * hrs_in_5_years

instance = {
'm4.10xlarge' :{'vCPU': 40, 'Mem': 160, 'Storage(Type)': 'EBS',
                'Net': 10, 'Proc': 'Xeon E5-2676 v3', 'Clock': 2.4, 'IPC': 16,
                'Price': 2.394 },
'm3.large' :   {'vCPU': 2, 'Mem': 7.5, 'Storage(Type)': 'SSD', 'Storage': 32,
                'Net': 0.5, 'Proc': 'Xeon E5-2670 v2', 'Clock': 2.5, 'IPC': 8,
                'Price': 0.133 },
'm3.2xlarge' : {'vCPU': 8, 'Mem': 30, 'Storage(Type)': 'SSD', 'Storage': 160,
                'Net': 1, 'Proc': 'Xeon E5-2670 v2', 'Clock': 2.5, 'IPC': 8,
                'Price': 0.532 },
'c3.8xlarge' : {'vCPU': 32, 'Mem': 60, 'Storage(Type)': 'SSD', 'Storage': 640,
                'Net': 10, 'Proc': 'Xeon E5-2680 v2', 'Clock': 2.8, 'IPC': 8,
                'Price': 1.680 },
'g2.2xlarge' : {'vCPU': 8, 'Mem': 15, 'Storage(Type)': 'SSD', 'Storage': 60,
                'Net': 1, 'Proc': 'Xeon E5-2670', 'Clock': 2.6, 'IPC': 8,
                'Price': 0.650 },
'r3.4xlarge' : {'vCPU': 16, 'Mem': 122, 'Storage(Type)': 'SSD', 'Storage': 320,
                'Net': 1, 'Proc': 'Xeon E5-2670 v2', 'Clock': 2.5, 'IPC': 8,
                'Price': 1.33 },
'i2.8xlarge' : {'vCPU': 32, 'Mem': 244, 'Storage(Type)': 'SSD', 'Storage': 6400,
                'Net': 10, 'Proc': 'Xeon E5-2670 v2', 'Clock': 2.5, 'IPC': 8,
                'Price': 6.82 },
'd2.8xlarge' : {'vCPU': 36, 'Mem': 244, 'Storage(Type)': 'HDD', 'Storage': 48000,
                'Net': 10, 'Proc': 'Xeon E5-2676 v3', 'Clock': 2.4, 'IPC': 16,
                'Price': 5.52 }}

measure_all_gflops(instance)
measure_cost_per_tflop(instance)
measure_5yr_cost(instance)
