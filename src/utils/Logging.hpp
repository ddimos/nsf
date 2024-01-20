#pragma once
#include <iostream>
#include <string>

// TODO to improve this
#define NSF_LOG_LEVEL 2

// TODO add categories

#if (NSF_LOG_LEVEL >= 1)
    #define NSF_LOG_ERROR(mes) std::cout << "[ERROR] " << mes << "\n";
#else
    #define NSF_LOG_ERROR(mes) (void(0))
#endif

#if (NSF_LOG_LEVEL >= 2)
    #define NSF_LOG(mes) std::cout << mes << "\n";
#else
    #define NSF_LOG(mes) (void(0))
#endif

#if (NSF_LOG_LEVEL >= 3)
    #define NSF_LOG_DEBUG(mes) std::cout << mes << "\n";
#else
    #define NSF_LOG_DEBUG(mes) (void(0))
#endif

#define NSF_TO_STR(x) std::to_string(x)
