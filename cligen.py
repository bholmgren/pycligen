#
#  PyCLIgen module
#
# Copyright (C) 2014 Benny Holmgren
#
#  This file is part of PyCLIgen.
#
#  PyPyCLIgen is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PyCLIgen is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with PyCLIgen; see the file LICENSE.

"""Python binding for the CLIgen library by Olof Hagsand

This module implements a Python API to CLIgen, allowing the developer
to utilize the power of CLIgen without coding in C

"""
# Make python2 behave more like python 3.
from __future__ import unicode_literals, absolute_import, print_function

__version__ = '0.1'


import sys
import ipaddress
if sys.version_info.major < 3:
    from urlparse import urlparse
else:
    from urllib.parse import urlparse
import _cligen
from _cligen import *


    
#
# obsolete: done by CgVar constructor
#
#def cv_parse(str, type, name=None):
#    """Parse a string into a CgVar object.
#
#    Args:
#        str:  A string representing a value to be parsed
#        type: The CLIgen variable type
#        name: The name of the variable (optional)
#
#    Returns:
#        A CgVar object
#
#    Raises:
#        ValueError:  If the string passed cannot be parsed successfully
#        MemoryError: If needed memory was not available
#    """
#
#    cv = CgVar(type)
#    if name is not None:
#        cv.name_set(name)
#    cv.parse(str)
#    return cv
        


#
# CLIgen
#
class CLIgen (_cligen.CLIgen):
    'The CLIgen class'

    def __init__(self, *args, **kwargs):
        """
   Args:
     None
      or
     syntax-spec:  An optional argument specifying a CLIgen syntax format
                   that will be parsed and activated. The specification can 
                   be provided as a normal string format or as named 
                   arguments containing the specification or a filename of
                   a file containing the specification. For example:

                     syntax='myvar="myval";\nhello-world("Greet the world");'
                     file='/usr/local/myapp/myapp.cli'

   Raises:
      TypeError:    If invalid arguments are provided.
      MemoryError:  If memory allcoation fails
      ValueError:   If the string passed cannot be parsed successfully
      """
        numargs = len(args) + len(kwargs)
        if numargs > 1:
            raise TypeError("function takes at most 1 argument ({:d} given)".format(numargs))

        # Call parent to setup CLIgen structures.
        super(CLIgen, self).__init__(*args, **kwargs)

        if numargs is 1:
            if len(kwargs) > 0: # named argument
                if "file" in kwargs:
                    pt = ParseTree(self, file=kwargs['file'])
                elif "syntax" in kwargs:
                    pt = ParseTree(self, syntax=kwargs['syntax'])
                else:
                    raise TypeError("'{:s}' is an invalid keyword argument for this function".format(list(kwargs.keys())[0]))
                
                
            elif len(args) > 0:
                pt = ParseTree(self, syntax=args[0])

            if pt:
                self.tree_add("__CLIgen__", pt)
                self.tree_active_set("__CLIgen__")



    def output(self, file, out):
        """
CLIgen output function. All terminal output should be made via this method

   Args:
      file:  The IO object to direct the output to, such as a file or sys.stdout
      out:   The output string/object

   Returns:
      None

   Raises:
      IOError:  If there is a problem writing to the file object

      """    
        return super(CLIgen, self)._output(file, str(out))




    def _cligen_cb(self, name, vr, arg):
#        module_name, class_name = name.rsplit(".", 1)
#        m = importlib.import_module(module_name)

        if hasattr(sys.modules['__main__'], name) is True:
            return getattr(sys.modules['__main__'], name)(self, vr, arg)
        return None

    def _cligen_expand(self, name, vr, arg):

        if hasattr(sys.modules['__main__'], name) is True:
            cmds = getattr(sys.modules['__main__'], name)(self, name, vr, arg)
            if type(cmds) != list:
                raise TypeError("expand callback must return list of dicts")    
            return cmds
        else:
            return None



