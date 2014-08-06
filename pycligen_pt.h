/* 
 * pycligen_pt.h
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

#ifndef _PY_CLIGEN_PT_H_
#define _PY_CLIGEN_PT_H_

int ParseTree_init_object(PyObject *m);

extern PyTypeObject ParseTree_Type;

parse_tree *ParseTree_pt(PyObject *pto);

int ParseTree_name_set(PyObject *obj, const char *name);
char *ParseTree_name(PyObject *obj);

#endif /* _PY_CLIGEN_PT_H_ */
