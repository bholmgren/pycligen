/* 
 * py_cligen_pt.c
 *
 * Copyright (C) 2014 Benny Holmgren
 *
 * This file is part of PyCLIgen.
 *
 * PyCLIgen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 *  PyCLIgen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along wth PyCLIgen; see the file LICENSE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <Python.h>

#include <cligen/cligen.h>

#include "pycligen.h"

typedef struct {
    PyObject_HEAD
    parse_tree pt;
    char *name;      /* Name of ParseTree, set by _cligen.tree_add() */
    PyObject *globals;
} ParseTree;

static void
ParseTree_dealloc(ParseTree* self)
{
    /* Free parse_tree ??? */
    Py_XDECREF(self->globals);
    free(self->name);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
ParseTree_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return (PyObject *)type->tp_alloc(type, 0);
}

static int
ParseTree_init(ParseTree *self, PyObject *args, PyObject *kwds)
{
    FILE *f = NULL;
    char *file = NULL;
    char *syntax = NULL;
    PyObject *cgen;
    cvec *globals_vec = NULL;
    int retval = -1;
    cg_var *cv;


    static char *kwlist[] = {"CLIgen", "syntax", "file", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ss", kwlist, &cgen, &syntax, &file))
	return -1;
    
    memset(&self->pt, 0, sizeof(self->pt));
    if ((globals_vec = cvec_new(0)) == NULL)
	return -1;

    self->name = NULL;
    
    if ((self->globals = PyDict_New()) == NULL)
	goto done;
    
    if (syntax != NULL) {
	if (cligen_parse_str(CLIgen_cligen_handle(cgen),
			     syntax,
			     "__syntax__",
			     &self->pt,
			     globals_vec) < 0)
	    goto done;
    } else if (file != NULL) {
	if ((f = fopen(file, "r")) == NULL) {
	    switch(errno) {
	    case EACCES:
		PyErr_Format(PyExc_PermissionError, "%s: '%s'", strerror(errno), file);
		goto done;
	    case ENOENT:
		PyErr_Format(PyExc_FileNotFoundError, "%s: '%s'", strerror(errno), file);
		goto done;
	    default:
		PyErr_Format(PyExc_IOError, "%s: '%s'", strerror(errno), file);
		goto done;
	    }
	}
	if (cligen_parse_file(CLIgen_cligen_handle(cgen),
			      f,
			      file,
			      &self->pt,
			      globals_vec) < 0)
	    goto done;
	fclose(f);
	f = NULL;
    }
    
    if (cligen_callback_str2fn(self->pt, CLIgen_str2fn, self) < 0)     
	goto done;

    if (cligen_expand_str2fn(self->pt, CLIgen_expand_str2fn, self) < 0)
        goto done;
    
    /* Populate globals dictionary */
    for (cv = NULL; (cv = cvec_each(globals_vec, cv)); ) {
	if (PyDict_SetItemString(self->globals, cv_name_get(cv), 
				 StringFromString(cv_string_get(cv))) < 0)
	    goto done;
    }
    
    retval = 0;

done:
    if (f)
	fclose(f);
    if (globals_vec)
	cvec_free(globals_vec);
    if (retval != 0 && self->pt.pt_vec != NULL)
	cligen_parsetree_free(self->pt, 1);
	
    return retval;
}

static PyObject *
ParseTree_globals(ParseTree *self)
{
    Py_INCREF(self->globals);
    return self->globals;
}


static PyObject *
ParseTree_repr(PyObject *self)
{
    FILE *f;
    PyObject *str;
    char *buf;
    size_t siz;

    if ((f = open_memstream(&buf, &siz)) == NULL) {
	PyErr_Format(PyExc_IOError, "%s", strerror(errno));
	return NULL;
    }

    cligen_print(f, ((ParseTree *)self)->pt, 1); 
    fclose(f);
    if (buf) {
	str = StringFromString(buf);
	free(buf);

    } else {
	Py_INCREF(Py_None);
	str = Py_None;
    }    
    
    return str;    
}

#if 0
static PyObject *
ParseTree_fprint(ParseTree *self, PyObject *args)
{
    FILE *f;
    int fd;
    PyObject *file;
    PyObject *fileno;

    if (!PyArg_ParseTuple(args, "O", &file))
        return NULL;
    if ((fileno = PyObject_CallMethod(file, "fileno", NULL)) == NULL)
	return NULL;
    
    fd = PyLong_AsLong(fileno);
    Py_DECREF(fileno);

    if ((f = fdopen(fd, "a")) != NULL)
	cligen_print(f, self->pt, 1); 
    
    return PyLong_FromLong(0);
}
#endif

static PyMethodDef ParseTree_methods[] = {

#if 0
    {"__init__", (PyCFunction)ParseTree_init, METH_VARARGS, 
     "ParseTree constructor\n\n   Arguments\n      cgen\t- A CLIgen instance\n      syntax-spec\t- A CLIgen syntax specification.\n\n   The syntax specification is a named argument as follows.\n      string='<syntax format>'\t- String containing syntax specification\n      file='<filename>'\t- A file containing syntax specification\n   If argument is unnamed, it implies a string argument"
    },
#endif

    {"globals", (PyCFunction)ParseTree_globals, METH_NOARGS, 
     "Get dictionary of ParseTree global variables"
    },

#if 0
    {"print", (PyCFunction)ParseTree_fprint, METH_VARARGS, 
     "Print CLIgen parse-tree to file, brief or detailed."
    },
#endif

   {NULL}  /* Sentinel */
};

PyTypeObject ParseTree_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_cligen.ParseTree",       /* tp_name */
    sizeof(ParseTree),         /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)ParseTree_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    ParseTree_repr,            /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    ParseTree_repr,            /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "ParseTree objects",       /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    ParseTree_methods,         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ParseTree_init,      /* tp_init */
    0,                         /* tp_alloc */
    ParseTree_new,                 /* tp_new */
};

int
ParseTree_init_object(PyObject *m)
{
    if (PyType_Ready(&ParseTree_Type) < 0)
        return -1;

    Py_INCREF(&ParseTree_Type);
    PyModule_AddObject(m, "ParseTree", (PyObject *)&ParseTree_Type);

    return 0;
}


parse_tree *
ParseTree_pt(PyObject *pto)
{
    ParseTree *pt = (ParseTree *)pto;

    return &pt->pt;
}

/*
 * Module internal function to set ParseTree name
 */ 
int
ParseTree_name_set(PyObject *obj, const char *name)
{
    char *new;
    ParseTree *Pt = (ParseTree *)obj;
    
    if ((new = strdup(name)) == NULL)
	return -1;
    
    if (Pt->name)
	free(Pt->name);

    Pt->name = new;

    return 0;
}

/*
 * Module internal function to get ParseTree name
 */ 
char *
ParseTree_name(PyObject *obj)
{
    ParseTree *Pt = (ParseTree *)obj;
    
    return Pt->name;
}
