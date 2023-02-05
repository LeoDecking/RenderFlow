import numpy as np
import escript


print("Hello World, by Python!")
print(escript.eval("10000+23+42;"))
print(escript.eval("[1,2,3]"))

def render(prerender):
    a = np.flip(prerender)
    return a