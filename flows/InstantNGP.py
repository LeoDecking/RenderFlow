# https://colab.research.google.com/github/bmild/nerf/blob/master/tiny_nerf.ipynb

from errno import ESRCH
import numpy as np
import cv2
import tensorflow as tf
from pathlib import Path
import shutil
import os
import random
import sys
import json

import escript


sys.path.append(str(Path(__file__).parent.absolute()) + "/instant-ngp/scripts/")
from common import *
from scenes import *
import pyngp as ngp


testbed = ngp.Testbed()
testbed.root_dir = str(Path(__file__).parent.absolute()) + '/instant-ngp'

res = 512


def loadSnapshot(path="instant-ngp/data/nerf/temp/base.ingp"):
    print("load snapshot:", str(Path(__file__).parent.absolute()) + '/' + path)
    testbed.load_snapshot(str(Path(__file__).parent.absolute()) + '/' + path)

def saveSnapshot(path="instant-ngp/data/nerf/temp/base.ingp"):
    testbed.save_snapshot(str(Path(__file__).parent.absolute()) + '/' + path, False)
    print("saved snapshot:", str(Path(__file__).parent.absolute()) + '/' + path)
    
def loadTransforms(path="instant-ngp/data/nerf/temp/transforms.json"):
    print("load transforms:", str(Path(__file__).parent.absolute()) + '/' + path)
    testbed.shall_train = False
    testbed.load_training_data(str(Path(__file__).parent.absolute()) + '/' + path)
    print(testbed.nerf.training.dataset)


def createTransforms(frames=100, path="instant-ngp/data/nerf/temp"):
    dirpath = Path(str(Path(__file__).parent.absolute()) + '/' + path)

    if dirpath.exists() and dirpath.is_dir():
        if path == "instant-ngp/data/nerf/temp":
            print("deleted non-empty temp directory")
            shutil.rmtree(dirpath)
        else:
            print("directory already exists!")
            return
    
    if type(frames) == float:
        frames = [pose_spherical(random.randint(0, 359), random.randint(-90, 0), 4) for _ in range(int(frames))]
    else:
        frames = np.array(frames)
        frames = frames.reshape((-1, 4, 4))
        for i in range(len(frames)):
            frames[i][0] = -frames[i][0]
            frames[i][[1, 2]] = frames[i][[2, 1]]


    f = open(str(Path(__file__).parent.absolute()) + "/InstantNGP_default_transforms.json")
    j = json.load(f)
    f.close()

    os.makedirs(str(Path(__file__).parent.absolute())+ '/' + path + "/images")

    for i in range(len(frames)):
        p = np.concatenate(frames[i][:])
        p1 = np.concatenate([-p[0:4], p[8:12], p[4:8], p[12:16]]).flatten().tolist()

        escript.eval("""
            PADrend.getActiveCamera().getParent().getParent().setMatrix(new Geometry.Matrix4x4({m}));
        """.format(m=p1))

        image = escript.screenshot(res, res)

        cv2.imwrite(str(Path(__file__).parent.absolute()) + '/' + path + "/images/" + str(i+10001)[-4:] + ".jpg", cv2.cvtColor(np.float32(image.reshape((res, res, 3))), cv2.COLOR_RGB2BGR))
        j["frames"].append({
            "file_path": "./images/" + str(i+10001)[-4:] + ".jpg",
            "sharpness": 987.4263954144989, # just took from example transforms.json,
            "transform_matrix": [p[0:4].flatten().tolist(), p[4:8].flatten().tolist(), p[8:12].flatten().tolist(), p[12:16].flatten().tolist()]
        })

    with open(str(Path(__file__).parent.absolute())+ '/' + path + "/transforms.json", "w") as f:
        json.dump(j, f)

    print("Written",len(frames),"transforms to",str(Path(__file__).parent.absolute())+ '/' + path + "/transforms.json")
    


gen = None
N = 0

def startTraining(iters=100):
    global N
    N += int(iters)

    global gen
    gen = train_generator()


def train_generator():
    global gen

    testbed.shall_train = True

    while testbed.frame():
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



trans_t = lambda t : tf.convert_to_tensor([
    [1,0,0,0],
    [0,1,0,0],
    [0,0,1,t],
    [0,0,0,1],
], dtype=tf.float32)

rot_phi = lambda phi : tf.convert_to_tensor([
    [1,0,0,0],
    [0,tf.cos(phi),-tf.sin(phi),0],
    [0,tf.sin(phi), tf.cos(phi),0],
    [0,0,0,1],
], dtype=tf.float32)

rot_theta = lambda th : tf.convert_to_tensor([
    [tf.cos(th),0,-tf.sin(th),0],
    [0,1,0,0],
    [tf.sin(th),0, tf.cos(th),0],
    [0,0,0,1],
], dtype=tf.float32)


def pose_spherical(theta, phi, radius):
    c2w = trans_t(radius)
    c2w = rot_phi(phi/180.*np.pi) @ c2w
    c2w = rot_theta(theta/180.*np.pi) @ c2w
    c2w = np.array([[-1,0,0,0],[0,0,1,0],[0,1,0,0],[0,0,0,1]]) @ c2w
    return c2w