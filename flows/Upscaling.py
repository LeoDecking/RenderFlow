import numpy as np
import tensorflow as tf
from pathlib import Path
# import escript

# import time

print("Hello World, by Python!")
model = tf.saved_model.load(str(Path(__file__).parent.absolute()) + '/upscaleModel/saved/')
print("P: Model loaded!")


def render(prerender):
    a = prerender.reshape(1, 324, 324, 3).astype("float32") / 256

    tn = tf.convert_to_tensor(a)
    r = model.signatures["serving_default"](tn)["out"]
    r = np.clip(r.numpy()*256, 0, 255)

    return r
