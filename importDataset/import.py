import numpy as np
import json

folder = "drums"
res = 100

f = open(folder + "/transforms_train.json",)
j = json.load(f)
f.close()

count = len(j["frames"])

images = np.zeros((count, res, res, 3))
poses = np.zeros((count, 4, 4))

print(count)