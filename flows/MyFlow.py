import numpy as np
import tensorflow as tf
import escript

cache = False

print("Hello World, by Python!")
model = tf.saved_model.load('../extPlugins/RenderFlow/model/')
print("Model loaded!")

lx, lz, lr = None, None, None

def render(prerender):
    x, z = escript.eval("""
        var cam = PADrend.getActiveCamera();
        return [cam.getWorldPosition().getX(), cam.getWorldPosition().getZ()];
    """)

    global lx, lz, lr
    if cache and x == lx and z == lz:
        return lr

    # print("x: " + str(x) + " z: " + str(z))

    r = model(np.array([[x / 10, z / 10]], dtype="float32")).numpy()
    lx, lz, lr = x, z, r
    return r
