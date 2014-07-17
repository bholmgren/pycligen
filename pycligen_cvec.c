/* 
 * pycligen_cv.c
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

#include "pycligen_cv.h"


#if 0
typedef struct {
    PyObject_HEAD
    PyObject *list;
} Cvec;

static void
Cvec_dealloc(Cvec* self)
{
//    puts("CV DEALLOC");
    Py_XDECREF(self->list);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
Cvec_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Cvec *self;
    
    if ((self = (Cvec *)type->tp_alloc(type, 0)) == NULL)
	return NULL;
    
    if ((self->list = PyList_New(0)) == NULL) {
	Py_DECREF(self);
	return NULL;
    }
    
    return (PyObject *)self;
}

static int
Cvec_init(Cvec *self, PyObject *args, PyObject *kwds)
{
    PyObject *cv;
    
    if (! PyArg_ParseTuple(args, "O", &cv))
        return -1;
    
    
    return 0;
}

static PyMethodDef Cvec_methods[] = {
    
    {NULL}  /* Sentinel */
};

static PyTypeObject CvecType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_cligen.Cvec",            /* tp_name */
    sizeof(Cvec),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)Cvec_dealloc, /* tp_dealloc */
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
    "Cvec objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Cvec_methods,             /* tp_methods */
    0,                        /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Cvec_init,      /* tp_init */
    0,                         /* tp_alloc */
    Cvec_new,                 /* tp_new */
};
#endif

int
cvec_init_object(PyObject *m)
{
#if 0
    if (PyType_Ready(&CvecType) < 0)
        return -1;

    Py_INCREF(&CvecType);
    PyModule_AddObject(m, "Cvec", (PyObject *)&CvecType);

#endif
    return 0;
}


/*
 * Create CLIgen.Cvec instance from cvec
 */
PyObject *
Cvec_from_cvec(PyObject *self, cvec *vr)
{
    PyObject *Name = NULL;
    PyObject *Cls = NULL;
    PyObject *Cvec = NULL;
    CgVar *Cv = NULL;
    cg_var *cv;
 
    /* Find class object */
#if 0
    {
	PyObject *pName, *pModule, *pDict, *pFunc;
	pName = PyUnicode_FromString("cligen.Cvec");
	assert(pName);
	pModule = PyImport_Import(pName);
	assert(pModule);
	pDict = PyModule_GetDict(pModule);
	assert(pDict);c
			  pFunc = PyDict_GetItemString(pDict, func);
	assert(pFunc);
	value =  PyObject_CallFunction(pFunc, "s", func);
    }
    Cls = PyObject_GetAttrString((PyObject *)self, Cls_str);
    PyErr_Clear(); /* Ignore err */
    if (Cls == NULL) {
        PyErr_SetString(PyExc_AttributeError, "class not found");
	return NULL;
    }
#error "apa"
    { 
	/* get sys.modules dict */
	PyObject *sysmdict, *mod;
	sysmdict = PyImport_GetModuleDict();
	mod = PyMapping_GetItemString(sys_mod_dict, "cligen");
    /* call the class inside the __main__ module */
    PyObject* instance = PyObject_CallMethod(main_mod, "MyClass", "");
    }
#error "apa"
#error "apa"

    if ( ! PyCallable_Check(Cls)) {
        PyErr_SetString(PyExc_TypeError, NULL);
	goto quit;
    }

    /* Get a Cvec instance */
    if ((Cvec = PyObject_CallObject((PyObject *) &CgVarType, NULL)) == NULL) {
/*	fprintf(stderr, "Failed to create CLIgen.%s instance\n", Cls_str);*/
	goto quit;
    }
#endif
    
    if ((Name = PyUnicode_FromString("cligen")) == NULL)
	goto quit;
    if ((Cls = PyImport_Import(Name)) == NULL)
	goto quit;
    if ((Cvec = PyObject_CallMethod(Cls, "Cvec", NULL)) == NULL)
	goto quit;
    
    /* Add CgVars */
    for (cv = NULL; (cv = cvec_each(vr, cv)); ) {
	Cv = (CgVar *)PyObject_CallMethod(Cvec, "add", "i", cv_type_get(cv));
	if (Cv == NULL)
	    goto quit;
	Py_DECREF(Cv);
	if ((cv_cp(Cv->cv, cv)) < 0) {
	    PyErr_SetString(PyExc_MemoryError, NULL);
	    goto quit;
	}
    }
    
    Py_DECREF(Name);
    Py_DECREF(Cls);

    return Cvec;
    
quit:
    Py_XDECREF(Name);
    Py_XDECREF(Cls);
    Py_XDECREF(Cvec);

    return NULL;
}
