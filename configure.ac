#
# Copyright (C) 2014 Benny Holmgren
#
# This file is part of PyCLIgen.
#
# PyCLIgen is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# PyCLIgen is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along wth PyCLIgen; see the file LICENSE.  If not, see
# <http://www.gnu.org/licenses/>.
#
#

dnl the pycligen version number
m4_define(pycligen_major_version, 0)
m4_define(pycligen_minor_version, 1)
m4_define(pycligen_version, pycligen_major_version.pycligen_minor_version)

dnl versions of packages we require ...
m4_define(python_required_version, 3.3)

AC_INIT(pycligen, pycligen_version, [http://github.com/bholmgren/pycligen])
AC_DEFINE(PYCLIGEN_MAJOR_VERSION, pycligen_major_version, [pycligen major version])
AC_DEFINE(PYCLIGEN_MINOR_VERSION, pycligen_minor_version, [pycligen minor version])

AC_CONFIG_SRCDIR([./pycligen.c])
AC_CONFIG_HEADER(pycligen_config.h)
PKG_PROG_PKG_CONFIG
AC_CANONICAL_TARGET
AC_DEFINE(AUTOCONF)
AC_SUBST(CC)
AC_SUBST(INCLUDES)
AC_SUBST(CPPFLAGS)
AC_SUBST(CFLAGS)
AC_SUBST(LIBS)
#
AC_PROG_CC()
AC_PROG_CPP
AC_PROG_INSTALL
CPPFLAGS="-DHAVE_PYCLIGEN_CONFIG_H"
CFLAGS="-Wall ${CFLAGS}"
LDFLAGS="-g ${LDFLAGS}"

# Check Python >= 3.3
PKG_CHECK_MODULES([python], [python3 >= 3.3])
# Check CLIgen 
AC_CHECK_HEADERS(cligen/cligen.h,,AC_MSG_ERROR("cligen headers are required. Install and rerun configure."))
AC_CHECK_LIB(cligen,cligen_init,,AC_MSG_ERROR("cligen libs are required. Install and rerrun configre."))


# This is where CLICON include files and libs are
#AC_ARG_WITH(clicon,
#[  --with-clicon=PATH	  set clicon sdk installation PATH],
#  INCLUDES="${INCLUDES} -I$withval/include"
#  LIBS="${LIBS} -L $withval/lib "
#)

AC_OUTPUT(Makefile)