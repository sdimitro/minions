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

print_instance_table(instance)
print()
print_plot1_table(instance_plot, instance)
print()

def mike_instance_cost(hsh):
    for k, v in hsh.items():
        v['Total']  = v['CPU-cost'] + v['Mother-cost']
        v['Total'] += v['RAM-Q'] * v['RAM-QPrice']
        v['Total'] += v['Storage-Price'] + v['Case-Price']
        v['Total'] += v['Net-Cost']

def print_instance_table(hsh):
    print("Instance\tPrice")
    for k, v in hsh.items():
        print("\t".join([k, str(v['Total'])]))

instance_mike = {
'm4.10xlarge': {
    'CPU-Link': 'http://www.ebay.com/itm/INTEL-XEON-E5-2676-V3-2-40GHz-SR1Y5-30Mb-12-Cores-/252340932142?hash=item3ac0b10e2e:g:3oAAAOSwKtVW1XQf',
    'CPU-cost': 2350,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 2,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820242019',
    'RAM-QPrice': 365,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820167360&cm_re=Server_SSD-_-20-167-360-_-Product',
    'Storage-Price': 600,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
},
'm3.large' :   {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16819116929&cm_re=Xeon_E5-2670_v2-_-19-116-929-_-Product',
    'CPU-cost': 1600,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 1,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239370&cm_re=Server_RAM-_-20-239-370-_-Product',
    'RAM-QPrice': 50,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA67S33E4594&cm_re=32_GB_SSD-_-9SIA67S33E4594-_-Product',
    'Storage-Price': 85,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16833106015',
    'Net-Cost': 143
},
'm3.2xlarge' : {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16819116929&cm_re=Xeon_E5-2670_v2-_-19-116-929-_-Product',
    'CPU-cost': 1600,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 1,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239371&cm_re=32_GB_RAM-_-20-239-371-_-Product',
    'RAM-QPrice': 205,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIAAEE3XS6865&cm_re=160_GB_SSD-_-20-167-055-_-Product',
    'Storage-Price': 140,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16833106015',
    'Net-Cost': 143
},
'c3.8xlarge' : {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16819116928&cm_re=Xeon_E5-2680_v2-_-19-116-928-_-Product',
    'CPU-cost': 1770,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 2,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239371&cm_re=32_GB_RAM-_-20-239-371-_-Product',
    'RAM-QPrice': 205,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIAAEE3XS6865&cm_re=160_GB_SSD-_-20-167-055-_-Product',
    'Storage-Price': 4 * 140,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
},
'g2.2xlarge' : {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA6393RS7704&cm_re=Xeon_E5-2670-_-9SIA6393RS7704-_-Product',
    'GPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16814132015&cm_re=nvidia_tesla-_-14-132-015-_-Product',
    'CPU-cost': 140 + 3130, # CPU (Refurbished)
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 1,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820242018&cm_re=16_GB_RAM_Server-_-20-242-018-_-Product',
    'RAM-QPrice': 93,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820233782&cm_re=60GB_SSD-_-20-233-782-_-Product',
    'Storage-Price': 35,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
},
'r3.4xlarge' : {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16819116929&cm_re=Xeon_E5-2670_v2-_-19-116-929-_-Product',
    'CPU-cost': 1600,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 4,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239371&cm_re=32_GB_RAM-_-20-239-371-_-Product',
    'RAM-QPrice': 205,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIAAEE3XS6865&cm_re=160_GB_SSD-_-20-167-055-_-Product',
    'Storage-Price': 2 * 140,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
},
'i2.8xlarge' : {
    'CPU-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16819116929&cm_re=Xeon_E5-2670_v2-_-19-116-929-_-Product',
    'CPU-cost': 1600,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 8,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239371&cm_re=32_GB_RAM-_-20-239-371-_-Product',
    'RAM-QPrice': 205,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA5EM2M30054&cm_re=800_GB_Server_SSD-_-20-167-160-_-Product',
    'Storage-Price': 8 * 730,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
},
'd2.8xlarge' : {
    'CPU-Link': 'http://www.ebay.com/itm/INTEL-XEON-E5-2676-V3-2-40GHz-SR1Y5-30Mb-12-Cores-/252340932142?hash=item3ac0b10e2e:g:3oAAAOSwKtVW1XQf',
    'CPU-cost': 2 * 2350,
    'Mother-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16813132256&cm_re=Xeon_Motherboard-_-13-132-256-_-Product',
    'Mother-cost': 550,
    'RAM-Q': 8,
    'RAM-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16820239371&cm_re=32_GB_RAM-_-20-239-371-_-Product',
    'RAM-QPrice': 205,
    'Storage-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16822236599&cm_re=Server_HDD-_-22-236-599-_-Product',
    'Storage-Price': 12 * 150,
    'Case-Link': 'http://www.newegg.com/Product/Product.aspx?Item=N82E16811165213&cm_re=Server_Case-_-11-165-213-_-Product',
    'Case-Price': 60,
    'Net-Link': 'http://www.newegg.com/Product/Product.aspx?Item=9SIA24G1XA5330',
    'Net-Cost': 500
}
}

mike_instance_cost(instance_mike)
print_instance_table(instance_mike)