#
# CgVar
#
class CgVar (_cligen.CgVar):
    'CLIgen variable object'
    


    def __init__(self, *args, **kwargs):
        """    Optional args:
        type:   The CLIgen variable type
        name:   The name of the variable
        value:  A string representing a value to be parsed

   Returns:
      A new CgVar object

   Raises:
      ValueError:  If the string passed cannot be parsed successfully
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self).__init__(*args, **kwargs)




    def name_get(self):
        """Get CgVar variable name

   Args:
      None

   Returns:
      Name as a string if available or None otherwise
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._name_get()



    def name_set(self, name):
        """Set CgVar variable name

   Args:
      'name':  New name of variable

   Returns:
      New name as a string
   
  Raises:
       MemoryError: If needed memory was not available
       """
        return super(CgVar, self)._name_set(name)



    def type_get(self):
        """Get CgVar variable type

   Args:
      None

   Returns:
      Variable type as int
    
   Raises: 
      None
      """
        return int(super(CgVar, self)._type_get())



    def type_set(self, type):
        """Set CgVar variable type

   Args:
      'type':  New type as int

   Returns:
      New type
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._type_set(type)



    def type2str(self, *args):
        """"Get string name of CgVar type

   Args:
      'type': Optionally specify type, otherwise self's current type is used

   Returns:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._type2str(*args)
        


    def int_get(self):
        """Get CgVar variable int value

   Args:
      None

   Returns:
      The int value
    
   Raises: 
      None
      """
        return super(CgVar, self)._int_get()



    def int_set(self, value):
        """Set CgVar variable int value

   Args:
      'value':  The new value

   Returns:
      The new value
    
   Raises:
      ValueError: If 'self' is not a int
      """
        return super(CgVar, self)._int_set(value)



    def long_get(self):
        """Get CgVar variable long value

   Args:
       None

   Returns:
      The long value
    
   Raises: 
      None
      """
        return super(CgVar, self)._long_get()



    def long_set(self, value):
        """Set CgVar variable long value

   Args:
      'value':  The new value

   Returns:
      The new value
    
   Raises:
      ValueError: If 'self' is not a long
      """
        return super(CgVar, self)._long_set(value)



    def bool_get(self):
        """Get CgVar variable boolean value

   Args:
      None

   Returns:
      True or False
    
   Raises: 
      None
      """
        return super(CgVar, self)._bool_get()



    def bool_set(self, boolean):
        """Set CgVar variable boolean value

   Args:
      'boolean':  The status as a boolean 

   Returns:
      The new value
    
   Raises:
      ValueError: If 'self' is not a boolean
      """
        return super(CgVar, self)._bool_set(boolean)


    def string_get(self):
        """Get CgVar variable string value

   Args:
      None

   Returns:
     The string value or None if not set
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._string_get()



    def string_set(self, string):
        """Set CgVar variable string value

   Args:
      'string':  New value of variable

   Returns:
      The new value
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._string_set(string)




    def ipv4addr_get(self):
        """Get IPv4 address value from CgVar object.

   Args:
      None

   Returns:
      An ipaddress.IPv4Address object

   Raises:
      TypeError:  If the CgVar object is not of the types CGV_IPV4ADDR
                  of CGV_IPV4PFX.
   """
        return ipaddress.IPv4Address(super(CgVar, self)._ipv4addr_get())


 
    def ipv4masklen_get(self):
        """Get mask length of IPv4 prefix value from CgVar object.

   Args:
      None

   Returns:
      The mask length as an int

   Raises:
      TypeError:  If the CgVar object is not of the types CGV_IPV4ADDR
                  of CGV_IPV4PFX.
    """
        return super(CgVar, self)._ipv4masklen_get()


#    def ipv4addr_set(self, addr):
#        """Get IPv4 address value from CgVar object.
#
#    Args:
#        addr: An ipaddress.IPv4Address object
#
#    Returns:
#        True if successful
#
#    Raises:
#        TypeError:  If the CgVar object is not of the type CGV_IPV4ADDR
#        MemoryError: If needed memory was not available
#
#    """
#        if self.type_get() is not CGV_IPV4ADDR:
#            raise TypeError("invalid type")    
#        return self.parse(str(addr))



