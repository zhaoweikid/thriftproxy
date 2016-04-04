#include "connection.h"
#include "endian_swap.h"

int conn_read_head(zcAsynIO *a, const char *data, int len)
{
    int headlen = 0;

    int i;
    for (i=0; i<len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
    
    memcpy(&headlen, data, 4);

    headlen = htob32(headlen);
    ZCDEBUG("head len:%d", headlen);

    zc_asynio_read_bytes(a, headlen, conn_read_body);
    
    return ZC_OK;
}


int conn_read_body(zcAsynIO *a, const char *data, int len)
{
    int i;
    for (i=0; i<len; i++) {
        unsigned char c = data[i];
        printf("%02x ", data[i]);
    }
    printf("\n");

    int namelen = 0;

    memcpy(&namelen, data+4, 4);
    namelen = htob32(namelen);
    ZCDEBUG("namelen:%d", namelen);

    char name[64] = {0};
    strncpy(name, data+8, namelen);

    ZCDEBUG("name:%s", name); 
    
    return ZC_OK;
}
 
