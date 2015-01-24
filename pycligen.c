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

#include <cligen/cligen.h>

#include "pycligen.h"
#include "pycligen_cv.h"
#include "pycligen_pt.h"
#include "pycligen_cvec.h"

#define CLIgen_MAGIC  0x8abe91a1

struct _CLIgen;

typedef struct {
    int                      ch_magic;    /* magic (HDR)*/
    struct _CLIgen	    *ch_self;
    cligen_handle            ch_cligen;   /* cligen handle */
} *CLIgen_handle;


typedef struct _CLIgen {
    PyObject_HEAD
    CLIgen_handle handle;
    PyObject *ptlist;	/* List of ParseTrees added */
} CLIgen;



/*
 * Keep an internal reference to the CLIgen module
 */
PyObject *
__cligen_module()
{
    static PyObject *__cligen = NULL;

    if (__cligen == NULL)
	__cligen = PyImport_ImportModule("cligen");

    return __cligen;
}

/*
 * Python 2/3 support function: Convert a char* to python string
 */
PyObject *
StringFromString(const char *str)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(str);
#else
    return PyString_FromString(str);
#endif
}


/*
 * Python 2/3 support function: Convert a python string into a malloc:ed char*
 */
char *
StringAsString(PyObject *obj)
{
    char *str;
    char *retval = NULL;
    PyObject *strobj = NULL;
    

#if  PY_MAJOR_VERSION >= 3
    strobj = PyUnicode_AsUTF8String(obj);
    if (strobj == NULL)
	goto done; 
    str = PyBytes_AsString(strobj);
#else
    str = PyString_AsString(obj);
#endif
    if (str == NULL)
	goto done;

    retval = strdup(str);
done:
#if  PY_MAJOR_VERSION >= 3
    Py_XDECREF(strobj);
#endif
    
    return retval;	
}

/*
 * Python 2/3 support function: Convert a int to python int
 */
PyObject *
IntFromLong(long n)
{
#if PY_MAJOR_VERSION >= 3
    return PyLong_FromLong(n);
#else
    return PyInt_FromLong(n);
#endif
}


/*
 * Python 2/3 support function: Get PyErr message as malloc:ed char*
 */
char *
ErrString(int restore)
{
    char *str = NULL;
    PyObject *type;
    PyObject *value;
    PyObject *traceback;
    
    if (!PyErr_Occurred())
	return NULL;
    
    PyErr_Fetch(&type, &value, &traceback);
    if (value) 
	str = StringAsString(value);
    
    if (restore)
	PyErr_Restore(type, value, traceback);
    else {
	Py_XDECREF(type);
	Py_XDECREF(value);
	Py_XDECREF(traceback);
    }

    return str;
}



