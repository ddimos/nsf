#pragma once

#ifdef NSF_DISABLE_ASSERT
#    undef NSF_ASSERT
#    define NSF_ASSERT(condition, message) (void(0))
#elif !defined NSF_ASSERT
#    include <cassert>
#    define NSF_ASSERT(condition, message) assert(condition && message)
#endif
