from collections import OrderedDict

import numpy as np

def measure_gflops(inst):
    return inst['vCPU'] * inst['Clock'] * inst['IPC']

def measure_all_gflops(hsh):
    for k, v in hsh.items():
        v['GFLOPS'] = round(measure_gflops(v), 2)

def measure_cost_per_tflop(hsh):
    for k, v in hsh.items():
        v['$/TFLOP'] = round(v['Price'] / (v['GFLOPS'] / 1000), 3)

def measure_5yr_cost(hsh):
    hrs_in_5_years = 24 * 365 * 5
    for k, v in hsh.items():
        v['5yr-Cost'] = round(v['Price'] * hrs_in_5_years, 2)

def print_instance_table(hsh):
    print("Instance name\tGFLOPS\t$/hr/TFLOP\t5 year price\t")
    for k, v in hsh.items():
        print("\t".join([k, str(v['GFLOPS']), str(v['$/TFLOP']),
                         str(v['5yr-Cost'])]))

def print_plot1_table(ord, hsh):
    print("GFLOPS\t\t$/hr/TFLOP\tInstances Used")
    for k, v in ord.items():
        rtflop = 0
        used = ""
        if len(set(v)) == 1:
            used = "%s (x%d)" % (v[0], len(v))
            rtflop = hsh[v[0]]['$/TFLOP']
        else:
            used = ', '.join(v)
            rtflop = np.mean([hsh[i]['$/TFLOP'] for i in v])
        print("\t".join([k, str(rtflop), used]))

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

instance_plot = OrderedDict()
instance_plot['1-GFLOPs']   = ['m3.large']
instance_plot['10-GFLOPs']  = ['m3.large']
instance_plot['100-GFLOPs'] = ['m3.large', 'm3.large', 'm3.large']
instance_plot['1-TFLOPs']   = ['c3.8xlarge', 'm3.2xlarge', 'm3.large']
instance_plot['10-TFLOPs']  = ['m4.10xlarge'] * 7
instance_plot['100-TFLOPs'] = ['m4.10xlarge'] * 66
instance_plot['1-PFLOPs']   = ['m4.10xlarge'] * 651

measure_all_gflops(instance)
measure_cost_per_tflop(instance)
measure_5yr_cost(instance)

