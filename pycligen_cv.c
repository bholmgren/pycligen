/* 
 * py_cligen_cv.c 
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cligen/cligen.h>

#include "pycligen.h"


typedef struct {
    PyObject_HEAD
    cg_var *cv;
} CgVar;


static void
CgVar_dealloc(CgVar* self)
{
//    puts("CgVar_dealloc");
    cv_free(self->cv);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
CgVar_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CgVar *self;

//    puts("CgVar_new");
    self = (CgVar *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->cv = cv_new(CGV_ERR);
        if (self->cv == NULL) {
            Py_DECREF(self);
            return NULL;
        }
    }
    return (PyObject *)self;
}

static int
CgVar_init(CgVar *self, PyObject *args, PyObject *kwds)
{
    char *str;
    char *name = NULL;
    int type = CGV_ERR;
    PyObject *Str;
    PyObject *value = NULL;

    static char *kwlist[] = {"type", "name", "value", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|isO", kwlist,
                                      &type, &name, &value))
        return -1;

    /* Set type */
    cv_type_set(self->cv, type);

    /* Set name if given */
    if (name && cv_name_set(self->cv, name) == NULL) {
	PyErr_SetNone(PyExc_MemoryError);
	return -1;
    }

    /* Parse value if specified */
    if (value != NULL && (Str = PyObject_Str(value))) {
	str = StringAsString(Str);
	Py_DECREF(Str);
	if (str == NULL)
	    return -1;
	if (type == CGV_BOOL)  /* CLIgen wants lowercase */
	    if (strlen(str) > 0)
		*str = tolower(*str);
	if (cv_parse(str, self->cv) < 0) {
	    free(str);
	    PyErr_Format(PyExc_ValueError,
			 "invalid format for type '%s'", cv_type2str(type));
	    return -1;
	}
	free(str);
    }
    
    return 0;
}

static PyObject * 
CgVar_repr(PyObject *self)
{
    char *str;
    PyObject *ob;

    str = cv2str_dup(((CgVar *)self)->cv);
    if (str == NULL) {
        PyErr_SetString(PyExc_MemoryError, "cv2str_dup");
        return NULL;
    }
    
    ob = StringFromString(str);
    free(str);
    
    return ob;
}

static int
CgVar_type_verify(CgVar *self, enum cv_type type)
{
    if (self->cv == NULL) {
        PyErr_SetString(PyExc_AttributeError, "cv");
        return -1;
    }
    if (cv_type_get(self->cv) != type) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return -1;
    }
    
    return 0;
}


static PyObject *
CgVar_name_get(CgVar *self)
{
    char *str;

    assert(self->cv);

    str = cv_name_get(self->cv);
    if (str == NULL)
	Py_RETURN_NONE;
    
    return StringFromString(str);
}

static PyObject *
CgVar_name_set(CgVar *self, PyObject *args)
{
    char *str;

    assert(self->cv);

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (cv_name_set(self->cv, str) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "string_set");
        return NULL;
    }
    
    return StringFromString(str);
}

static PyObject *
CgVar_type_get(CgVar *self)
{
    assert(self->cv);

    return PyLong_FromLong(cv_type_get(self->cv));
}

static PyObject *
CgVar_type_set(CgVar *self, PyObject *args)
{
    enum cv_type type;
    cg_var *cv;

    assert(self->cv);

    if (!PyArg_ParseTuple(args, "i", &type))
        return NULL;

    if (type != cv_type_get(self->cv)) {
	
	/* Clean-up by creating new cg_var and free old */
	if ((cv = cv_new(type)) == NULL || 
	         cv_name_set(cv, cv_name_get(self->cv)) == NULL) {
	    PyErr_SetString(PyExc_MemoryError, "cv_new");
	    return NULL;
	}
	cv_free(self->cv);
	self->cv = cv;
    }
    
    return PyLong_FromLong(type);
}

static PyObject *
CgVar_type2str(CgVar *self, PyObject *args)
{
    enum cv_type type = CGV_ERR;

    if (!PyArg_ParseTuple(args, "|i", &type)) {
	if (self->cv == NULL) {
	    PyErr_SetString(PyExc_AttributeError, "cv");
	    return NULL;
	}
    } 
    if (type == CGV_ERR)
	type = cv_type_get(self->cv);    

    return StringFromString(cv_type2str(type));
}

static PyObject *
CgVar_const_get(CgVar *self)
{
    if (cv_const_get(self->cv))
	Py_RETURN_TRUE;
    else
	Py_RETURN_FALSE;
}

static PyObject *
CgVar_const_set(CgVar *self, PyObject *args)
{
    PyObject *bool;

    if (!PyArg_ParseTuple(args, "O!", &PyBool_Type, &bool))
        return NULL;

    if (cv_const_set(self->cv, (bool == Py_True) ? 1 : 0))
	Py_RETURN_TRUE;
    else
	Py_RETURN_FALSE;
}

