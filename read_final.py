import numpy as np
import os

data = np.load("final_embeddings.npy")

dictionary_size = 50000

print("Data sizes: ", data.size, data.shape, data.ndim)

subset = data[500:512]

print(subset)
