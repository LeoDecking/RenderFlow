# https://colab.research.google.com/github/bmild/nerf/blob/master/tiny_nerf.ipynb

from errno import ESRCH
import numpy as np
import cv2
import tensorflow as tf
from pathlib import Path
import math
import random
import sys

import escript


sys.path.append(str(Path(__file__).parent.absolute()) + "/instant-ngp/scripts/")
from common import *
from scenes import *
import pyngp as ngp # noqa


testbed = ngp.Testbed()
testbed.root_dir = str(Path(__file__).parent.absolute()) + '/instant-ngp'

# TODO custom output res

res = 512




def init():
    print("Hey, I'm Python!")
    # loadSnapshot()


def loadSnapshot(path="instant-ngp/data/nerf/padrend/base.ingp"):
    print("load snapshot:", str(Path(__file__).parent.absolute()) + '/' + path)
    testbed.load_snapshot(str(Path(__file__).parent.absolute()) + '/' + path)
    
def loadTransforms(path="instant-ngp/data/nerf/padrend/transforms.json"):
    print("load transforms:", str(Path(__file__).parent.absolute()) + '/' + path)
    testbed.shall_train = False
    testbed.load_training_data(str(Path(__file__).parent.absolute()) + '/' + path)
    print(testbed.nerf.training.dataset)

gen = None
N = 0

def startTraining(n_steps=100):
    global N
    N += int(n_steps)

    global gen
    gen = train_generator()


    # testbed.shall_train = False

def train_generator():
    global gen

    testbed.shall_train = True

    while testbed.frame():
        # if testbed.want_repl():
        #     repl(testbed)
        # What will happen when training is done?
        if testbed.training_step >= N:
            testbed.shall_train = False
            gen = None
            print("done")
            break

        if testbed.training_step % 25 == 0:
            yield _render()
    
    yield _render()


def render(prerender):
    if gen != None:
        return next(gen)
    

    

    # if testbed.training_step >= N:
    #     testbed.shall_train = False
    # print("training_step: ", testbed.training_step, ", N: ", N)

    return _render()

def _render():
    m = escript.eval("""
                var cam = PADrend.getActiveCamera();
                return cam.getWorldTransformationMatrix().toArray()
            """)

    m = np.array(m)
    m = np.concatenate([m[0:4], m[8:12], m[4:8], m[12:16]])
    m = np.reshape(m, (4, 4))
    testbed.set_nerf_camera_matrix(np.matrix(m)[:-1,:])
    r = testbed.render(res, res, 1, True) * 256
    
    return r[:, :, :3]