static PyObject *
CgVar_flag(CgVar *self, PyObject *args)
{
    unsigned char mask;

    if (!PyArg_ParseTuple(args, "b", &mask))
        return NULL;

    return PyLong_FromLong(cv_flag(self->cv, mask));
}

static PyObject *
CgVar_flag_clr(CgVar *self, PyObject *args)
{
    unsigned char mask;

    if (!PyArg_ParseTuple(args, "b", &mask))
        return NULL;

    return PyLong_FromLong(cv_flag_clr(self->cv, mask));
}

static PyObject *
CgVar_flag_set(CgVar *self, PyObject *args)
{
    unsigned char mask;

    if (!PyArg_ParseTuple(args, "b", &mask))
        return NULL;

    return PyLong_FromLong(cv_flag_set(self->cv, mask));
}

static PyObject *
CgVar_int_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_INT))
        return NULL;
	
    return PyLong_FromLong(cv_int_get(self->cv));
}

static PyObject *
CgVar_int_set(CgVar *self, PyObject *args)
{
    int32_t num;

    if (CgVar_type_verify(self, CGV_INT))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "i", &num))
        return NULL;

    cv_int_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
CgVar_long_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_LONG))
        return NULL;
	
    return PyLong_FromLong(cv_long_get(self->cv));
}

static PyObject *
CgVar_long_set(CgVar *self, PyObject *args)
{
    int64_t num;

    if (CgVar_type_verify(self, CGV_LONG))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "l", &num))
        return NULL;

    cv_long_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
CgVar_bool_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_BOOL))
        return NULL;
	
    if (cv_bool_get(self->cv) == 0)
	Py_RETURN_FALSE;
    else
	Py_RETURN_TRUE;
}

static PyObject *
CgVar_bool_set(CgVar *self, PyObject *args)
{
    PyObject *bool;

    if (CgVar_type_verify(self, CGV_BOOL))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "O!", &PyBool_Type, &bool))
        return NULL;

    cv_bool_set(self->cv, (bool == Py_True) ? 1 : 0);
    return CgVar_bool_get(self);
}

static PyObject *
CgVar_string_get(CgVar *self)
{
    char *str;

    if (CgVar_type_verify(self, CGV_STRING))
        return NULL;
	
    str = cv_string_get(self->cv);
    if (str == NULL)
	Py_RETURN_NONE;
    
    return StringFromString(str);
}

static PyObject *
CgVar_string_set(CgVar *self, PyObject *args)
{
    char *str;

    if (CgVar_type_verify(self, CGV_STRING))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if (cv_string_set(self->cv, str) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "string_set");
        return NULL;
    }
    
    return StringFromString(str);
}