static CLIgen_handle
CLIgen_handle_init(void)
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
    PyObject *Ret;
    PyObject *Value = NULL;
    PyObject *Cvec = NULL;
    PyObject *Capsule = NULL;
    int retval = -1;
    PyObject *Arg = NULL;
    
    
    if ((Cvec = PyObject_CallMethod(__cligen_module(), "Cvec",  NULL)) == NULL)
	return -1; 
    if ((Capsule = PyCapsule_New((void *)vars, NULL, NULL)) == NULL) {
	Py_DECREF(Cvec);
	return -1;
    }
    Ret = PyObject_CallMethod(Cvec, "__Cvec_from_cvec", "O", Capsule);
    Py_DECREF(Capsule);
    if (Ret == NULL) {
	Py_DECREF(Cvec);
	return -1;
    }
    Py_DECREF(Ret);

    /* arg */
    if (arg)
	Arg = (PyObject *)CgVar_Instance(arg);
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
CLIgen_expand_cb(cligen_handle *h, char *func, cvec *vars, cg_var *arg, 
	      int  *nr,
	      char ***commands,     /* vector of function strings */
	      char ***helptexts)   /* vector of help-texts */
{
    int i;
    int num;
    int retval = -1;
    CLIgen_handle ch = (CLIgen_handle)h;
    PyObject *self = (PyObject *)ch->ch_self;
    PyObject *Ret;
    PyObject *Value = NULL;
    PyObject *Cvec = NULL;
    PyObject *Capsule;
    PyObject *Arg = NULL;
    PyObject *iterator = NULL;
    PyObject *item = NULL;
    PyObject *cmd;
    PyObject *hlp;

    *nr = 0;

    /* Get a Cvec instance */ 
    if ((Cvec = PyObject_CallMethod(__cligen_module(), "Cvec",  NULL)) == NULL)
	return -1;
    if ((Capsule = PyCapsule_New((void *)vars, NULL, NULL)) == NULL) {
	Py_DECREF(Cvec);
	return -1;
    }
    Ret = PyObject_CallMethod(Cvec, "__Cvec_from_cvec", "O", Capsule);
    Py_DECREF(Capsule);
    if (Ret == NULL) {
	Py_DECREF(Cvec);
	return -1;
    }
    Py_DECREF(Ret);

    /* arg */
    if (arg)
	Arg = (PyObject *)CgVar_Instance(arg);
    else {
	Py_INCREF(Py_None);
	Arg = Py_None;
    }
    
    Value = PyObject_CallMethod(self, "_cligen_expand", "sOO", func, Cvec, Arg);
    if (Value == NULL)
	goto done;
    if (!PyList_Check(Value))
	goto done;

    num = PyList_Size(Value);
    *commands = calloc(num, sizeof(char *));
    *helptexts = calloc(num, sizeof(char *));
    if (*commands == NULL || *helptexts == NULL) {
	PyErr_NoMemory();
	goto done;
    }
    
    if ((iterator = PyObject_GetIter(Value)) == NULL) {
	PyErr_SetString(PyExc_ValueError, "get_iter");
	goto done;
    }
    
    i = 0;
    while ((item = PyIter_Next(iterator))) {
	if (!PyDict_Check(item))
	    goto done;
	if ((cmd = PyDict_GetItemString(item, "command")) == NULL)
	    goto done;
 	if (((*commands)[i] = StringAsString(cmd)) == NULL)
	    goto done;
	if ((hlp = PyDict_GetItemString(item, "help")) == NULL)
	    goto done;
	if (((*helptexts)[i] = StringAsString(hlp)) == NULL)
	    goto done;
	i++;
	Py_DECREF(item); 	/* iterator item ref must be released */
	item = NULL;
    }
    Py_DECREF(iterator);
    iterator = NULL;

  
    *nr = i;
    retval = 0;
done:
	if (PyErr_Occurred())
	    PyErr_Print();
    if (retval != 0) {
	for(i = 0; i < num; i++) {
	    if ((*commands) && (*commands)[i])
		free((*commands)[i]);
	    if ((*helptexts) && (*helptexts)[i])
		free((*helptexts)[i]);
	}
	free(*commands);
	free(*helptexts);
	*commands = *helptexts = NULL;
	*nr = 0;
    }
    Py_XDECREF(Arg);
    Py_XDECREF(Cvec);
    Py_XDECREF(Value);
    Py_XDECREF(item);
    Py_XDECREF(iterator);

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


static PyObject *
_CLIgen_ptlist(CLIgen *self)
{
    Py_INCREF(self->ptlist);
    return self->ptlist;
}


static PyObject *
CLIgen_prompt(CLIgen *self)
{
    return StringFromString(cligen_prompt(self->handle->ch_cligen));
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

    return StringFromString(c);
}

static PyObject *
CLIgen_comment_set(CLIgen *self, PyObject *args)
{
    char *c;
    char retval[2] = {0,0};

    if (!PyArg_ParseTuple(args, "s", &c))
        return NULL;

    retval[0] = cligen_comment_set(self->handle->ch_cligen, *c);
    return StringFromString(retval);
}

static PyObject *
CLIgen_completion(CLIgen *self)
{
    return PyLong_FromLong(cligen_completion(self->handle->ch_cligen));
}

static PyObject *
CLIgen_completion_set(CLIgen *self, PyObject *args)
{
    int mode;

    if (!PyArg_ParseTuple(args, "i", &mode))
        return NULL;

    cligen_completion_set(self->handle->ch_cligen, (mode == 0 ? 0 : 1));
    return CLIgen_completion(self);
}

static PyObject *
CLIgen_terminalrows(CLIgen *self)
{
    return PyLong_FromLong(cligen_terminalrows(self->handle->ch_cligen));
}

static PyObject *
CLIgen_terminalrows_set(CLIgen *self, PyObject *args)
{
    int rows;

    if (!PyArg_ParseTuple(args, "i", &rows))
        return NULL;

    cligen_terminalrows_set(self->handle->ch_cligen, rows);
    return CLIgen_terminalrows(self);
}

static PyObject *
CLIgen_terminal_length(CLIgen *self)
{
    return PyLong_FromLong(cligen_terminal_length(self->handle->ch_cligen));
}

static PyObject *
CLIgen_terminal_length_set(CLIgen *self, PyObject *args)
{
    int len;

    if (!PyArg_ParseTuple(args, "i", &len))
        return NULL;

    cligen_terminal_length_set(self->handle->ch_cligen, len);
    return CLIgen_terminal_length(self);
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

    return PyLong_FromLong(cligen_tabmode_set(self->handle->ch_cligen, 
					      (mode == 0 ? 0 : 1)));

}

static PyObject *
CLIgen_lexicalorder(CLIgen *self)
{
    return PyLong_FromLong(cligen_lexicalorder(self->handle->ch_cligen));
}

static PyObject *
CLIgen_lexicalorder_set(CLIgen *self, PyObject *args)
{
    int mode;

    if (!PyArg_ParseTuple(args, "i", &mode))
        return NULL;

    return PyLong_FromLong(cligen_lexicalorder_set(self->handle->ch_cligen,
						   (mode == 0 ? 0 : 1)));
}

static PyObject *
CLIgen_ignorecase(CLIgen *self)
{
    if (cligen_ignorecase(self->handle->ch_cligen) == 0)
	Py_RETURN_FALSE;
    else
	Py_RETURN_TRUE;
}

static PyObject *
CLIgen_ignorecase_set(CLIgen *self, PyObject *args)
{
    PyObject *bool;

    if (!PyArg_ParseTuple(args, "O!", &PyBool_Type, &bool))
        return NULL;

    cligen_ignorecase_set(self->handle->ch_cligen, (bool == Py_True) ? 1 : 0);
    return CLIgen_ignorecase(self);
}

static PyObject *
_CLIgen_output(CLIgen *self, PyObject *args)
{
    int fd;
    FILE *f;
    char *output;
    PyObject *file;
    PyObject *fileno;
    
    if (!PyArg_ParseTuple(args, "Os", &file, &output))
        return NULL;
    
    if ((fileno = PyObject_CallMethod(file, "fileno", NULL)) == NULL)
	return NULL;
    fd = PyLong_AsLong(fileno);
    Py_DECREF(fileno);

    if ((f = fdopen(fd, "a")) == NULL) {	
	PyErr_Format(PyExc_IOError, "%s", strerror(errno));
	return NULL;
    }
    
    cligen_output(f, output);

    Py_RETURN_NONE;
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
CLIgen_tree(CLIgen *self, PyObject *args)
{
    char *name;
    PyObject *Pt = NULL;
    PyObject *iterator;
    PyObject *item;
    char *n;
    

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;


    if ((iterator = PyObject_GetIter(self->ptlist)) == NULL) {
	PyErr_SetString(PyExc_ValueError, "get_iter");
	return NULL;
    }
    while (Pt == NULL && (item = PyIter_Next(iterator))) {
	n = ParseTree_name(item);
	if (n && strcmp(name, n) == 0)
	    Pt = item;
	Py_DECREF(item); 	/* iterator item ref must be released */
    }
    Py_DECREF(iterator);

    if (Pt) {
	Py_INCREF(Pt);
	return Pt;
    }
    
    Py_RETURN_NONE;
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

    if (ParseTree_name_set(Pt, name) < 0)
	return PyErr_NoMemory();
    
    pt = ParseTree_pt(Pt);
    if (cligen_tree_add(self->handle->ch_cligen, name, *pt) < 0)
	return PyErr_NoMemory();
    
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

    if (cligen_tree_active_set(self->handle->ch_cligen, name) < 0)
	return PyErr_NoMemory();
    
    return PyLong_FromLong(0);
}

static PyObject *
CLIgen_tree_active(CLIgen *self)
{
    char *name;

    if ((name = cligen_tree_active(self->handle->ch_cligen)) == NULL)
	Py_RETURN_NONE;

    return StringFromString(name);
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

    {"completion", (PyCFunction)CLIgen_completion, METH_NOARGS,
     "Get CLIgen completion mode"
    },
    {"completion_set", (PyCFunction)CLIgen_completion_set, METH_VARARGS,
     "Set CLIgen completion mode"
    },

    {"terminalrows", (PyCFunction)CLIgen_terminalrows, METH_NOARGS,
     "Get number of displayed terminal rows"
    },
    {"terminalrows_set", (PyCFunction)CLIgen_terminalrows_set, METH_VARARGS,
     "Set number of displayed terminal rows"
    },

    {"terminal_length", (PyCFunction)CLIgen_terminal_length, METH_NOARGS,
     "Get length of lines (number of columns on a line)"
    },
    {"terminal_length_set", (PyCFunction)CLIgen_terminal_length_set, METH_VARARGS,
     "Set length of lines (number of columns on a line)"
    },

    {"tabmode", (PyCFunction)CLIgen_tabmode, METH_NOARGS,
     "Get CLIgen tab-mode. 0 is short/ios mode, 1 is long/junos mode"
    },
    {"tabmode_set", (PyCFunction)CLIgen_tabmode_set, METH_VARARGS,
     "Set CLIgen tab-mode. 0 is short/ios mode, 1 is long/junos mode"
    },

    {"lexicalorder", (PyCFunction)CLIgen_lexicalorder, METH_NOARGS,
     "Get lexical matching order> strcmp(0) or strvercmp(1)"
    },
    {"lexicalorder_set", (PyCFunction)CLIgen_lexicalorder_set, METH_VARARGS,
     "Set lexical matching order> strcmp(0) or strvercmp(1)"
    },

    {"ignorecase", (PyCFunction)CLIgen_ignorecase, METH_NOARGS,
     "Get status of CLIgen ignoring uppercase/lowercase"
    },
    {"ignorecase_set", (PyCFunction)CLIgen_ignorecase_set, METH_VARARGS,
     "Set status of CLIgen ignoring uppercase/lowercase"
    },

    {"_output", (PyCFunction)_CLIgen_output, METH_VARARGS,
     "CLIgen terminal output function. All outout should be made via this method"
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

    {"tree", (PyCFunction)CLIgen_tree, METH_VARARGS,
     "Get ParseTree by name"
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


/*
 * CLIgen class object
 */
PyTypeObject CLIgen_Type = {
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

/*
 * Module methods
 */
static PyMethodDef cligen_methods[] = {

    {NULL}  /* Sentinel */
};


MOD_INIT(_cligen)
{
    PyObject* m;

    MOD_DEF(m, "_cligen", "Python bindings for CLIgen", cligen_methods);
    if (m == NULL)
        return MOD_ERROR_VAL;

    if (PyType_Ready(&CLIgen_Type) < 0)
        return MOD_ERROR_VAL;

    Py_INCREF(&CLIgen_Type);
    PyModule_AddObject(m, "CLIgen", (PyObject *)&CLIgen_Type);


    if (CgVar_init_object(m) < 0)
        return MOD_ERROR_VAL;
    if (Cvec_init_object(m) < 0)
        return MOD_ERROR_VAL;
    if (ParseTree_init_object(m) < 0)
        return MOD_ERROR_VAL;

    return MOD_SUCCESS_VAL(m);
}


cligen_handle
CLIgen_cligen_handle(PyObject *obj)
{
    if ( ! PyObject_TypeCheck(obj, &CLIgen_Type)) {
	PyErr_SetString(PyExc_ValueError, "Object not CLIgen type");
	return NULL;
    }

    return ((CLIgen *)obj)->handle->ch_cligen;
}

