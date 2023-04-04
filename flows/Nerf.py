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
radius = 1
speed = 1

i = 0


def init():
    print("Hey, I'm Python!")


# oldH = 0
# oldV = 0

# def loadModel(path):
#     loadModel(path)


def screenshot(h, v):
    x = radius * math.sin(2*math.pi*h/360) * math.cos(2*math.pi*v/360)
    y = radius * math.sin(2*math.pi*v/360)
    z = radius * math.cos(2*math.pi*h/360) * math.cos(2*math.pi*v/360)

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

    escript.eval("""
        var cam = PADrend.getActiveCamera();
        cam.setWorldPosition([{x}, {y}, {z}]);
    """.format(x=x, y=y, z=z))

    return r, np.array(escript.eval("PADrend.getActiveCamera().getWorldTransformationMatrix().toArray()"))


def sample(count, filename):
    count = int(count)

    images = np.zeros((count, res, res, 3))
    poses = np.zeros((count, 4, 4))

    for i in range(count):
        image, pose = screenshot(random.randint(0, 359), random.randint(0, 30))
        
        images[i] = image.reshape((res, res, 3)) / 256
        poses[i] = pose.reshape((4, 4)) / radius

    # return images, poses

    np.savez_compressed(filename, images=images.astype("float32"), poses=poses.astype("float32"), focal=20)
    print("sampled", count, "poses to", filename)


def render():
    global i

    # h = 360 * math.sin(2 * math.pi * i / 360)
    h = i
    v = 30 * math.cos(2 * math.pi * i / 360)

    # h = 0
    # v = 0

    i += speed

    if model == None:
        r, _ = screenshot(h, v)
    else:
        r = nerfRender(h, v, 3) * 256

    return r




IN_COLAB = False

# if IN_COLAB:
#     %tensorflow_version 1.x

import os, sys
import time
from pathlib import Path
import tensorflow as tf
tf.compat.v1.enable_eager_execution()

from tqdm import tqdm_notebook as tqdm
import numpy as np
# import matplotlib.pyplot as plt

# TODO 
model = None
H, W, focal = None, None, None

N_samples = 64


def loadModel(path):
    global H, W, focal
    H = 100
    W = 100
    focal = 20
    
    global model
    print("load", str(Path(__file__).parent.absolute()) + '/' + path)
    model = tf.saved_model.load(str(Path(__file__).parent.absolute()) + '/' + path)


def posenc(x):
  rets = [x]
  for i in range(L_embed):
    for fn in [tf.sin, tf.cos]:
      rets.append(fn(2.**i * x))
  return tf.concat(rets, -1)

L_embed = 6
embed_fn = posenc
# L_embed = 0
# embed_fn = tf.identity

def init_model(D=8, W=256):
    relu = tf.keras.layers.ReLU()    
    dense = lambda W=W, act=relu : tf.keras.layers.Dense(W, activation=act)

    inputs = tf.keras.Input(shape=(3 + 3*2*L_embed)) 
    outputs = inputs
    for i in range(D):
        outputs = dense()(outputs)
        if i%4==0 and i>0:
            outputs = tf.concat([outputs, inputs], -1)
    outputs = dense(4, act=None)(outputs)
    
    model = tf.keras.Model(inputs=inputs, outputs=outputs)
    return model

def get_rays(H, W, focal, c2w):
    i, j = tf.meshgrid(tf.range(W, dtype=tf.float32), tf.range(H, dtype=tf.float32), indexing='xy')
    dirs = tf.stack([(i-W*.5)/focal, -(j-H*.5)/focal, -tf.ones_like(i)], -1)
    rays_d = tf.reduce_sum(dirs[..., np.newaxis, :] * c2w[:3,:3], -1)
    rays_o = tf.broadcast_to(c2w[:3,-1], tf.shape(rays_d))
    return rays_o, rays_d

