/* 
 * pycligen_cv.c
 *
 * Copyright (C) 2014-2015 Benny Holmgren
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

typedef struct {
    PyObject_HEAD
} Cvec;


static void
Cvec_dealloc(Cvec* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Cvec_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return (PyObject *)type->tp_alloc(type, 0);
}

static int
Cvec_init(Cvec *self, PyObject *args, PyObject *kwds)
{
    return 0;
}


/*
 * Create CLIgen.Cvec instance from cvec
 */
PyObject *
__Cvec_from_cvec(PyObject *self, PyObject *args)
{
    PyObject *Capsule = NULL;
    PyObject *Cv = NULL;
    PyObject *Method = NULL;
    PyObject *Ret;
    cvec *vr = NULL;
    cg_var *cv;

    if (!PyArg_ParseTuple(args, "O", &Capsule))
        return NULL;

    if ((vr = (cvec *)PyCapsule_GetPointer(Capsule, NULL)) == NULL)
	return NULL;

    /* Add CgVars */
    for (cv = NULL; (cv = cvec_each(vr, cv)); ) {	
	if ((Cv = CgVar_Instance(cv)) == NULL)
	    return NULL;
	
	if ((Method = StringFromString("append")) == NULL)
	    return NULL;
	
	Ret = PyObject_CallMethodObjArgs(self, Method, Cv, NULL);
	Py_DECREF(Method);
	Py_DECREF(Cv);  /* leave only reference owned by Cvec object */
	if (Ret == NULL)
	    return NULL;
	Py_DECREF(Ret);
    }

    Py_RETURN_NONE;
}



static PyMethodDef Cvec_methods[] = {
    
    {"__Cvec_from_cvec", (PyCFunction)__Cvec_from_cvec, METH_VARARGS, 
     "Populate self from cvec* pointer"
    },


   {NULL}  /* Sentinel */
};

PyTypeObject Cvec_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_cligen.Cvec",            /* tp_name */
    sizeof(Cvec),              /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Cvec_dealloc,  /* tp_dealloc */
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
    "Cvec objects",            /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Cvec_methods,              /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Cvec_init,       /* tp_init */
    0,                         /* tp_alloc */
    Cvec_new,                  /* tp_new */
};

int
Cvec_init_object(PyObject *m)
{

    if (PyType_Ready(&Cvec_Type) < 0)
        return -1;

    Py_INCREF(&Cvec_Type);
    PyModule_AddObject(m, "Cvec", (PyObject *)&Cvec_Type);

    return 0;
}
