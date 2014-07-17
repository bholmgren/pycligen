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

cg_fnstype_t *CLIgen_str2fn(char *name, void *arg, char **error);

#endif /* __PY_CLIGEN_H__ */