def render_rays(network_fn, rays_o, rays_d, near, far, N_samples, rand=False):

    def batchify(fn, chunk=1024*32):
        return lambda inputs : tf.concat([fn(inputs[i:i+chunk]) for i in range(0, inputs.shape[0], chunk)], 0)
    
    # Compute 3D query points
    z_vals = tf.linspace(near, far, N_samples) 
    if rand:
      z_vals += tf.random.uniform(list(rays_o.shape[:-1]) + [N_samples]) * (far-near)/N_samples
    pts = rays_o[...,None,:] + rays_d[...,None,:] * z_vals[...,:,None]
    
    # Run network
    pts_flat = tf.reshape(pts, [-1,3])
    pts_flat = embed_fn(pts_flat)
    raw = batchify(network_fn)(pts_flat)
    raw = tf.reshape(raw, list(pts.shape[:-1]) + [4])
    
    # Compute opacities and colors
    sigma_a = tf.nn.relu(raw[...,3])
    rgb = tf.math.sigmoid(raw[...,:3]) 
    
    # Do volume rendering
    dists = tf.concat([z_vals[..., 1:] - z_vals[..., :-1], tf.broadcast_to([1e10], z_vals[...,:1].shape)], -1) 
    alpha = 1.-tf.exp(-sigma_a * dists)  
    weights = alpha * tf.math.cumprod(1.-alpha + 1e-10, -1, exclusive=True)
    
    rgb_map = tf.reduce_sum(weights[...,None] * rgb, -2) 
    depth_map = tf.reduce_sum(weights * z_vals, -1) 
    acc_map = tf.reduce_sum(weights, -1)

    return rgb_map, depth_map, acc_map

def train(path):
    data = np.load(str(Path(__file__).parent.absolute()) + '/' + path)
    images = data['images']
    poses = data['poses']
    focal = data['focal']
    global H, W
    H, W = images.shape[1:3]
    print(images.shape, poses.shape, focal)

    testimg, testpose = images[101], poses[101]
    images = images[:100,...,:3]
    poses = poses[:100]

    # plt.imshow(testimg)
    # plt.show()

    model = init_model()
    optimizer = tf.keras.optimizers.Adam(5e-4)

    N_iters = 1000
    psnrs = []
    iternums = []
    i_plot = 25

    t = time.time()
    for i in range(N_iters+1):
        # random img
        img_i = np.random.randint(images.shape[0])
        target = images[img_i]
        pose = poses[img_i]
        rays_o, rays_d = get_rays(H, W, focal, pose)
        with tf.GradientTape() as tape:
            rgb, depth, acc = render_rays(model, rays_o, rays_d, near=2., far=6., N_samples=N_samples, rand=True)
            loss = tf.reduce_mean(tf.square(rgb - target))
        gradients = tape.gradient(loss, model.trainable_variables)
        optimizer.apply_gradients(zip(gradients, model.trainable_variables))
        
        # TODO visualize
        # if i%i_plot==0:
        #     print(i, (time.time() - t) / i_plot, 'secs per iter')
        #     t = time.time()
            
        #     # Render the holdout view for logging
        #     rays_o, rays_d = get_rays(H, W, focal, testpose)
        #     rgb, depth, acc = render_rays(model, rays_o, rays_d, near=2., far=6., N_samples=N_samples)
        #     loss = tf.reduce_mean(tf.square(rgb - testimg))
        #     psnr = -10. * tf.math.log(loss) / tf.math.log(10.)

        #     psnrs.append(psnr.numpy())
        #     iternums.append(i)
            
        #     plt.figure(figsize=(10,4))
        #     plt.subplot(121)
        #     plt.imshow(rgb)
        #     plt.title(f'Iteration: {i}')
        #     plt.subplot(122)
        #     plt.plot(iternums, psnrs)
        #     plt.title('PSNR')
        #     plt.show()

    print('Done')


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


def nerfRender(theta, phi, radius):

    print("render", theta, phi, radius)

    c2w = pose_spherical(theta, phi, radius)
    rays_o, rays_d = get_rays(H, W, focal, c2w[:3,:4])
    rgb, depth, acc = render_rays(model, rays_o, rays_d, near=2., far=6., N_samples=N_samples)
    img = np.clip(rgb,0,1)

    print("rendered")
    
    return img

    # plt.figure(2, figsize=(20,6))
    # plt.imshow(img)
    # plt.show()