static PyObject *
_CgVar_ipv4addr_get(CgVar *self)
{
   struct in_addr *addr;
    char addrp[INET_ADDRSTRLEN];

    if (cv_type_get(self->cv) != CGV_IPV4ADDR && 
	cv_type_get(self->cv) != CGV_IPV4PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
	
    if ((addr = cv_ipv4addr_get(self->cv)) == NULL)
	Py_RETURN_NONE;
    
    return StringFromString(inet_ntop(AF_INET, addr, addrp, INET_ADDRSTRLEN));
}

static PyObject *
_CgVar_ipv4masklen_get(CgVar *self)
{
    if (cv_type_get(self->cv) != CGV_IPV4ADDR && 
	cv_type_get(self->cv) != CGV_IPV4PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
	
    return PyLong_FromLong(cv_ipv4masklen_get(self->cv));
}

#if notimplemented
static PyObject *
_CgVar_ipv4addr_set(CgVar *self, PyObject *args)
{
    PyObject * addr;

    if (cv_type_get(self->cv) != CGV_IPV4ADDR && 
	cv_type_get(self->cv) != CGV_IPV4PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
	
    if (!PyArg_ParseTuple(args, "O", &addr))
        return NULL;
    
    PyErr_SetNone(PyExc_NotImplementedError);
    return NULL;
}
#endif /* notimplemented */

static PyObject *
_CgVar_ipv6addr_get(CgVar *self)
{
    struct in6_addr *addr;
    char addrp[INET6_ADDRSTRLEN];

    if (cv_type_get(self->cv) != CGV_IPV6ADDR && 
	cv_type_get(self->cv) != CGV_IPV6PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
	
    if ((addr = cv_ipv6addr_get(self->cv)) == NULL)
	Py_RETURN_NONE;
    
    return StringFromString(inet_ntop(AF_INET6, addr, addrp, INET6_ADDRSTRLEN));
}

static PyObject *
_CgVar_ipv6masklen_get(CgVar *self)
{
    if (cv_type_get(self->cv) != CGV_IPV6ADDR && 
	cv_type_get(self->cv) != CGV_IPV6PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
	
    return PyLong_FromLong(cv_ipv6masklen_get(self->cv));
}

#if notimplemented
static PyObject *
_CgVar_ipv6addr_set(CgVar *self, PyObject *args)
{
    PyObject * addr;

    if (cv_type_get(self->cv) != CGV_IPV6ADDR && 
	cv_type_get(self->cv) != CGV_IPV6PFX) {
        PyErr_SetString(PyExc_TypeError, "invalid type");
        return NULL;
    }
    
    PyErr_SetNone(PyExc_NotImplementedError);
    return NULL;
}
#endif /* notimplemented */

#define MACADDR_STRLEN 12 /* 6 * "xx" */
static PyObject *
CgVar_mac_get(CgVar *self)
{
    char *m;
    char mac[MACADDR_STRLEN + 1];
    unsigned long l;

    if (CgVar_type_verify(self, CGV_MACADDR))
        return NULL;
	
    m = cv_mac_get(self->cv);
    snprintf(mac, sizeof(mac),
	     "%02x%02x%02x%02x%02x%02x", 
	     (unsigned char)m[0],
	     (unsigned char)m[1],
	     (unsigned char)m[2],
	     (unsigned char)m[3],
	     (unsigned char)m[4],
	     (unsigned char)m[5]);
    
    l = strtoul(mac, NULL, 16);

    return PyLong_FromLong(l);
}

static PyObject *
CgVar_uuid_get(CgVar *self)
{
    return StringFromString("not implemented");
}

static PyObject *
CgVar_parse(CgVar *self, PyObject *args)
{
    char *str;
    cg_var *cv;

    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    if ((cv = cv_new(cv_type_get(self->cv))) == NULL) {
        PyErr_SetString(PyExc_MemoryError, "cv_new");
	return NULL;
    }
     
    if (cv_name_set(cv, cv_name_get(self->cv)) < 0) {
        PyErr_SetString(PyExc_MemoryError, "cv_name_set");
	cv_free(cv);
	return NULL;
    }
    
   if (cv_parse(str, cv) < 0) {
	PyErr_SetString(PyExc_ValueError, "cv_parse");
	cv_free(cv);
	return NULL;
    }

    cv_free(self->cv);
    self->cv = cv;

    Py_RETURN_TRUE;
}

static PyMethodDef CgVar_methods[] = {
    {"name_get", (PyCFunction)CgVar_name_get, METH_NOARGS,
    "Return the name of the variable"
    },
    {"name_set", (PyCFunction)CgVar_name_set, METH_VARARGS,
    "Set the name of the variable"
    },

    {"type_get", (PyCFunction)CgVar_type_get, METH_NOARGS, 
     "Return the type of the variable"
    },
    {"type_set", (PyCFunction)CgVar_type_set, METH_VARARGS, 
     "Set the type of the variable"
    },

    {"const_get", (PyCFunction)CgVar_const_get, METH_NOARGS, 
     "Return the const value of the variable"
    },
    {"const_set", (PyCFunction)CgVar_const_set, METH_VARARGS, 
     "Set the const value of the variable"
    },
 
    {"flag", (PyCFunction)CgVar_flag, METH_VARARGS, 
     "Return the flag of the variable"
    },
    {"flag_set", (PyCFunction)CgVar_flag_set, METH_VARARGS, 
     "Set the flag of the variable"
    },
    {"flag_clr", (PyCFunction)CgVar_flag_clr, METH_VARARGS, 
     "Set the flag of the variable"
    },

#if 0
#endif
    {"int_get", (PyCFunction)CgVar_int_get, METH_NOARGS, 
     "Return the int value of the variable"
    },
    {"int_set", (PyCFunction)CgVar_int_set, METH_VARARGS, 
     "Set the int value of the variable"
    },

    {"long_get", (PyCFunction)CgVar_long_get, METH_NOARGS, 
     "Return the long value of the variable"
    },
    {"long_set", (PyCFunction)CgVar_long_set, METH_VARARGS, 
     "Set the bool value of the variable"
    },

    {"bool_get", (PyCFunction)CgVar_bool_get, METH_NOARGS, 
     "Return the boolean value of the variable"
    },
    {"bool_set", (PyCFunction)CgVar_bool_set, METH_VARARGS, 
     "Set the boolean value of the variable"
    },

    {"string_get", (PyCFunction)CgVar_string_get, METH_NOARGS, 
     "Return the string value of the variable"
    },
    {"string_set", (PyCFunction)CgVar_string_set, METH_VARARGS, 
     "Set the string value of variable"
    },

    {"_ipv4addr_get", (PyCFunction)_CgVar_ipv4addr_get, METH_NOARGS, 
     "Return the IPv4 address value of the variable"
    },

    {"_ipv4masklen_get", (PyCFunction)_CgVar_ipv4masklen_get, METH_NOARGS, 
     "Return the IPv4 prefix masklen of the variable"
    },
#if notimplemented
    {"_ipv4addr_set", (PyCFunction)_CgVar_ipv4addr_set, METH_VARARGS, 
     "Set the IPv4 address value of the variable"
    },
#endif

    {"_ipv6addr_get", (PyCFunction)_CgVar_ipv6addr_get, METH_NOARGS, 
     "Return the Ipv6 address value of the variable"
    },

    {"_ipv6masklen_get", (PyCFunction)_CgVar_ipv6masklen_get, METH_NOARGS, 
     "Return the Ipv6 prefix masklen of the variable"
    },
#if notimplemented
    {"_ipv6addr_set", (PyCFunction)_CgVar_ipv6addr_set, METH_VARARGS, 
     "Set the Ipv6 address value of the variable"
    },
#endif

    {"mac_get", (PyCFunction)CgVar_mac_get, METH_NOARGS, 
     "Get the MAC address value of the variable"
    },

    {"uuid_set", (PyCFunction)CgVar_uuid_get, METH_NOARGS, 
     "Get the UUID value of the variable"
    },

    {"parse", (PyCFunction)CgVar_parse, METH_VARARGS, 
     "Parse a string representation of a value into a CgVar"
    },


    {"type2str", (PyCFunction)CgVar_type2str, METH_VARARGS, 
     "Get the string representation of variable value"
    },

   {NULL}  /* Sentinel */
};

PyTypeObject CgVar_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_cligen.CgVar",            /* tp_name */
    sizeof(CgVar),             /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)CgVar_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    CgVar_repr,                /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    CgVar_repr,                /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "CgVar objects",           /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    CgVar_methods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)CgVar_init,      /* tp_init */
    0,                         /* tp_alloc */
    CgVar_new,                 /* tp_new */
};

int
CgVar_init_object(PyObject *m)
{

    if (PyType_Ready(&CgVar_Type) < 0)
        return -1;

    Py_INCREF(&CgVar_Type);
    PyModule_AddObject(m, "CgVar", (PyObject *)&CgVar_Type);

    PyModule_AddIntConstant(m, (char *) "CGV_ERR", CGV_ERR);
    PyModule_AddIntConstant(m, (char *) "CGV_INT", CGV_INT);
    PyModule_AddIntConstant(m, (char *) "CGV_LONG", CGV_LONG);
    PyModule_AddIntConstant(m, (char *) "CGV_BOOL", CGV_BOOL);
    PyModule_AddIntConstant(m, (char *) "CGV_REST", CGV_REST);
    PyModule_AddIntConstant(m, (char *) "CGV_STRING", CGV_STRING);
    PyModule_AddIntConstant(m, (char *) "CGV_INTERFACE", CGV_INTERFACE);
    PyModule_AddIntConstant(m, (char *) "CGV_IPV4ADDR", CGV_IPV4ADDR);
    PyModule_AddIntConstant(m, (char *) "CGV_IPV4PFX", CGV_IPV4PFX);
    PyModule_AddIntConstant(m, (char *) "CGV_IPV6ADDR", CGV_IPV6ADDR);
    PyModule_AddIntConstant(m, (char *) "CGV_IPV6PFX", CGV_IPV6PFX);
    PyModule_AddIntConstant(m, (char *) "CGV_MACADDR", CGV_MACADDR);
    PyModule_AddIntConstant(m, (char *) "CGV_URL", CGV_URL);
    PyModule_AddIntConstant(m, (char *) "CGV_UUID", CGV_UUID);
    PyModule_AddIntConstant(m, (char *) "CGV_TIME", CGV_TIME);
    PyModule_AddIntConstant(m, (char *) "CGV_VOID", CGV_VOID);

    return 0;
}

PyObject *
CgVar_Instance(void)
{
    PyObject *Cv;

    Cv = PyObject_CallObject((PyObject *) &CgVar_Type, NULL);

    return Cv;
}    

PyObject *
CgVar_InstanceFromCv(cg_var *cv)
{
    PyObject *Cv;

    if ((Cv = CgVar_Instance()) == NULL)
	return NULL;
    
    if (cv_cp(((CgVar *)Cv)->cv, cv) < 0) {
        PyErr_SetString(PyExc_MemoryError, "failed to allocate memory");
        return NULL;
    }

    return Cv;
}

/*
 * Get cg_var* pointer from CgVar object
 */
cg_var *
CgVar_cv(PyObject *Cv)
{
    if ( ! PyObject_TypeCheck(Cv, &CgVar_Type)) {
	PyErr_SetString(PyExc_ValueError, "Object not CgVar type");
	return NULL;
    }

    return ((CgVar *)Cv)->cv;
}