#    def ipv4prefix_set(self, pfx):
#        """Get IPv4 address value from CgVar object.
#
#    Args:
#        pfx: An ipaddress.IPv4Network object
#
#    Returns:
#        True if successful
#
#    Raises:
#        TypeError:  If the CgVar object is not of the types CGV_IPV4PFX.
#        MemoryError: If needed memory was not available
#
#    """
#        if self.type_get() is not CGV_IPV4PFX:
#            raise TypeError("invalid type")    
#        return self.parse(str(pfx))



    def ipv6addr_get(self):
        """Get IP v6 address value from CgVar object.

   Args:
      None

   Returns:
      An ipaddress.IPv6Address object

   Raises:
      TypeError:  If the CgVar object is not of the types CGV_IPV6ADDR
                  of CGV_IPV6PFX.
    """
        return ipaddress.IPv6Address(super(CgVar, self)._ipv6addr_get())


    def ipv6masklen_get(self):
        """Get mask length of IPv6 prefix value from CgVar object.

   Args:
      None

   Returns:
      The mask length as an int

   Raises:
      TypeError:  If the CgVar object is not of the types CGV_IPV6ADDR
                  of CGV_IPV6PFX.
    """
        return super(CgVar, self)._ipv6masklen_get()



#    def ipv6addr_set(self, addr):
#        """Get IPv6 address value from CgVar object.
#
#    Args:
#        addr: An ipaddress.IPv6Address object
#
#    Returns:
#        True if successful
#
#    Raises:
#        TypeError:  If the CgVar object is not of the type CGV_IPV6ADDR
#        MemoryError: If needed memory was not available
#
#    """
#        if self.type_get() is not CGV_IPV6ADDR:
#            raise TypeError("invalid type")    
#        return self.parse(str(addr))



#    def ipv6prefix_set(self, pfx):
#        """Get IPv6 address value from CgVar object.
#
#    Args:
#        pfx: An ipaddress.IPv6Network object
#
#    Returns:
#        True if successful
#
#    Raises:
#        TypeError:  If the CgVar object is not of the types CGV_IPV6PFX.
#        MemoryError: If needed memory was not available
#
#    """
#        if self.type_get() is not CGV_IPV6PFX:
#            raise TypeError("invalid type")    
#        return self.parse(str(pfx))


    def mac_get(self):
        """Get CgVar variable MAC address value

   Args:
      None

   Returns:
     The MAC address value as a 'long'
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._mac_get()



    def uuid_get(self):
        """Get CgVar variable UUID value

   Args:
      None

   Returns:
     The UUID as an 'uuid' object
    
   Raises:
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._uuid_get()




    def time_get(self):
        """Get time value of CgVar object.

   Args:
      None

   Returns:
      The time since the epoch as a 'float' object

   Raises:
      MemoryError: If needed memory was not available
    """
        return super(CgVar, self)._time_get()



    def time_set(self, timespec):
        """Set time value of CgVar object.

   Args:
      timespec: The time specification which can either be a Python 'float' or
      'int' object specifying seconds since the epoch or a 'string' to be
       parsed by CLIgen.

   Returns:
      The new value as a 'float' object

   Raises:
      TypeError:   If self is not a CGV_TIME or timespec is an invalid type
      ValueError:  If timespec could not be parsed
      MemoryError: If needed memory was not available
    """
        if self.type_get() is not CGV_TIME:
            raise TypeError("'self' is of invalid type")

        if isinstance(timespec, int) or isinstance(timespec, float):
            super(CgVar, self)._time_set(str(float(timespec)))
        elif isinstance(timespec, str):
            self.parse(timespec)
            
        return self.time_get()


    def url_get(self):
        """Get URL value from CgVar object.
   Args:
      None

   Returns:
      A populated urlib/urlparse ParseResult object

   Raises:
      TypeError:   If self is not CGV_URL type
      """

        if self.type_get() is not CGV_URL:
            raise TypeError("'self' is of invalid type")

        return urlparse(str(self))

    def url_set(self, url):
        """Set URL value to CgVar object.
   Args:
      'url':  A string representing the url

   Returns:
      A populated urlib/urlparse ParseResult object

   Raises:
      ValueError:  If 'string' could not be parsed
      MemoryError: If needed memory was not available
      """

        if self.type_get() is not CGV_URL:
            raise TypeError("'self' is of invalid type")

        self.parse(url)
        return self.url_get()

        

    def parse(self, string):
        """Parse a string representation of a value and assign the result 
to 'self'.  The parsing is based on the current type set.

   Args:
      'string': The string representation of the value. Ex: "1.2.3.4" or
                "01:00:af:43:fd:aa"

   Returns:
       True on success

   Raises:
      ValueError:  If 'string' could not be parsed
      MemoryError: If needed memory was not available
      """
        return super(CgVar, self)._parse(string)


