# Python API of finiteflow+firefly interface

from _cffi_fflowfirefly import lib as _lib, ffi as _ffi
import fflow as _ff


# verbosity options
FIREFLY_SILENT = _lib.FF_FIREFLY_SILENT
FIREFLY_IMPORTANT = _lib.FF_FIREFLY_IMPORTANT
FIREFLY_CHATTY = _lib.FF_FIREFLY_CHATTY


class FireflyRatFunList:
    '''A list of rational functions reconstructed by Firefly'''
    def __init__(self):
        self._ptr = _ffi.NULL

    def __del__(self):
        _lib.ffFireflyRatFunListFree(self._ptr)

    def nvars(self):
        if self._ptr == _ffi.NULL:
            return 0
        return _lib.ffFireflyRatFunListNVars(self._ptr)

    def size(self):
        if self._ptr == _ffi.NULL:
            return 0
        return _lib.ffFireflyRatFunListLen(self._ptr)

    def __len__(self):
        return self.size()

    def to_string(self, svars, idx=None):
        if len(svars) != self.nvars():
            raise ValueError("List svars must have the same length as the "
                             "number of variables self.nvars()")
        if idx is None:
            return list(self.to_string(svars,i) for i in range(self.size()))
        else:
            cvars = [_ffi.new("char[]", x.encode('utf8')) for x in svars]
            cstr = _lib.ffFireflyRatFunToStr(self._ptr,idx,cvars)
            if cstr == _ffi.NULL:
                raise _ff.FFlowError()
            ret = _ffi.string(cstr).decode()
            _ff._lib.ffFreeCStr(cstr)
            return ret


def FireflyReconstructFunction(graph, **kwargs):
    '''Reconstruct the output of a FinieFlow graph with Firefly.
    Available options are n_threads, bunch_size and verbosity.
    '''
    recopt = _ffi.new("FFFireflyRecOptions *",kwargs)
    ret = _lib.ffFireflyReconstructFunction(graph,recopt[0])
    if ret == _ffi.NULL:
        return _ff.FFlowError()
    else:
        rec = FireflyRatFunList()
        rec._ptr = ret
        return rec


if __name__ == '__main__':
    pass
