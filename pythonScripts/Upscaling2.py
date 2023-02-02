import numpy as np
import tensorflow as tf
# import escript

import time

print("Hello World, by Python!")


# from https://leimao.github.io/blog/Save-Load-Inference-From-TF2-Frozen-Graph/
def wrap_frozen_graph(graph_def, inputs, outputs, print_graph=False):
    def _imports_graph_def():
        tf.compat.v1.import_graph_def(graph_def, name="")

    wrapped_import = tf.compat.v1.wrap_function(_imports_graph_def, [])

    return wrapped_import.prune(
        tf.nest.map_structure(wrapped_import.graph.as_graph_element, inputs),
        tf.nest.map_structure(wrapped_import.graph.as_graph_element, outputs))


# Load frozen graph using TensorFlow 1.x functions
with tf.io.gfile.GFile("../extPlugins/RenderFlow/upscaleModel/model.pb", "rb") as f:
    graph_def = tf.compat.v1.GraphDef()
    loaded = graph_def.ParseFromString(f.read())

# Wrap frozen graph to ConcreteFunctions
frozen_func = wrap_frozen_graph(graph_def=graph_def,
                                inputs=["input:0"],
                                outputs=["Identity:0"],
                                print_graph=True)
# print(frozen_func)
# print(frozen_func((np.random.rand(1, 324, 324, 3).astype("float32"))))

def render(prerender):
    # a = tf.convert_to_tensor
    t = time.time()
    a = prerender.reshape(1, 324, 324, 3).astype("float32") / 256
    
    print("reshape: " + str(time.time()-t))
    t = time.time()

    tn = tf.convert_to_tensor(a)
    print("tensor: " + str(time.time()-t))
    t = time.time()


    r = frozen_func(tn)

    
    print("model: " + str(time.time()-t))
    t = time.time()

    r = np.clip(np.array(r)*256, 0, 255)

    print("numpy: " + str(time.time()-t))
    t = time.time()

    # print(r)
    return r