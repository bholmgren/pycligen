import sys
import ipaddress
import _cligen
from _cligen import *

def cv_parse(str, type, name=None):
    """Parse a string into a CgVar object.

    Args:
        str:  A string representing a value to be parsed
        type: The CLIgen variable type
        name: The name of the variable (optional)

    Returns:
        A CgVar object

    Raises:
        ValueError:  If the string passed cannot be parsed successfully
        MemoryError: If needed memory was not available
    """

    cv = CgVar(type)
    if name is not None:
        cv.name_set(name)
    cv.parse(str)
    return cv
        

#
# CLIgen
#
class CLIgen (_cligen.CLIgen):
    'The CLIgen class'

    def __init__(self, *args, **kwargs):
        if len(kwargs) == 1:
            if "file" in kwargs:
                with open(kwargs['file'], 'r') as f:
                    self.syntax = f.read()
            elif "string" in kwargs:
                self.syntax = kwargs['string']
            else:
                raise AttributeError("Argument named 'string' or 'file' expected")
            super().__init__(self.syntax)

        else:
            super().__init__()


    def _cligen_cb(self, name, vr, arg):
#        module_name, class_name = name.rsplit(".", 1)
#        m = importlib.import_module(module_name)

        if hasattr(sys.modules['__main__'], name) is True:
            return getattr(sys.modules['__main__'], name)(self, vr, arg)
        return None



#
# CgVar
#
class CgVar (_cligen.CgVar):
    'A CLIgen variable object'

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
        return ipaddress.IPv4Address(super()._ipv4addr_get())


 
    def ipv4masklen_get(self):
        """Get mask length of IPv4 prefix value from CgVar object.

    Args:
        None

    Returns:
        An int

    Raises:
        TypeError:  If the CgVar object is not of the types CGV_IPV4ADDR
                    of CGV_IPV4PFX.
    """
        return super()._ipv4masklen_get()


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
        return ipaddress.IPv6Address(super()._ipv6addr_get())


    def ipv6masklen_get(self):
        """Get mask length of IPv6 prefix value from CgVar object.

    Args:
        None

    Returns:
        An int

    Raises:
        TypeError:  If the CgVar object is not of the types CGV_IPV6ADDR
                    of CGV_IPV6PFX.
    """
        return super()._ipv6masklen_get()



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




#
# Cvec
#
class Cvec ():
    'A vector of CgVar'

    def __init__(self):
#        print("Cvec.__init__")
        self._cvec = []
#        super().__init__(self)
        
#    def __del__(self):
#        print("Cvec.__del__")
#        del self._cvec[:]
#        self._cvec = None  # XXX free list??

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
            pass
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
    
    def haskey(self, key):
        for cv in self._cvec:
            if cv.name_get() == key:
                return True
        return False

    def add(self, type):
        cv = CgVar(type)
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
##        super().__init__(*args, **kwargs)
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
