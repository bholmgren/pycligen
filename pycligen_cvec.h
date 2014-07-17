/*
  CVS Version: $Id: cligen_cv.h,v 1.3 2013/04/12 10:59:52 olof Exp $ 

  Copyright (C) 2001-2013 Olof Hagsand

  This file is part of CLIgen.

  CLIgen is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  CLIgen is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with CLIgen; see the file COPYING.

  Private cg_var type definition. Dont include this to external apps, only 
  internal cligen
*/

#ifndef _PY_CLIGEN_CVEC_H_
#define _PY_CLIGEN_CVEC_H_

#if 0
typedef struct {
    PyObject_HEAD
    cvec *vr;
} Cvec;
#endif

PyObject *Cvec_from_cvec(PyObject *self, cvec *vr);

#endif /* _PY_CLIGEN_CVEC_H_ */
