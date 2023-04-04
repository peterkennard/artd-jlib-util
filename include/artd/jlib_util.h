#ifndef __artd_jlib_util_h
#define __artd_jlib_util_h

#include "artd/jlib_base.h"
#include "artd/int_types.h"
#include "artd/Logger.h"

#ifdef BUILDING_artd_jlib_util
#define ARTD_API_JLIB_UTIL ARTD_SHARED_LIBRARY_EXPORT
#else
#define ARTD_API_JLIB_UTIL ARTD_SHARED_LIBRARY_IMPORT
#endif

#endif // __artd_jlib_util_h
