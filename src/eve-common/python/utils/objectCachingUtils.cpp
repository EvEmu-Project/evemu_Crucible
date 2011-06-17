#include "EVECommonPCH.h"
#include "python/utils/objectCachingUtils.h"

/**
 * this file should implement the following python classes:
 * CachedObject
 * CachedMethodCallResult
 *
 * the classes should be implemented the same way CacheOK is implemented
 */

CacheOK::CacheOK() : PyObjectEx_Type1( PyToken( "objectCaching.CacheOK" ), new_tuple( "CacheOK" ), NULL ) {}