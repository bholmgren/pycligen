/* 
 * py_cligen.c
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
#include "structmember.h"

#include <cligen/cligen.h>

#include "pycligen.h"
#include "pycligen_cv.h"
#include "pycligen_pt.h"
#include "pycligen_cvec.h"

#define CLIgen_MAGIC  0x8abe91a1

static CLIgen_handle
CLIgen_handle_init()
{
    CLIgen_handle h;
    
    if ((h = malloc(sizeof(*h))) == NULL)
	return NULL;

    memset(h, 0, sizeof(*h));
    h->ch_magic = CLIgen_MAGIC;

    return h;
}

static void CLIgen_handle_exit(CLIgen_handle h)
{
    free(h);
}

static int
CLIgen_callback(cligen_handle h, cvec *vars, cg_var *arg)
{
    char *func;
    CLIgen_handle ch = (CLIgen_handle)h;
    PyObject *self = (PyObject *)ch->ch_self;
    PyObject *Value = NULL;
    PyObject *Cvec = NULL;
    int retval = -1;
    PyObject *Arg = NULL;
    
    
    /* Get a Cvec instance */
    if ((Cvec = Cvec_from_cvec(self, vars)) == NULL) {
	fprintf(stderr, "Failed to create CLIgen.Cvec instance\n");
	return -1;
    }
    
    /* arg */
    if (arg)
	Arg = (PyObject *)CgVar_InstanceFromCv(arg);
    else {
	Py_INCREF(Py_None);
	Arg = Py_None;
    }
    
    
    /* Run callback */
    func = cligen_fn_str_get(ch->ch_cligen);
    Value =  PyObject_CallMethod(self, "_cligen_cb", "sOO", func, Cvec, Arg);
    if (PyErr_Occurred())
	PyErr_Print();
    if (Value) {
	retval = PyLong_AsLong(Value);
    }
    Py_DECREF(Arg);
    Py_DECREF(Cvec);
    Py_XDECREF(Value);

    return retval;
}

cg_fnstype_t *
CLIgen_str2fn(char *name, void *arg, char **error)
{
    return CLIgen_callback;
}

int
CLIgen_expand_cb(CLIgen_handle *h, char *func, cvec *vars, cg_var *arg, 
	      int  *nr,
	      char ***commands,     /* vector of function strings */
	      char ***helptexts)   /* vector of help-texts */
{
    int num;
    int i;
    CLIgen_handle ch = (CLIgen_handle)h;
    PyObject *self = (PyObject *)ch->ch_self;
    PyObject *Value = NULL;
    PyObject *Cvec = NULL;
    int retval = -1;
    PyObject *Arg = NULL;
    PyObject *iterator;
    PyObject *item;


    *nr = 0;

    /* Get a Cvec instance */
    if ((Cvec = Cvec_from_cvec(self, vars)) == NULL) {
	fprintf(stderr, "Failed to create CLIgen.Cvec instance\n"); /* XXX */
	return -1;
    }
    
    /* arg */
    if (arg)
	Arg = (PyObject *)CgVar_InstanceFromCv(arg);
    else {
	Py_INCREF(Py_None);
	Arg = Py_None;
    }
    
    Value = PyObject_CallMethod(self, "_cligen_expand", "sOO", func, Cvec, Arg);
    if (Value == NULL)
	goto done;
    
    num = PyList_Size(Value);
    *commands = calloc(num, sizeof(char *));
    *helptexts = calloc(num, sizeof(char *));
    if (*commands == NULL || *helptexts == NULL) {
        PyErr_SetString(PyExc_MemoryError, "calloc");
        goto done;
    }
    
    if ((iterator = PyObject_GetIter(Value)) == NULL) {
	PyErr_SetString(PyExc_ValueError, "get_iter");
	goto done;
    }

    i = 0;
    while ((item = PyIter_Next(iterator))) {
	if (PyDict_Check(item)) {
	    PyObject *cmd = PyDict_GetItemString(item, "command");
	    PyObject *hlp = PyDict_GetItemString(item, "help");
	    if (cmd && hlp) {
		PyObject* strobj; 
		char *str;

		strobj = PyUnicode_AsUTF8String(cmd);
		str = PyBytes_AsString(strobj);
		(*commands)[i] = strdup(str);
		Py_DECREF(strobj);

		strobj = PyUnicode_AsUTF8String(hlp);
		str = PyBytes_AsString(strobj);
		(*helptexts)[i] = strdup(str);
		Py_DECREF(strobj);

		i++;
	    }
	}
	Py_DECREF(item); 	/* release reference when done */
    }
    Py_DECREF(iterator);
    
    if (PyErr_Occurred()) {
	PyErr_SetString(PyExc_ValueError, "get_iter");
	goto done;
    }

    *nr = i;
    retval = 0;
done:
    Py_XDECREF(Arg);
    Py_XDECREF(Cvec);
    Py_XDECREF(Value);
    
    return retval;
}

