import numpy as np


def render():
    # a = np.ones([100* 100* 3])
    # for i in range(len(a)):
    #     a[i] = ((i / 3) + (i % 3) * 40) % 256

    a = np.ones([100, 100, 3])
    a *= 42
    a[30:, :, 0] *= 3
    a[60:, :, 0] *= 2
    a[60:, :, 2] *= 4
    a[:, :50, 1] *= 0
    a[30:, :70, 1] += 30
    
    return a