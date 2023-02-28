import numpy as np
import tensorflow as tf
from pathlib import Path
import escript

cache = False
model = None
lx, lz, lr = None, None, None

def init():
    print("Hello World, by Python!")
    global model
    model = tf.saved_model.load(str(Path(__file__).parent.absolute()) + '/model/')
    print("p: Model loaded!")


def finalize():
    print("Goodbye, by Python!")
    global model
    model = None


def render():
    x, z = escript.eval("""
        var cam = PADrend.getActiveCamera();
        return [cam.getWorldPosition().getX(), cam.getWorldPosition().getZ()];
    """)

    global lx, lz, lr
    if cache and x == lx and z == lz:
        return lr

    r = model(np.array([[x / 10, z / 10]], dtype="float32")).numpy()
    lx, lz, lr = x, z, r
    return r
