#include "connection.h"


int conn_read_head(zcAsynIO *a, const char *data, int len)
{
    int headlen = 0;
    
    memcpy(&headlen, data, 4);

    ZCDEBUG("head len:%d", headlen);

    zc_asynio_read_bytes(a, headlen, conn_read_body);
    
    return ZC_OK;
}


int conn_read_body(zcAsynIO *a, const char *data, int len)
{
    /*int i;
    char buffer[1024] = {0};
    int bi = 0;
    for (i=0; i<len; i++) {
        unsigned char c = data[i];
        buffer[bi] = data[i];
    }*/
    
    return ZC_OK;
}
 
