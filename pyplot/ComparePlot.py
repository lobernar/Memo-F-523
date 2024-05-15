import matplotlib.pyplot as plt
import numpy as np
import os

# Calculate the moving average
def moving_average(data, window_size):
    cumsum = np.cumsum(data, dtype=int)
    cumsum[window_size:] = cumsum[window_size:] - cumsum[:-window_size]
    x = cumsum[window_size - 1:] / window_size
    y = (np.sum(data[window_size:]))
    return cumsum[window_size - 1:] / window_size

# Calculate the median filter
def median_filter(data, window_size):
    filtered_data = np.zeros_like(data)
    for i in range(len(data)):
        window = data[max(0, i - window_size//2):min(len(data), i + window_size//2)]
        filtered_data[i] = np.median(window)
    return filtered_data

def calculate_average(array, step):
    averages = []
    for i in range(0, len(array), step):
        chunk = array[i:i+step]
        avg = sum(chunk) / len(chunk) if chunk else 0  # Handle empty chunk
        averages.append(avg)
    return averages

def plot_data_from_files(file_paths, B, delta, varDELTA=False, varB=False):
    counter = 0
    for file_path in file_paths:
        if not os.path.isfile(file_path):
            print(f"File not found: {file_path}")
            continue
        with open(file_path, 'r') as file:
            x, y = [], []
            for line in file:
                if line.strip():  # Check if the line is not empty
                    data = line.strip().split()
                    if len(data) == 2:
                        x.append(float(data[0]))
                        y.append(float(data[1]))
                    else:
                        print(f"Ignoring malformed line in {file_path}: {line.strip()}")

            avg = calculate_average(y, size)
            if(counter==0): plt.plot(avg, label="B-Tree: B=" + str(B))
        #     plt.subplot(1,2,counter)
            elif(counter==2): plt.plot(avg, label="$B^\\delta$-Tee: B = " + str(B) + ", $\\delta$ = " + str(0.5))
        #         plt.xlabel('Delete')
        #         plt.ylabel('Block Transfers')
        #         plt.legend(loc='upper left')
            else: plt.plot(avg, label="$B^\\epsilon$-Tee: B = " + str(B) + ", $\\epsilon$ = " + str(0.5))
        #         plt.legend(loc='upper left')
        # if(varDELTA): delta += 0.25
        # if(varB): B+=50
        counter += 1

    plt.xlabel('Insert')
    plt.ylabel('Block Transfers')
    plt.suptitle('Average comparison between B, $B^\\epsilon$ and $B^\\delta$-Tree when deleting $10^7$ items in flushing cascades order')
    plt.legend(loc='upper left')
    plt.show()

# Example usage:
#file_paths = ['../B-Tree/data_10mil_remove_50.dat', '../BeTree/data_del_50_0.500000_10mil_.dat', '../Bd-Tree/data_VarDelta_del_50_0.250000_10mil_.dat']
#file_paths = ['../B-Tree/data_10mil_50.dat', '../BeTree/data_ins_50_0.500000_10mil_.dat', '../Bd-Tree/data_VarDelta_ins_50_0.250000_10mil_.dat']
file_paths = ['../B-Tree/data_flushCasc2_50.dat', '../BeTree/data_flushCasc2_50_0.500000.dat', '../Bd-Tree/data_VarDelta_del_50_0.500000_10mil_.dat']
size = 1000
B = 50
#B = [100, 300, 500, 700, 1000]
delta = 0.25
eps = 0.5
varDELTA = False
varB = False
plt.rcParams.update({'font.size': 18})
plot_data_from_files(file_paths, B, delta, varDELTA, varB)

