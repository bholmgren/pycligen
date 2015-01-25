#!/usr/bin/env python
#
#  pycligen hello world
#
# Copyright (C) 2014-2015 Benny Holmgren
#
#  This file is part of PyCLIgen.
#
#  PyPyCLIgen is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PyCLIgen is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with PyCLIgen; see the file LICENSE.

from cligen import *

def cb(cgen, vr, arg):
    print ("cvec = ",vr)
    print ("arg = ",arg)
    return 0

c = CLIgen('hello("Greet the world") world, cb("Hello World!");')
c.eval()

