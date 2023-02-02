import numpy as np
import tensorflow as tf
# import escript

# import time

print("Hello World, by Python!")
model = tf.saved_model.load('../extPlugins/RenderFlow/upscaleModel/saved/')
print("Model loaded!")


def render(prerender):

    # t = time.time()

    # a = tf.convert_to_tensor
    a = prerender.reshape(1, 324, 324, 3).astype("float32") / 256
    # r = frozen_func(tf.convert_to_tensor(a))
    # print(r)

    # print("reshape: " + str(time.time()-t))
    # t = time.time()

    tn = tf.convert_to_tensor(a)
    # print("tensor: " + str(time.time()-t))
    # t = time.time()

    r = model.signatures["serving_default"](tn)["out"]

    # print("model: " + str(time.time()-t))
    # t = time.time()

    r = np.clip(r.numpy()*256, 0, 255)

    # print("numpy: " + str(time.time()-t))
    # t = time.time()

    return r
