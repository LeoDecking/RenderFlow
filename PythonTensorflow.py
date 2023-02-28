import numpy as np
import tensorflow as tf

def unload():
    # print("unload model, by Python")
    global model, shape, input_name, output_name
    model=None
    shape=None
    input_name=None
    output_name=None

    tf.keras.backend.clear_session()

def loadModel(path, s, i, o):
    print("try to load: " + path)
    global model, shape, input_name, output_name

    shape = np.array([1, *s]).astype("int")
    input_name = i
    output_name = o

    model = tf.saved_model.load(path)
    print("p: model loaded")


def predict(i):    
    a = i.reshape(shape).astype("float32")
    tn = tf.convert_to_tensor(a)

    r = model.signatures[input_name](tn)[output_name] if input_name != "void" else model(tn)

    return r.numpy()
