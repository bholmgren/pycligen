/* 
 * pycligen.h
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

#ifndef __PY_CLIGEN_H__
#define __PY_CLIGEN_H__

/*
 * Support Python 2 and 3
 */
#if PY_MAJOR_VERSION >= 3
  #define MOD_ERROR_VAL NULL
  #define MOD_SUCCESS_VAL(val) val
  #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
          ob = PyModule_Create(&moduledef);
  #define PyInt_FromLong(l)  PyLong_FromLong(l)

#else
  #define MOD_ERROR_VAL
  #define MOD_SUCCESS_VAL(val)
  #define MOD_INIT(name) void init##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          ob = Py_InitModule3(name, methods, doc);

  #define PyExc_FileNotFoundError PyExc_IOError
  #define PyExc_PermissionError PyExc_IOError

  #define PyFloat_FromString(x)	PyFloat_FromString((x), NULL)
  #define PyCapsule_New(p,n,d)  PyCObject_FromVoidPtr((p),(d))
#endif


cligen_handle CLIgen_cligen_handle(PyObject *cgen);

/* Python 3/2 support functions */
PyObject *StringFromString(const char *str);
char *StringAsString(PyObject *obj);
char *ErrString(int restore);

/* CLIgen callbacks */
cg_fnstype_t *CLIgen_str2fn(char *name, void *arg, char **error);
expand_cb *CLIgen_expand_str2fn(char *name, void *arg, char **error);


#endif /* __PY_CLIGEN_H__ */
