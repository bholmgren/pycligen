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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cligen/cligen.h>

#include "pycligen.h"
#include "pycligen_cv.h"


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
    PyObject *ret;
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
    if (value && (Str = PyObject_Str(value))) {
	ret = PyObject_CallMethod((PyObject *)self, "_parse", "O", Str);
	Py_DECREF(Str);
	if (ret == NULL)
	    return -1;
	Py_DECREF(ret);
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
_CgVar_name_get(CgVar *self)
{
    char *str;

    assert(self->cv);

    str = cv_name_get(self->cv);
    if (str == NULL)
	Py_RETURN_NONE;
    
    return StringFromString(str);
}

static PyObject *
_CgVar_name_set(CgVar *self, PyObject *args)
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
_CgVar_type_get(CgVar *self)
{
    assert(self->cv);

    return PyLong_FromLong(cv_type_get(self->cv));
}

static PyObject *
_CgVar_type_set(CgVar *self, PyObject *args)
{
    enum cv_type type;
    cg_var *cv;

    assert(self->cv);

    if (!PyArg_ParseTuple(args, "i", &type))
        return NULL;

    if (type != cv_type_get(self->cv)) {
	
	/* Clean-up by creating new cg_var and free old */
	if ((cv = cv_new(type)) == NULL || 
	    (cv_name_get(self->cv) && !cv_name_set(cv, cv_name_get(self->cv)))){
	    PyErr_SetString(PyExc_MemoryError, "cv_new");
	    return NULL;
	}
	cv_free(self->cv);
	self->cv = cv;
    }
    
    return PyLong_FromLong(type);
}

static PyObject *
_CgVar_type2str(CgVar *self, PyObject *args)
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
_CgVar_int8_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_INT8))
        return NULL;
	
    return PyLong_FromLong(cv_int8_get(self->cv));
}

static PyObject *
_CgVar_int8_set(CgVar *self, PyObject *args)
{
    int64_t num;

    if (CgVar_type_verify(self, CGV_INT8))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "l", &num))
        return NULL;

    cv_int8_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_int16_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_INT16))
        return NULL;
	
    return PyLong_FromLong(cv_int16_get(self->cv));
}

static PyObject *
_CgVar_int16_set(CgVar *self, PyObject *args)
{
    int64_t num;

    if (CgVar_type_verify(self, CGV_INT16))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "l", &num))
        return NULL;

    cv_int16_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_int32_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_INT32))
        return NULL;
	
    return PyLong_FromLong(cv_int32_get(self->cv));
}

static PyObject *
_CgVar_int32_set(CgVar *self, PyObject *args)
{
    int64_t num;

    if (CgVar_type_verify(self, CGV_INT32))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "l", &num))
        return NULL;

    cv_int32_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_int64_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_INT64))
        return NULL;
	
    return PyLong_FromLong(cv_int64_get(self->cv));
}

static PyObject *
_CgVar_int64_set(CgVar *self, PyObject *args)
{
    int64_t num;

    if (CgVar_type_verify(self, CGV_INT64))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "l", &num))
        return NULL;

    cv_int64_set(self->cv, num);
    
    return PyLong_FromLong(num);
}


static PyObject *
_CgVar_uint8_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_UINT8))
        return NULL;
	
    return PyLong_FromLong(cv_uint8_get(self->cv));
}

static PyObject *
_CgVar_uint8_set(CgVar *self, PyObject *args)
{
    uint64_t num;

    if (CgVar_type_verify(self, CGV_UINT8))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "L", &num))
        return NULL;

    cv_uint8_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_uint16_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_UINT16))
        return NULL;
	
    return PyLong_FromLong(cv_uint16_get(self->cv));
}

static PyObject *
_CgVar_uint16_set(CgVar *self, PyObject *args)
{
    uint64_t num;

    if (CgVar_type_verify(self, CGV_UINT16))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "L", &num))
        return NULL;

    cv_uint16_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_uint32_get(CgVar *self)
{
    uint32_t v;
    
    if (CgVar_type_verify(self, CGV_UINT32))
        return NULL;
	
    v = cv_uint32_get(self->cv);
    return PyLong_FromLong(v);
}

static PyObject *
_CgVar_uint32_set(CgVar *self, PyObject *args)
{
    uint64_t num;

    if (CgVar_type_verify(self, CGV_UINT32))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "L", &num))
        return NULL;

    cv_uint32_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_uint64_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_UINT64))
        return NULL;
	
    return PyLong_FromLong(cv_uint64_get(self->cv));
}

