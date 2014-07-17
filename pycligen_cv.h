/* 
 * pycligen_cv.h
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

#ifndef _PY_CLIGEN_CV_H_
#define _PY_CLIGEN_CV_H_

typedef struct {
    PyObject_HEAD
    cg_var *cv;
} CgVar;

extern PyTypeObject CgVarType;

CgVar *CgVar_Instance();
CgVar *CgVar_InstanceFromCv(cg_var *cv);

#endif /* _PY_CLIGEN_CV_H_ */
