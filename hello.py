from cligen import *

def cb(cligen, vr, arg):
    print("cvec = ",vr)
    print("arg = ",arg)
    return 0

c = CLIgen('hello("Greet the world") world, cb("Hello World!");')
c.eval()