static PyObject *
_CgVar_uint64_set(CgVar *self, PyObject *args)
{
    uint64_t num;

    if (CgVar_type_verify(self, CGV_UINT64))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "L", &num))
        return NULL;

    cv_uint64_set(self->cv, num);
    
    return PyLong_FromLong(num);
}

static PyObject *
_CgVar_dec64_get(CgVar *self)
{
    char *dec64;
    PyObject *Dec64;

    if (CgVar_type_verify(self, CGV_DEC64))
        return NULL;

    if ((dec64 = cv2str_dup(self->cv)) == NULL)
	return PyErr_NoMemory();
    Dec64 = StringFromString(dec64);
    free(dec64);

    return Dec64;
}

static PyObject *
_CgVar_dec64_set(CgVar *self, PyObject *args)
{
    PyObject *Dec64;
    int n;

    if (CgVar_type_verify(self, CGV_DEC64))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "Oi", &Dec64, &n))
        return NULL;

    cv_dec64_n_set(self->cv, n);
    if (PyObject_CallMethod((PyObject *)self, "_parse", "O", Dec64) == NULL)
	return NULL;
    
    return _CgVar_dec64_get(self);
}

static PyObject *
_CgVar_bool_get(CgVar *self)
{
    if (CgVar_type_verify(self, CGV_BOOL))
        return NULL;
	
    if (cv_bool_get(self->cv) == 0)
	Py_RETURN_FALSE;
    else
	Py_RETURN_TRUE;
}

static PyObject *
_CgVar_bool_set(CgVar *self, PyObject *args)
{
    PyObject *bool;

    if (CgVar_type_verify(self, CGV_BOOL))
        return NULL;
	
    if (!PyArg_ParseTuple(args, "O!", &PyBool_Type, &bool))
        return NULL;

    cv_bool_set(self->cv, (bool == Py_True) ? 1 : 0);
    return _CgVar_bool_get(self);
}

static PyObject *
_CgVar_string_get(CgVar *self)
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
_CgVar_string_set(CgVar *self, PyObject *args)
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
_CgVar_mac_get(CgVar *self)
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
_CgVar_uuid_get(CgVar *self)
{
    char *uuidstr;
    PyObject *uuid;
    PyObject *mod;
    PyObject *ret;

    if (CgVar_type_verify(self, CGV_UUID))
        return NULL;

    if ((uuidstr = cv2str_dup(self->cv)) == NULL)
	return PyErr_NoMemory();
    uuid = StringFromString(uuidstr);
    free(uuidstr);
    if (uuid == NULL)
	return NULL;

    if ((mod = PyImport_ImportModule("uuid")) == NULL) {
	Py_DECREF(uuid);
	return NULL; 
    }
    
    ret = PyObject_CallMethod(mod, "UUID", "O", uuid);
    Py_DECREF(uuid);
    Py_DECREF(mod);
    
    return ret;
}

static PyObject *
_CgVar_time_get(CgVar *self)
{
    size_t len;
    char *fstr;
    struct timeval t;
    PyObject *tstr;
    PyObject *tobj;

    if (CgVar_type_verify(self, CGV_TIME))
        return NULL;

    t = cv_time_get(self->cv);
    len = snprintf(NULL, 0, "%ld",  t.tv_sec) +
	strlen(".") + snprintf(NULL, 0, "%ld",  t.tv_usec) + 1;
    
    if ((fstr = malloc(len)) == NULL)
	return PyErr_NoMemory();
    snprintf(fstr, len, "%ld.%ld",  t.tv_sec, t.tv_usec);

    tstr = StringFromString(fstr);
    free(fstr);
    if (tstr == NULL)
	return NULL;

    tobj = PyFloat_FromString(tstr);
    Py_DECREF(tstr);

    return tobj;
}

static PyObject *
_CgVar_time_set(CgVar *self, PyObject *args)
{
    char *secs;
    char *usecs;
    struct timeval t;

    if (CgVar_type_verify(self, CGV_TIME))
        return NULL;

    if (!PyArg_ParseTuple(args, "s", &secs))
        return NULL;

    if ((usecs  = strchr(secs, '.')) != NULL) {
	*usecs++ = '\0';
	t.tv_usec = strtoul(usecs, NULL, 10);
    }
    t.tv_sec = strtoul(secs, NULL, 10);

    cv_time_set(self->cv, t);
    
    return _CgVar_time_get(self);
}

