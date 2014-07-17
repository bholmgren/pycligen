/*
  CVS Version: $Id: cligen_cv.h,v 1.3 2013/04/12 10:59:52 olof Exp $ 

  Copyright (C) 2014 Benny Holmgren

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
*/

#ifndef _PY_CLIGEN_PT_H_
#define _PY_CLIGEN_PT_H_

extern PyTypeObject ParseTree_Type;

parse_tree *ParseTree_pt(PyObject *pto);

#endif /* _PY_CLIGEN_CV_H_ */