#
# Cvec
#
class Cvec ():
    'A vector of CgVar'

    def __init__(self):
        self._cvec = []
        
    def __str__(self):

        return self._cvec.__str__()

#        str = "["
#        for cv in self._cvec:
#            str += "$%s=%s" % (cv.name_get(), cv)
#        str += "]"
#
#        return str

    def __repr__(self):
        return self._cvec.__repr__()

    def __getitem__(self, key):

        if isinstance(key, int):
            return self._cvec[key]

        elif isinstance(key, str):
            for cv in self._cvec:
                if cv.name_get() == key:
                    return cv
            raise IndexError("element not found")

        else:
            raise TypeError('key must be int or str')
    

    def __setitem__(self, key, cv):
        if isinstance(key, int):
            pass
        elif isinstance(key, str):
            for idx, c in enumerate(self._cvec):
                if c.name_get() == key:
                    key = idx
                    break
            if isinstance(key, str):  # not found
                raise IndexError("element not found")

        else:
            raise TypeError('key must be int or str')

        if (isinstance(cv, CgVar)):
            self._cvec[key] = cv;
        elif isinstance(cv, str):
            self._cvec[key].parse(cv)
        else:
            raise TypeError('cv must be CgVar or str')

        return self._cvec[key]


    def __iter__(self):
        for cv in self._cvec:
            yield cv

    def __len__(self):
        return len(self._cvec)
    
    def __contains__(self, key):
        for cv in self._cvec:
            if cv.name_get() == key:
                return True
        return False

    def append(self, arg):
        if isinstance(arg, int):
            cv = CgVar(arg)
        elif isinstance(arg, CgVar):
            cv = arg
        else:
            raise TypeError("argument must be int or CgVar")

        self._cvec.append(cv)
        return cv;

    def keys(self):
        keys = []
        for cv in self._cvec:
            if cv.name_get() != None:
                keys.append(cv.name_get()) 
        return keys                        




#
# ParseTree
#
#class ParseTree():
#    'CLIgen parse tree'
#
#    def __init__(self, *args, **kwargs):
#        """ParseTree constructor. 
#        
#Takes one named argument:
#  string='<syntax format>' - String will be parsed as CLIgen syntax.
#  file='<filename>'        - Argument specifies a file containing CLIgen syntax.
#If argument is unnamed, itinplies a "string" argument.
#"""
##        super(ParseTree, self).__init__(*args, **kwargs)
#        if len(kwargs) == 1:
#            if "file" in kwargs:
#                with open(kwargs['file'], 'r') as f:
#                    self.syntax = f.read()
#            elif "string" in kwargs:
#                self.syntax = kwargs['string']
#            else:
#                raise AttributeError("Argument named 'string' or 'file' expected")
#            
#        elif len(args) == 1:
#            self.syntax = args[0]
#
#        else:
#            raise TypeError("__init__() takes 1 positional argument")
#
#        if isinstance(self.syntax, str) is False:
#                raise AttributeError("Argument must be string")
#
#




def type2str(type):
    """Get string representation of a CLIgen type

    Args:
        'type': The CLIgen type

    Returns:
        MemoryError: If needed memory was not available
        """
    return CgVar().type2str(type)


# Testing..
#_types = {
#    type2str(CGV_INT) : CGV_INT,
#    type2str(CGV_LONG) : CGV_LONG,
#    type2str(CGV_BOOL) : CGV_BOOL,
#    type2str(CGV_STRING) : CGV_STRING,
#    type2str(CGV_REST) : CGV_REST,
#    type2str(CGV_INTERFACE) : CGV_INTERFACE,
#    type2str(CGV_IPV4ADDR) : CGV_IPV4ADDR,
#    type2str(CGV_IPV4PFX) : CGV_IPV4PFX,
#    type2str(CGV_IPV6ADDR) : CGV_IPV6ADDR,
#    type2str(CGV_IPV6PFX) : CGV_IPV6PFX,
#    type2str(CGV_MACADDR) : CGV_MACADDR,
#    type2str(CGV_URL) : CGV_URL,
#    type2str(CGV_UUID) : CGV_UUID,
#    type2str(CGV_TIME) : CGV_TIME,
#}