static PyObject *
_CgVar_parse(CgVar *self, PyObject *args)
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
    
    /* XXX begin hack */
    if (cv_type_get(cv) == CGV_BOOL) {  /* CLIgen wants lowercase */
	if (strlen(str) > 0)
	    *str = tolower(*str);
    } else if (cv_type_get(cv) == CGV_DEC64) {
	char *dot = strrchr(str, '.');
	cv_dec64_n_set(cv, (dot ? strlen(dot+1) : 1));
    }
    /* XXX end hack */
    
    if (cv_parse(str, cv) < 0) {
	PyErr_Format(PyExc_ValueError, "invalid format for type '%s'",
		     cv_type2str(cv_type_get(cv)));
	cv_free(cv);
	return NULL;
    }

    cv_free(self->cv);
    self->cv = cv;

    Py_RETURN_TRUE;
}

static PyObject *
_CgVar__cmp__(CgVar *self, PyObject *args)
{
    CgVar *other;

    if (!PyArg_ParseTuple(args, "O!", &CgVar_Type, &other))
        return NULL;

    return PyInt_FromLong(cv_cmp(self->cv, other->cv));
}

static PyObject *
_CgVar__copy__(CgVar *self)
{
    return (PyObject *)CgVar_Instance(self->cv);
}



/*
 * Internal method: Get cg_var* pointer to self->cv. 
 */
static PyObject *
__CgVar_cv(CgVar *self)
{
    PyObject *Cv;

    if ((Cv = PyCapsule_New((void *)self->cv, NULL, NULL)) == NULL)
	return NULL;

    return Cv;
}


