import escript

import numpy as np
import random

print("Hello World, by Python!")

print(escript.eval("""
    var a = [2, 3, 4];

    return {"a": a, "b": 42, [true, false]: void, "hu": [true, false, "Hallo", [2,3]]};
"""))

def render(prerender):
    a = np.ones([648, 648, 3])
    a *= 42
    a[300:, :, 0] *= 2
    a[600:, :, 0] *= 2
    a[:, :512, 1] *= 0
    
    return a