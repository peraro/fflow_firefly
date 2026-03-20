# Taken from symengine

include(LibFindMacros)

libfind_include(firefly/FFInt.hpp firefly)
libfind_library(firefly firefly)

set(FIREFLY_LIBRARIES ${FIREFLY_LIBRARY})
set(FIREFLY_INCLUDE_DIRS ${FIREFLY_INCLUDE_DIR})
set(FIREFLY_TARGETS firefly)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FIREFLY DEFAULT_MSG FIREFLY_LIBRARIES
    FIREFLY_INCLUDE_DIRS)

mark_as_advanced(FIREFLY_INCLUDE_DIR FIREFLY_LIBRARY)
