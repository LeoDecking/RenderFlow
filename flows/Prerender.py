import numpy as np

def init():
    print("init Prerender")

def render(prerender):
    a = np.flip(prerender)
    return a