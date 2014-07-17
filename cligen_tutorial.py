#!/usr/bin/python
#
#  PyCLIgen tutorial application
#
# Copyright (C) 2014 Benny Holmgren
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

    print(arg)
    return 0
    

def cb(cgen, vars, arg):
    'This is a generic callback printing the variable vector and argument'

    print('variables:');
    for i, cv in enumerate(vars):
        name = cv.name_get()
        if name is None:
            name = "(null)"
        print('\t{:d} name:{:s} type:{:s} value:{:s}'.format(
                i, 
                name,
                cv.type2str(),
                str(cv)))
    
    if arg is not None:
        print('\targument: {:s}'.format(str(arg)))

    return 0


def letters(cgen, vars, arg):
    'An example of a callback handling a complex syntax'

    if vars.haskey('ca'):
        print(vars['ca'])
    if vars.haskey('int'):
        print(vars['int'])
    if vars.haskey('cb'):
        print(vars['cb'])
    if vars.haskey('dd'):
        print(vars['dd'])
    if vars.haskey('ee'):
        print(vars['ee'])




def secret(cgen, vars, arg):
    'This callback changes the prompt to the variable setting'

    print('This is a hidden command: {:s}'.format(str(arg)))
    return 0;




def setprompt(cgen, vars, arg):
    'This callback changes the prompt to the variable setting'
    
    if vars.haskey('new'):
        cgen.prompt_set(str(vars['new']))
        return 0;




def Quit(cgen, vars, arg):
    'request quitting the CLI'

    cgen.exiting_set(1);
    return 0

def unknown(cgen, vars, arg):
    'Command without assigned callback'
    
    print('The command has no assigned callback: {:d}'.format(vars[0]))
    return 0;


# * str2fn
# * Example of static string to function mapper for the callback
# * functions above.
# * Better to use dlopen, mmap or some other more flexible scheme.
# */
#str2fn(char *name, void *arg, char **error)
#{
#    *error = NULL;
#    if (strcmp(name, "hello") == 0)
#        return hello;
#    if (strcmp(name, "cb") == 0)
#        return cb;
#    if (strcmp(name, "letters") == 0)
#        return letters;
#    if (strcmp(name, "secret") == 0)
#        return secret;
#    if (strcmp(name, "setprompt") == 0)
#        return setprompt;
#    if (strcmp(name, "quit") == 0)
#        return quit;
#
#    return unknown; /* allow any function (for testing) */
#}

#/*
# * This is an example of an expansion function. It is called every time
# * a variable of the form <expand> needs to be evaluated.
# * Note the mallocing of vectors which could probably be done in a
# * friendlier way.
# * Note also that the expansion is not very dynamic, a script or reading a file
# * would have introduced som more dynamics.
# */
#int
#cli_expand_cb(void *h, char *fn_str, cvec *vars, cg_var *cv, 
#	      int  *nr,
#	      char ***commands,     /* vector of function strings */
#	      char ***helptexts)   /* vector of help-texts */
#{
#    int n = 2;
#     /* Interface name expansion. */
#    *commands = calloc(n, sizeof(char*));
#    (*commands)[0] = strdup("eth0");
#    (*commands)[1] = strdup("eth1");
#    *nr = n;
#    *helptexts = calloc(n, sizeof(char*));
#    (*helptexts)[0] = strdup("Interface A");
#    (*helptexts)[1] = strdup("Interface B");
#    return 0;
#}
#
#static expand_cb *
#str2fn_exp(char *name, void *arg, char **error)
#{
#    return cli_expand_cb;
#}

def interface(cgen, func, cvec, arg):
    return [ { 'command':'eth0', 'help':'Interface eth0' }, 
             { 'command':'eth1', 'help':'Interface eth1' } 
           ]



def usage(argv):
    print('Usage: {:s} [-h][-f <filename>][-s <syntax>]'.format(sys.argv[0]))
    sys.exit(2)

    

def main(argv):
    syntax = None
    infile = None
    try:
        opts, args = getopt.getopt(argv,"hf:s:")
    except getopt.GetoptError:
        usage(argv)

    for opt, arg in opts:
      if opt == '-h':
          usage(argv)
          sys.exit()
      elif opt in ("-s"):
         syntax = arg
      elif opt in ("-f"):
         infile = arg

    c = CLIgen()
    if syntax is not None:
        pt = ParseTree(c, syntax=syntax)
    elif infile is not None:
        pt = ParseTree(c, file=infile)
    else:
        usage(argv)

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

    print('Syntax:')
    pt.print(sys.stdout)

#    if (cligen_expand_str2fn(pt, str2fn_exp, NULL) < 0)
#        return -1;

    c.eval()

    
if __name__ == "__main__":
    main(sys.argv[1:])
