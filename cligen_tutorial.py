#!/usr/bin/env python
#
#  PyCLIgen tutorial application
#
# Copyright (C) 2014-2015 Benny Holmgren
#
#  Translation to Python of cligen_tutorial.c included in CLIgen distribution
#  by Olof Hagsand, https://github.com/olofhagsand/cligen.
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

import sys, getopt
from cligen import *

def hello(cgen, vars, arg):
    'This callback just prints the function argument'

    cgen.output(sys.stdout, "{:s}\n".format(str(arg)))
    return 0
    

def cb(cgen, vars, arg):
    'This is a generic callback printing the variable vector and argument'

    cgen.output(sys.stdout, 'variables:\n');
    for i, cv in enumerate(vars):
        name = cv.name_get()
        if name is None:
            name = "(null)"
        cgen.output(sys.stdout, '\t{:d} name:{:s} type:{:s} value:{:s}\n'.format(
                i, 
                name,
                cv.type2str(),
                str(cv)))
    
    if arg is not None:
        cgen.output(sys.stdout, '\targument: {:s}\n'.format(str(arg)))

    return 0


def letters(cgen, vars, arg):
    'An example of a callback handling a complex syntax'

    if 'ca' in vars:
        cgen.output(sys.stdout, "{:s}\n".format(str(vars['ca'])))
    if 'int' in vars:
        cgen.output(sys.stdout, "{:s}\n".format(str(vars['int'])))
    if 'cb' in vars:
        cgen.output(sys.stdout, "{:s}\n".format(str(vars['cb'])))
    if 'dd' in vars:
        cgen.output(sys.stdout, "{:s}\n".format(str(vars['dd'])))
    if 'ee' in vars:
        cgen.output(sys.stdout, "{:s}\n".format(str(vars['ee'])))

    return 0
        

def secret(cgen, vars, arg):
    'This callback changes the prompt to the variable setting'

    cgen.output(sys.stdout, 'This is a hidden command: {:s}\n'.format(str(arg)))
    return 0;




def setprompt(cgen, vars, arg):
    'This callback changes the prompt to the variable setting'
    
    if 'new' in vars:
        cgen.prompt_set(str(vars['new']))

    return 0;




def Quit(cgen, vars, arg):
    'request quitting the CLI'

    cgen.exiting_set(1);
    return 0

def unknown(cgen, vars, arg):
    'Command without assigned callback'
    
    cgen.output(sys.stdout,
                'The command has no assigned callback: {:d}\n'.format(vars[0]))
    return 0;


def interface(cgen, func, cvec, arg):
    return [ { 'command':'eth0', 'help':'Interface eth0' }, 
             { 'command':'eth1', 'help':'Interface eth1' } 
           ]



def usage(cgen, argv):
    cgen.output(sys.stdout, 'Usage: {:s} [-h][-f <filename>][-s <syntax>]\n'.format(sys.argv[0]))
    sys.exit(2)

    

def main(argv):
    syntax = None
    infile = None

    c = CLIgen()

    try:
        opts, args = getopt.getopt(argv,"hf:s:")
    except getopt.GetoptError:
        usage(c, argv)

    for opt, arg in opts:
      if opt == '-h':
          usage(c, argv)
          sys.exit()
      elif opt in ("-s"):
         syntax = arg
      elif opt in ("-f"):
         infile = arg

    if syntax is not None:
        pt = ParseTree(c, syntax=syntax)
    elif infile is not None:
        pt = ParseTree(c, file=infile)
    else:
        usage(c, argv)

    if 'prompt' in pt.globals():
        c.prompt_set(pt.globals()['prompt'])
    if 'comment' in pt.globals():
        c.comment_set(pt.globals()['comment'])
    if 'tabmode' in pt.globals():
        c.tabmode_set(int(pt.globals()['tabmode']))

    try:
        treename = pt.globals()['name']
    except KeyError:
        treename = 'tutorial'
    c.tree_add(treename, pt)
    c.tree_active_set(treename)

    c.output(sys.stdout, 'Syntax:\n')
    c.output(sys.stdout, pt)

    c.eval()

    
if __name__ == "__main__":
    main(sys.argv[1:])
