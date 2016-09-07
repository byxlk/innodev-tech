#ifndef COMMON_H
#define COMMON_H

#define DEBUG_FLAG 1
#define ERROR_FLAG 1



#if DEBUG_FLAG
#define _DEBUG(msg...) do{ printf("[DEBUG][%s: %d] ",__FUNCTION__,__LINE__); \
        printf(msg); \
        printf("\r\n"); \
}while(0);
#else
#define _DEBUG(msg...)
#endif

#if ERROR_FLAG
#define _ERROR(msg...) do{ printf("[ERROR][%s: %d] ",__FUNCTION__,__LINE__); \
        printf(msg); \
        printf("\r\n"); \
}while(0);
#else
#define _ERROR(msg...)
#endif


#endif