static PyMethodDef CgVar_methods[] = {
    {"_name_get", (PyCFunction)_CgVar_name_get, METH_NOARGS,
    "Return the name of the variable"
    },
    {"_name_set", (PyCFunction)_CgVar_name_set, METH_VARARGS,
    "Set the name of the variable"
    },

    {"_type_get", (PyCFunction)_CgVar_type_get, METH_NOARGS, 
     "Return the type of the variable"
    },
    {"_type_set", (PyCFunction)_CgVar_type_set, METH_VARARGS, 
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

    {"_int_get", (PyCFunction)_CgVar_int32_get, METH_NOARGS, 
     "Return the int value of the variable"
    },
    {"_int_set", (PyCFunction)_CgVar_int32_set, METH_VARARGS, 
     "Set the int value of the variable"
    },

    {"_long_get", (PyCFunction)_CgVar_int64_get, METH_NOARGS, 
     "Return the long value of the variable"
    },
    {"_long_set", (PyCFunction)_CgVar_int64_set, METH_VARARGS, 
     "Set the bool value of the variable"
    },

    {"_int8_get", (PyCFunction)_CgVar_int8_get, METH_NOARGS, 
     "Return the 8-bit int value of the variable"
    },
    {"_int8_set", (PyCFunction)_CgVar_int8_set, METH_VARARGS, 
     "Set the 8-bit int value of the variable"
    },

    {"_int16_get", (PyCFunction)_CgVar_int16_get, METH_NOARGS, 
     "Return the 16-bit int value of the variable"
    },
    {"_int16_set", (PyCFunction)_CgVar_int16_set, METH_VARARGS, 
     "Set the 16-bit int value of the variable"
    },

    {"_int32_get", (PyCFunction)_CgVar_int32_get, METH_NOARGS, 
     "Return the 32-bit int value of the variable"
    },
    {"_int32_set", (PyCFunction)_CgVar_int32_set, METH_VARARGS, 
     "Set the 32-bit int value of the variable"
    },

    {"_int64_get", (PyCFunction)_CgVar_int64_get, METH_NOARGS, 
     "Return the 64-bit int value of the variable"
    },
    {"_int64_set", (PyCFunction)_CgVar_int64_set, METH_VARARGS, 
     "Set the 64-bit int value of the variable"
    },

    {"_uint8_get", (PyCFunction)_CgVar_uint8_get, METH_NOARGS, 
     "Return the unsigned 8-bit int of the variable"
    },
    {"_uint8_set", (PyCFunction)_CgVar_uint8_set, METH_VARARGS, 
     "Set the unsigned 8-bit int of the variable"
    },

    {"_uint16_get", (PyCFunction)_CgVar_uint16_get, METH_NOARGS, 
     "Return the unsigned 16-bit int of the variable"
    },
    {"_uint16_set", (PyCFunction)_CgVar_uint16_set, METH_VARARGS, 
     "Set the unsigned 16-bit int of the variable"
    },

    {"_uint32_get", (PyCFunction)_CgVar_uint32_get, METH_NOARGS, 
     "Return the unsigned 32-bit int of the variable"
    },
    {"_uint32_set", (PyCFunction)_CgVar_uint32_set, METH_VARARGS, 
     "Set the unsigned 32-bit int of the variable"
    },

    {"_uint64_get", (PyCFunction)_CgVar_uint64_get, METH_NOARGS, 
     "Return the unsigned 64-bit int of the variable"
    },
    {"_uint64_set", (PyCFunction)_CgVar_uint64_set, METH_VARARGS, 
     "Set the unsigned 64-bit int of the variable"
    },

    {"_dec64_get", (PyCFunction)_CgVar_dec64_get, METH_NOARGS, 
     "Return the 64-bit decimal value of the variable"
    },
    {"_dec64_set", (PyCFunction)_CgVar_dec64_set, METH_VARARGS, 
     "Set the 64-bit decimal value of the variable"
    },

    {"_bool_get", (PyCFunction)_CgVar_bool_get, METH_NOARGS, 
     "Return the boolean value of the variable"
    },
    {"_bool_set", (PyCFunction)_CgVar_bool_set, METH_VARARGS, 
     "Set the boolean value of the variable"
    },

    {"_string_get", (PyCFunction)_CgVar_string_get, METH_NOARGS, 
     "Return the string value of the variable"
    },
    {"_string_set", (PyCFunction)_CgVar_string_set, METH_VARARGS, 
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

    {"_mac_get", (PyCFunction)_CgVar_mac_get, METH_NOARGS, 
     "Get the MAC address value of the variable"
    },

    {"_uuid_get", (PyCFunction)_CgVar_uuid_get, METH_NOARGS, 
     "Get the UUID value of the variable"
    },

    {"_time_get", (PyCFunction)_CgVar_time_get, METH_NOARGS, 
     "Get the time value of the variable"
    },

    {"_time_set", (PyCFunction)_CgVar_time_set, METH_VARARGS, 
     "Set the time value of the variable"
    },

    {"_parse", (PyCFunction)_CgVar_parse, METH_VARARGS, 
     "Parse a string representation of a value into a CgVar"
    },


    {"_type2str", (PyCFunction)_CgVar_type2str, METH_VARARGS, 
     "Get the string representation of variable value"
    },

    {"__cmp__", (PyCFunction)_CgVar__cmp__, METH_VARARGS, 
     "Compare CgVars"
    },

    {"__copy__", (PyCFunction)_CgVar__copy__, METH_NOARGS, 
     "Copy CgVar"
    },

    {"__cv", (PyCFunction)__CgVar_cv, METH_NOARGS, 
     "Get a pointer to self->cv"
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
    PyModule_AddIntConstant(m, (char *) "CGV_INT8", CGV_INT8);
    PyModule_AddIntConstant(m, (char *) "CGV_INT16", CGV_INT16);
    PyModule_AddIntConstant(m, (char *) "CGV_INT32", CGV_INT32);
    PyModule_AddIntConstant(m, (char *) "CGV_INT64", CGV_INT64);
    PyModule_AddIntConstant(m, (char *) "CGV_UINT8", CGV_UINT8);
    PyModule_AddIntConstant(m, (char *) "CGV_UINT16", CGV_UINT16);
    PyModule_AddIntConstant(m, (char *) "CGV_UINT32", CGV_UINT32);
    PyModule_AddIntConstant(m, (char *) "CGV_UINT64", CGV_UINT64);
    PyModule_AddIntConstant(m, (char *) "CGV_DEC64", CGV_DEC64);
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
    /* Compat */
    PyModule_AddIntConstant(m, (char *) "CGV_INT", CGV_INT32);
    PyModule_AddIntConstant(m, (char *) "CGV_LONG", CGV_INT64);

    return 0;
}

PyObject *
CgVar_Instance(cg_var *cv)
{
    PyObject *Cv;

    Cv = PyObject_CallMethod(__cligen_module(), "CgVar", NULL);
    if (Cv == NULL)
	return NULL;

    if (cv) {
	if (cv_cp(((CgVar *)Cv)->cv, cv) < 0) {
	    PyErr_SetString(PyExc_MemoryError, "failed to allocate memory");
	    return NULL;
	}
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
