#ifndef REX_ENGINE_GLOBAL_H
#define REX_ENGINE_GLOBAL_H

#ifdef _WINDOWS
    #if REX_DYNAMIC_LINK
        #ifdef REX_ENGINE_LIB
            # define REX_ENGINE_EXPORT __declspec(dllexport)
        #else
            # define REX_ENGINE_EXPORT __declspec(dllimport)
        #endif
    #else
        #define REX_ENGINE_EXPORT
    #endif
#else
    #error We only support windows
#endif

#endif // REX_ENGINE_GLOBAL_H
