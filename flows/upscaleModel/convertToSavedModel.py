# from https://stackoverflow.com/a/67028058/6004362

import shutil
import tensorflow.compat.v1 as tf
from tensorflow.python.saved_model import signature_constants
from tensorflow.python.saved_model import tag_constants

export_dir = 'saved'
graph_pb = 'model.pb'
# Clear out folder
shutil.rmtree(export_dir, ignore_errors=True)

builder = tf.saved_model.builder.SavedModelBuilder(export_dir)

with tf.io.gfile.GFile(graph_pb, "rb") as f:
    graph_def = tf.GraphDef()
    graph_def.ParseFromString(f.read())

sigs = {}

with tf.Session(graph=tf.Graph()) as sess:
    # Prepare input and outputs of model
    tf.import_graph_def(graph_def, name="")
    g = tf.get_default_graph()
    inp = g.get_tensor_by_name("input:0")
    out = g.get_tensor_by_name("Identity:0")

    sigs[signature_constants.DEFAULT_SERVING_SIGNATURE_DEF_KEY] = \
        tf.saved_model.signature_def_utils.predict_signature_def(
            {"in": inp}, {"out": out})

    builder.add_meta_graph_and_variables(sess,
                                         [tag_constants.SERVING],
                                         signature_def_map=sigs)

builder.save()