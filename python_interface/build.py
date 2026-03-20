#! /usr/bin/env python3

import pathlib
from cffi import FFI
import sys

thisfile = pathlib.Path(__file__).resolve()

ffibuilder = FFI()

header = open(str((thisfile.parent.parent / 'include' / 'fflowfirefly.h').resolve())).read()
header = header.split("/* API begin */")[1].split("/* API end */")[0]

ffibuilder.cdef("typedef unsigned FFGraph;\ntypedef const char * FFCStr;\n" + header)

ffibuilder.set_source("_cffi_fflowfirefly",
                      r'''
                      #include <fflowfirefly.h>
                      '''
                      )

if __name__ == "__main__":
    pathlib.Path("_cffi_fflowfirefly.c").touch()
    ffibuilder.emit_c_code("_cffi_fflowfirefly.c")