expand_cb *
CLIgen_expand_str2fn(char *name, void *arg, char **error)
{
    return CLIgen_expand_cb;
}


static void
CLIgen_dealloc(CLIgen* self)
{
    Py_XDECREF(self->ptlist);
    if (self->handle) {
	if (self->handle->ch_cligen)
	    cligen_exit(self->handle->ch_cligen);
	CLIgen_handle_exit(self->handle);
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
CLIgen_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return (PyObject *)type->tp_alloc(type, 0);
}

static int
CLIgen_init(CLIgen *self, PyObject *args, PyObject *kwds)
{
    if ((self->ptlist = PyList_New(0)) == NULL)
	return -1;
    if ((self->handle = CLIgen_handle_init()) == NULL)
	return -1;
    if ((self->handle->ch_cligen = cligen_init()) == NULL)
	return -1;
    self->handle->ch_self = self;
    cligen_userhandle_set(self->handle->ch_cligen, self->handle);
    
    return 0;
}


#if 0
static PyObject *
CLIgen_parse_str(CLIgen *self, PyObject *args)
{
    char *str;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    return PyLong_FromLong(CLIgen_do_parse(self, str));
}
#endif

static PyObject *
_CLIgen_ptlist(CLIgen *self)
{
    Py_INCREF(self->ptlist);
    return self->ptlist;
}


static PyObject *
CLIgen_prompt(CLIgen *self)
{
    return PyUnicode_FromString(cligen_prompt(self->handle->ch_cligen));
}


static PyObject *
CLIgen_prompt_set(CLIgen *self, PyObject *args)
{
    char *prompt;

    if (!PyArg_ParseTuple(args, "s", &prompt))
        return NULL;

    return PyLong_FromLong(cligen_prompt_set(self->handle->ch_cligen, prompt));
}

static PyObject *
CLIgen_comment(CLIgen *self)
{
    char c[2] = {0, 0};

    c[0] = cligen_comment(self->handle->ch_cligen);
    return PyUnicode_FromString(c);
}

static PyObject *
CLIgen_comment_set(CLIgen *self, PyObject *args)
{
    char *c;

    if (!PyArg_ParseTuple(args, "s", &c))
        return NULL;

    return PyLong_FromLong(cligen_comment_set(self->handle->ch_cligen, *c));
}

static PyObject *
CLIgen_tabmode(CLIgen *self)
{
    return PyLong_FromLong(cligen_tabmode(self->handle->ch_cligen));
}

static PyObject *
CLIgen_tabmode_set(CLIgen *self, PyObject *args)
{
    int mode;

    if (!PyArg_ParseTuple(args, "i", &mode))
        return NULL;

    return PyLong_FromLong(cligen_tabmode_set(self->handle->ch_cligen, mode));
}


static PyObject *
CLIgen_exiting(CLIgen *self)
{
    return PyLong_FromLong(cligen_exiting(self->handle->ch_cligen));
}

static PyObject *
CLIgen_exiting_set(CLIgen *self, PyObject *args)
{
    int e;

    if (!PyArg_ParseTuple(args, "i", &e))
        return NULL;

    return PyLong_FromLong(cligen_exiting_set(self->handle->ch_cligen, e));
}

static PyObject *
CLIgen_tree_add(CLIgen *self, PyObject *args)
{
    char *name;
    PyObject *Pt;
    parse_tree *pt;
    PyObject *Value;

    if (!PyArg_ParseTuple(args, "sO!", &name, &ParseTree_Type, &Pt))
        return NULL;

    pt = ParseTree_pt(Pt);
    if (cligen_tree_add(self->handle->ch_cligen, name, *pt) < 0) {
        PyErr_SetString(PyExc_MemoryError, "cligen_tree_add");
        return NULL;
    }
    
    Value =  PyObject_CallMethod(self->ptlist, "append", "O", Pt);
    Py_XDECREF(Value);
	
    return PyLong_FromLong(0);
}

static PyObject *
CLIgen_tree_active_set(CLIgen *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    if (cligen_tree_active_set(self->handle->ch_cligen, name) < 0) {
        PyErr_SetString(PyExc_MemoryError, "failed to allocate memory");
        return NULL;
    }
    
    return PyLong_FromLong(0);
}

static PyObject *
CLIgen_tree_active(CLIgen *self)
{
    char *name;

    if ((name = cligen_tree_active(self->handle->ch_cligen)) == NULL)
	Py_RETURN_NONE;

    return PyUnicode_FromString(name);
}



static PyObject *
CLIgen_eval(CLIgen *self)
{
    char *line;
    int cb_ret;
    int retval = CG_ERROR;
    
    while (!cligen_exiting(self->handle->ch_cligen)){
	switch (cliread_eval(self->handle->ch_cligen, &line, &cb_ret)){
	case CG_EOF: /* eof */
	    goto done;
	    break;
	case CG_ERROR: /* cligen match errors */
	    printf("CLI read error\n");
	    goto done;
	case CG_NOMATCH: /* no match */
	    printf("CLI syntax error in: \"%s\": %s\n", line, cligen_nomatch(self->handle->ch_cligen))
;
	    break;
	case CG_MATCH: /* unique match */
	    if (cb_ret < 0)
		printf("CLI callback error\n");
	    break;
	default: /* multiple matches */
	    printf("Ambigous command\n");
	    break;
	}
    }
    retval = 0;

 done:
    return PyLong_FromLong(retval);
}


static PyObject *
CLIgen_print(CLIgen *self)
{
    Py_RETURN_NONE;
}


static PyMethodDef CLIgen_methods[] = {
    {"_ptlist", (PyCFunction)_CLIgen_ptlist, METH_NOARGS,
     "Get list of ParseTrees added"
    },

    {"prompt", (PyCFunction)CLIgen_prompt, METH_NOARGS,
     "Get CLIgen prompt"
    },

    {"prompt_set", (PyCFunction)CLIgen_prompt_set, METH_VARARGS,
     "Set CLIgen prompt"
    },

    {"comment", (PyCFunction)CLIgen_comment, METH_NOARGS,
     "Get CLIgen comment character"
    },
    {"comment_set", (PyCFunction)CLIgen_comment_set, METH_VARARGS,
     "Set CLIgen comment character"
    },

    {"tabmode", (PyCFunction)CLIgen_tabmode, METH_NOARGS,
     "Get CLIgen tab-mode. 0 is short/ios mode, 1 is long/junos mode"
    },
    {"tabmode_set", (PyCFunction)CLIgen_tabmode_set, METH_VARARGS,
     "Set CLIgen tab-mode. 0 is short/ios mode, 1 is long/junos mode"
    },

    {"exiting", (PyCFunction)CLIgen_exiting, METH_NOARGS,
     "Check if CLIgen is exiting"
    },

    {"exiting_set", (PyCFunction)CLIgen_exiting_set, METH_VARARGS,
     "Check if CLIgen is exiting"
    },

    {"eval", (PyCFunction)CLIgen_eval, METH_NOARGS,
     "CLIgen command evaluation loop"
    },

    {"print", (PyCFunction)CLIgen_print, METH_NOARGS,
     "Return CLIgen syntax as a string"
    },

    {"tree_add", (PyCFunction)CLIgen_tree_add, METH_VARARGS,
     "Add ParseTree to CLIgen instance"
    },
    {"tree_active", (PyCFunction)CLIgen_tree_active, METH_NOARGS,
     "Get active ParseTree for CLIgen instance" 
    },
    {"tree_active_set", (PyCFunction)CLIgen_tree_active_set, METH_VARARGS,
     "Set active ParseTree for CLIgen instance"
    },

    

   {NULL}  /* Sentinel */
};

PyTypeObject CLIgenType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_cligen.CLIgen",          /* tp_name */
    sizeof(CLIgen),            /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)CLIgen_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "CLIgen object",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    CLIgen_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)CLIgen_init,     /* tp_init */
    0,                         /* tp_alloc */
    CLIgen_new,                /* tp_new */
};

static PyModuleDef cligenmodule = {
    PyModuleDef_HEAD_INIT,
    "_cligen",
    "Python bindings for cligen.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};


PyMODINIT_FUNC
PyInit__cligen(void)
{
    PyObject* m;
    int cgv_init_object(PyObject *m);
    int cvec_init_object(PyObject *m);
    int ParseTree_init_object(PyObject *m);

    m = PyModule_Create(&cligenmodule);
    if (m == NULL)
        return NULL;

    if (PyType_Ready(&CLIgenType) < 0)
        return NULL;

    Py_INCREF(&CLIgenType);
    PyModule_AddObject(m, "CLIgen", (PyObject *)&CLIgenType);


    if (cgv_init_object(m) < 0)
	return NULL;
    if (cvec_init_object(m) < 0)
	return NULL;
    if (ParseTree_init_object(m) < 0)
	return NULL;

    return m;
}
