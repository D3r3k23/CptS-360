#ifndef LOG_H
#define LOG_H

#ifdef EN_LOG // gcc -DEN_LOG to enable
    #include <stdio.h>
    // Forwards __VA_ARGS__ to printf
    #define LOG(...)                             \
        do {                                     \
            printf("[LOG] <%s> ", __FUNCTION__); \
            printf(__VA_ARGS__);                 \
            printf("\n");                        \
        } while (0)
#else
    #define LOG(...)
#endif

#endif // LOG_H
