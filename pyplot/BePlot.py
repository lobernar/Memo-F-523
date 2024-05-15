import matplotlib.pyplot as plt
import numpy as np
import os

# Calculate the moving average
def moving_average(data, window_size):
    cumsum = np.cumsum(data, dtype=int)
    cumsum[window_size:] = cumsum[window_size:] - cumsum[:-window_size]
    return cumsum[window_size - 1:] / window_size

# Calculate the median filter
def median_filter(data, window_size):
    filtered_data = np.zeros_like(data)
    for i in range(len(data)):
        window = data[max(0, i - window_size//2):min(len(data), i + window_size//2)]
        filtered_data[i] = np.median(window)
    return filtered_data

def median_filter_optimized(data, window_size):
    # Pad the data array to handle edge cases
    padded_data = np.pad(data, (window_size//2, window_size//2), mode='edge')
    
    # Create a view of the data with rolling windows
    shape = (len(data), window_size)
    strides = (padded_data.strides[0], padded_data.strides[0])
    windowed_data = np.lib.stride_tricks.as_strided(padded_data, shape=shape, strides=strides)
    
    # Compute median along the window axis
    filtered_data = np.median(windowed_data, axis=1)
    
    return filtered_data

def calculate_average(array, step):
    averages = []
    for i in range(0, len(array), step):
        chunk = array[i:i+step]
        avg = sum(chunk) / len(chunk) if chunk else 0  # Handle empty chunk
        averages.append(avg)
    return averages

def plot_data_from_files(file_paths, B, eps, varEPS=False, varB=False):
    index = 0
    index1=0
    for file_path in file_paths:
        #b = B[index]
        ep = eps[index1]
        index += 1
        index1 += 1
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

            #cum = moving_average(y, size)
            avg = calculate_average(y, size)
            #plt.plot(x[size-1:], cum, label="B = " + str(B) + " eps = " + str(eps))
            plt.plot(avg, label="B = " + str(50) + ", $\\epsilon $ = " + str(ep))
        if(varEPS): eps += 0.25
        if(varB): B += 50

    plt.xlabel('Delete')
    plt.ylabel('Block Transfers')
    plt.title('Deleting $10^7$ items in flushing cascades order in $B^\\epsilon$-Tree')
    #plt.title('Average block transfers required for deleting 10 million elements in $B^\\epsilon$-Tree')
    plt.legend(loc='upper left')
    plt.show()

# Example usage:

#file_paths = ['../BeTree/data_ins_50_0.250000_10mil_.dat', '../BeTree/data_ins_50_0.500000_10mil_.dat', '../BeTree/data_ins_50_0.750000_10mil_.dat']
#file_paths = ['../BeTree/data_flushCasc2_50_0.250000.dat', '../BeTree/data_flushCasc2_50_0.500000.dat', '../BeTree/data_flushCasc2_50_0.750000.dat']
#file_paths = ['../BeTree/data_flushCasc2_50_0.500000.dat']
# file_paths = ['../BeTree/data_flushCasc2_VarB_50.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_100.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_150.000000_0.500000.dat',
#               '../BeTree/data_flushCasc2_VarB_200.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_250.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_300.000000_0.500000.dat',
#               '../BeTree/data_flushCasc2_VarB_350.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_400.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_450.000000_0.500000.dat',
#               '../BeTree/data_flushCasc2_VarB_500.000000_0.500000.dat']
#file_paths = ['../BeTree/data_flushCasc2_VarB_100.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_300.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_500.000000_0.500000.dat',
#              '../BeTree/data_flushCasc2_VarB_700.000000_0.500000.dat', '../BeTree/data_flushCasc2_VarB_1000_0.500000.dat']
# file_paths = ['../BeTree/data_ins_50_0.500000_10mil_.dat', '../BeTree/data_ins_10mil_100.000000_0.500000.dat', '../BeTree/data_ins_10mil_400.000000_0.500000.dat',
#              '../BeTree/data_ins_10mil_700.000000_0.500000.dat', '../BeTree/data_ins_10mil_1000.000000_0.500000.dat']
# file_paths = ['../BeTree/data_del_50_0.500000_10mil_.dat', '../BeTree/data_del_10mil_100.000000_0.500000.dat', '../BeTree/data_del_10mil_400.000000_0.500000.dat', '../BeTree/data_del_10mil_700.000000_0.500000.dat',
#               '../BeTree/data_del_10mil_1000.000000_0.500000.dat']
#file_paths = ['../BeTree/data_del_50_0.500000_10mil_.dat']
file_paths = ['../BeTree/data_flushCasc2_50_0.500000.dat']
#file_paths = ['../BeTree/data_del_50_0.250000_10mil_.dat', '../BeTree/data_del_50_0.500000_10mil_.dat', '../BeTree/data_del_50_0.750000_10mil_.dat']
size=1
#size = 1000
B = [50]
#B = [50, 100, 400, 700, 1000]
eps = [0.5]
varEPS = False
varB = False
plt.rcParams.update({'font.size': 18})
plot_data_from_files(file_paths, B, eps, varEPS, varB)

