import matplotlib.pyplot as plt
import numpy as np
import os

# Calculate the moving average
def moving_average(data, window_size):
    cumsum = np.cumsum(data, dtype=int)
    cumsum[window_size:] = cumsum[window_size:] - cumsum[:-window_size]
    return cumsum[window_size - 1:] / window_size

def calculate_average(array, step):
    averages = []
    for i in range(0, len(array), step):
        chunk = array[i:i+step]
        avg = sum(chunk) / len(chunk) if chunk else 0  # Handle empty chunk
        averages.append(avg)
    return averages

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

def plot_data_from_files(file_paths, B):
    index = 0
    for file_path in file_paths:
        q = B[index]
        index += 1
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
            #cum = median_filter_optimized(y, size)
            avg = calculate_average(y, size)
            plt.plot(avg, label="B = " + str(q))

    plt.xlabel('Insert')
    plt.ylabel('Block Transfers')
    plt.title('Inserting $10^7$ items in B-Tree')
    #plt.title('Average number of block transfers for deleting 10 million elements in B-Tree')
    plt.legend(loc='upper left')
    plt.show()

# Example usage:
file_paths = ['../B-Tree/data_10mil_50.dat']
#file_paths = ['../B-Tree/data_10mil_50.dat', '../B-Tree/data_ins_10mil_100.000000.dat', '../B-Tree/data_ins_10mil_400.000000.dat', '../B-Tree/data_ins_10mil_700.000000.dat',
#              '../B-Tree/data_ins_10mil_1000.000000.dat']
# file_paths = ['../B-Tree/data_10mil_remove_50.dat', '../B-Tree/data_del_10mil_100.000000.dat', '../B-Tree/data_del_10mil_400.000000.dat', '../B-Tree/data_del_10mil_700.000000.dat',
#                '../B-Tree/data_del_10mil_1000.000000.dat']
#file_paths = ['../B-Tree/data_flushCasc2_50.dat']
#size = 1000
size=1
B = [50]
#b = [50, 100, 400, 700, 1000]
plt.rcParams.update({'font.size': 18})
plot_data_from_files(file_paths, B)

