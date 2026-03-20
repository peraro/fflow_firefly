# FiniteFlow+Firefly

Note: this currently requires using the `exp` branch of FiniteFlow.

This is a proof-of-concept interface between [FiniteFlow](https://github.com/peraro/finiteflow) and [Firely](https://gitlab.com/firefly-library/firefly).  More specifically, it implements functions to reconstruct the output of FiniteFlow graphs using the reconstruction routines of Firefly.

It is mostly meant to be a test of FiniteFlow's API for evaluating graphs from [threads managed by external programs](https://github.com/peraro/finiteflow/blob/exp/include/fflow/capi_custom_threads.h) such as external reconstruction programs.

It depends on GMP, FLINT, FiniteFlow and Firefly (which need to be installed first and we refer to their documentation) and uses the CMake build system.  A Python interface can be enabled by calling `cmake` with the flag `-DPYTHON_INTERFACE=1`.

While it has been successfully tested in a few examples, this interface is quite hacky and I don't particularly recommend it - but you're free to try it, of course.
