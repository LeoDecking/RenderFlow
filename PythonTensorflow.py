import numpy as np
import tensorflow as tf


def loadModel(path, s, i, o):
    print("try to load: " + path)
    global model, shape, input_name, output_name
    shape = np.array([1, *s]).astype("int")
    input_name = i
    output_name = o

    model = tf.saved_model.load(path)
    print("model loaded")



def predict(i):
    a = i.reshape(shape).astype("float32")
    tn = tf.convert_to_tensor(a)

    r = model.signatures[input_name](tn)[output_name] if input_name != "void" else model(tn)

    return r.numpy()
