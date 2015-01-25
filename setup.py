#! /usr/bin/env python
#
#  PyCLIgen tutorial application
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

from distutils.core import setup, Extension

long_desc = """The PyCLIgen bindings implements a Python API for the CLIgen library by Olof Hagsand."""

setup(name = "pycligen",
      version = "0.1",
      description = "Python bindings for CLIgen",
      long_description=long_desc,
      author = "Benny Holmgren",
      author_email = "benny@holmgren.id.au",
      license = "GPLv3",
      url = "https://github.com/bholmgren/cligen",
      py_modules=['cligen'],
      ext_modules = [
        Extension(
            "_cligen",
            ["pycligen.c", "pycligen_cv.c", "pycligen_pt.c", "pycligen_cvec.c"],
            libraries=['cligen','python2.7'],
            )
        ]
)
