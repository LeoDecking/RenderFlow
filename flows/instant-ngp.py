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

# TODO custom output res

res = 512




def init():
    print("Hey, I'm Python!")
    loadSnapshot()


def loadSnapshot(path="instant-ngp/data/nerf/padrend/base.ingp"):
    print("load", str(Path(__file__).parent.absolute()) + '/' + path)
    testbed.load_snapshot(str(Path(__file__).parent.absolute()) + '/' + path)
    


# def screenshot(h, v, rd=radius):
#     p = pose_spherical(h, v, rd)
#     p = np.concatenate(p[:])
#     p1 = np.concatenate([-p[0:4], p[8:12], p[4:8], p[12:16]]).flatten().tolist()

#     escript.eval("""
#         PADrend.getActiveCamera().getParent().getParent().setMatrix(new Geometry.Matrix4x4({m}));
#         // return PADrend.getActiveCamera().getWorldTransformationMatrix();
#     """.format(m=p1))

#     r = escript.screenshot(res, res)

#     return r, p

# def setAngles(t, p, r):
#     global theta, phi, radius
#     theta=t
#     phi=p
#     radius=r

# def sample(count, filename):
#     count = int(count)

#     images = np.zeros((count, res, res, 3))
#     poses = np.zeros((count, 4, 4))

#     for i in range(count):

#         image, pose = screenshot(random.randint(0, 359), random.randint(-90, 0))
        
#         images[i] = image.reshape((res, res, 3)) / 256
#         p = pose.reshape((4, 4))

#         poses[i] = p


#     print(images.shape, poses.shape)

#     np.savez_compressed(filename, images=images.astype("float32"), poses=poses.astype("float32"), focal=130)
#     print("sampled", count, "poses to", filename)

# def sampleInstant(count, directory):

#     if Path(directory).exists():
#         print("directory already exists!")
#         return
#     os.makedirs(directory + "/images")

#     count = int(count)

#     images = np.zeros((count, res, res, 3))
#     poses = np.zeros((count, 4, 4))

#     for i in range(count):

#         image, pose = screenshot(random.randint(0, 359), random.randint(-90, 0))
        
#         cv2.imwrite(directory + "/images/" + str(i+10001)[-4:] + ".jpg", cv2.cvtColor(np.float32(image.reshape((res, res, 3))), cv2.COLOR_RGB2BGR))
#         images[i] = image.reshape((res, res, 3)) / 256
#         p = pose.reshape((4, 4))

#         poses[i] = p

#     print(images.shape, poses.shape)

    
#     print("sampled", count, "poses to", directory)



def render(prerender):
    m = escript.eval("""
                var cam = PADrend.getActiveCamera();
                return cam.getWorldTransformationMatrix().toArray()
            """)
        # r = cv2.resize(r, (res, res))
    # print(matrix)
    m = np.array(m)
    m = np.concatenate([m[0:4], m[8:12], m[4:8], m[12:16]])
    m = np.reshape(m, (4, 4))
    # print(matrix)
    testbed.set_nerf_camera_matrix(np.matrix(m)[:-1,:])
    r = testbed.render(res, res, 1, True) * 256

    return r[:, :, :3]
