import numpy as np
import matplotlib.pyplot as plt

def draw(means_w, means_v, budget, type):
    title = "%s sensors at budget %d" % (type, budget)
    filename = "%s_%d_final.png" % (type, budget)

    n_groups = 4
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.35
    opacity = 0.4
    
    rects1 = plt.bar(index, means_w, bar_width,
        alpha=opacity,
        color='b',
        label='Window')
    
    rects2 = plt.bar(index + bar_width, means_v, bar_width,
        alpha=opacity,
        color='r',
        label='Variance')
    
    plt.ylabel('Mean Absolute Error')
    plt.ylim(0, max(means_w + means_v) + 0.5)
    plt.title(title)
    plt.xticks(index + bar_width, ('Phase 1 Model',
                                   'Phase 2 Model 1',
                                   'Phase 2 Model 2',
                                   'Phase 3 Model'))
    plt.legend()
    
    plt.tight_layout()
    plt.savefig(filename)


def setup_means(type, budget, inference, data):
    means = []
    means.append(data[type+'_phase1_mae.txt@'+inference+str(budget)+'.csv'])
    means.append(data[type+'_model_1_mae.txt@h-'+inference+str(budget)+'.csv'])
    means.append(data[type+'_model_2_mae.txt@d-'+inference+str(budget)+'.csv'])
    means.append(data[type+'_model_mae.txt@'+inference+str(budget)+'.csv'])
    return means

means_w = [20, 35, 30]
means_v = [25, 32, 34]

types = ['humidity', 'temperature']
inference = ['w', 'v']
budgets = [0, 5, 10, 20, 25]

phase1_files = ["humidity_phase1_mae.txt", "temperature_phase1_mae.txt"]
model1_files = ["humidity_model_1_mae.txt", "temperature_model_1_mae.txt"]
model2_files = ["humidity_model_2_mae.txt", "temperature_model_2_mae.txt"]
phase3_files = ["humidity_model_mae.txt", "temperature_model_mae.txt"]
all_files = phase1_files + model1_files + model2_files + phase3_files

global_hash = {}

for file in all_files:
    with open(file) as f:
        contents = f.readlines()
        for line in contents:
            tokens = line.split()
            global_hash[file+'@'+tokens[0]] = float(tokens[1])

for type in types:
    for budget in budgets:
        means_w = setup_means(type, budget, inference[0], global_hash)
        means_v = setup_means(type, budget, inference[1], global_hash)
        draw(means_w, means_v, budget, type)
