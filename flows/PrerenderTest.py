import numpy as np
import escript


def init():
    print("Hello World, by Python!")


def finalize():
    print("Goodbye, by Python!")

def render(prerender):
    a = np.flip(prerender)
    return a