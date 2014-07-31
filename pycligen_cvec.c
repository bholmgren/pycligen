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

#include <cligen/cligen.h>

#include "pycligen.h"
#include "pycligen_cv.h"


/*
 * Create CLIgen.Cvec instance from cvec
 */
PyObject *
Cvec_from_cvec(PyObject *self, cvec *vr)
{
    PyObject *Name = NULL;
    PyObject *Cls = NULL;
    PyObject *Cvec = NULL;
    PyObject *Cv = NULL;
    PyObject *Retval = NULL;
    cg_var *new = NULL;
    cg_var *cv;
 
    /* Find class object */
    
    Name = StringFromString("cligen");
    if (Name == NULL)
	return NULL;
    Cls = PyImport_Import(Name);
    Py_DECREF(Name);
    if (Cls == NULL)
	return NULL;
    Cvec = PyObject_CallMethod(Cls, "Cvec", NULL);
    Py_DECREF(Cls);
    if (Cvec == NULL)
	return NULL;
    
    /* Add CgVars */
    for (cv = NULL; (cv = cvec_each(vr, cv)); ) {
	Cv = PyObject_CallMethod(Cvec, "add", "i", cv_type_get(cv));
	if (Cv == NULL)
	    goto quit;
	Py_DECREF(Cv);  /* leave only reference owned by Cvec object */
	if ((new = CgVar_cv(Cv)) == NULL)
	    goto quit;
	if ((cv_cp(new, cv)) < 0) {
	    PyErr_SetString(PyExc_MemoryError, NULL);
	    goto quit;
	}
    }
    
    Retval = Cvec;
quit:
    if (Retval == NULL)
	Py_XDECREF(Cvec);

    return Retval;
}
