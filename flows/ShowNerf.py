# https://colab.research.google.com/github/bmild/nerf/blob/master/tiny_nerf.ipynb

from errno import ESRCH
import numpy as np
import tensorflow as tf
from pathlib import Path
import math
import random

import escript

# import TinyNerf as nerf

res = 100

offset = [0, 0, 0]
radius = 2
theta = 0
phi = 0

oldH=None
oldV=None
oldR=None
oldImg=None


speed =  0.005

i = 0


def init():
    print("Hey, I'm Python!")
    global images, poses
    data = np.load(str(Path(__file__).parent.absolute()) + '/' + 'tiny_nerf_data.npz')
    images = data['images']
    poses = data['poses']

def setSpeed(s):
    global speed
    speed = s

# oldH = 0
# oldV = 0

# def loadModel(path):
#     loadModel(path)


def screenshot(h, v, rd=radius):
    x = rd * math.sin(2*math.pi*h/360) * math.cos(2*math.pi*v/360)
    y = rd * math.sin(2*math.pi*v/360)
    z = rd * math.cos(2*math.pi*h/360) * math.cos(2*math.pi*v/360)

    # global oldH
    # global oldV

    escript.eval("""
        var cam = PADrend.getActiveCamera();
        cam.reset();
        cam.setWorldPosition([{x}, {y}, {z}]);

        cam.rotateLocal_deg({h}, [0, 1, 0]);
        cam.rotateLocal_deg({v}, [-1, 0, 0]);
    """.format(x=offset[0] + x, y=offset[1] + y, z=offset[2] + z, h=h, v=v))

    # oldH = h
    # oldV = v

    r = escript.screenshot(res, res)

    # escript.eval("""
    #     var cam = PADrend.getActiveCamera();

    #     cam.rotateLocal_deg({h}, [0, 1, 0]);
    #     //cam.rotateLocal_deg({v}, [1, 0, 0]);
    # """.format(x=x, y=y, z=z, h=360-h, v=360-v))

    # escript.eval("""
    #     var cam = PADrend.getActiveCamera();
    #     cam.setWorldPosition([{x}, {y}, {z}]);
    # """.format(x=x, y=y, z=z))

    return r, np.array(escript.eval("PADrend.getActiveCamera().getWorldTransformationMatrix().toArray()"))

def printMatrix(theta, phi, rd):
    p = screenshot(theta, phi, rd)[1]
    a = p #np.concatenate([-p[0:2], p[2:4], -p[8:10], p[10:12], p[4:6], -p[6:8], p[12:16]])
    print(a[0:4])
    print(a[4:8])
    print(a[8:12])
    print(a[12:16])

def setAngles(t, p, r):
    global theta, phi, radius
    theta=t
    phi=p
    radius=r

def sample(count, filename):
    count = int(count)

    images = np.zeros((count, res, res, 3))
    poses = np.zeros((count, 4, 4))

    for i in range(count):
        image, pose = screenshot(random.randint(0, 359), random.randint(-35, 35))
        
        images[i] = image.reshape((res, res, 3)) / 256
        p = pose.reshape((4, 4))

        # the order has to be changed, don't know why
        # poses[i] = np.concatenate([-p[0:2], p[2:4], -p[8:10], p[10:12], p[4:6], -p[6:8], p[12:16]])
        poses[i] = np.concatenate([-p[0:2], p[2:4], -p[8:10], p[10:12], p[4:6], -p[6:8], p[12:16]])

    # return images, poses

    np.savez_compressed(filename, images=images.astype("float32"), poses=poses.astype("float32"), focal=130)
    print("sampled", count, "poses to", filename)



def render(prerender):
    global i, oldH, oldV, oldR, oldImg

    i = i + speed

    if i >= len(images): i = 0
    
    # print(i, len(images))

    p = poses[math.floor(i)].flatten()
    # p1 = np.concatenate([-p[0:2], p[2:4], -p[8:10], p[10:12], p[4:6], -p[6:8], p[12:16]]).tolist()
    p1 = np.concatenate([-p[0:4], p[8:12], p[4:8], p[12:16]]).tolist()
    # print(p1)

    escript.eval("""
        PADrend.getActiveCamera().getParent().getParent().setMatrix(new Geometry.Matrix4x4({m}));
        // return PADrend.getActiveCamera().getWorldTransformationMatrix();
    """.format(m=p1))

    return images[math.floor(i)] * 256


