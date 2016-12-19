#include "endian_swap.h"
#include "backend.h"
#include "config.h"

int frontconn_delete(zcAsynIO *conn)
{
    if (conn->data) {
        Session   *fdata = (Session*)conn->data;
        backinfo_put_backend_conn(g_run->binfo, fdata->back_pool, fdata->back_conn);
        zc_free(conn->data);
    }
    return ZC_OK;
}

int frontconn_read_body(zcAsynIO *conn, const char *data, int len)
{
    int namelen = 0;

    memcpy(&namelen, data+4, 4);
    namelen = htob32(namelen);
    ZCDEBUG("namelen:%d", namelen);

    char name[256] = {0};
    strncpy(name, data+8, namelen);

    ZCDEBUG("name:%s", name); 
    
    BackendPool *pool;
    BackendConn *bconn = backinfo_get_backend_conn(g_run->binfo, name, &pool);
    if (NULL == bconn) {
        ZCWARN("no backend by name:%s", name);
        zc_asynio_delete_delay(conn);
        return ZC_OK;
    }

    Session *fdata;
    if (conn->data) {
        fdata = (Session*)conn->data;
        fdata->back_conn = bconn;
        fdata->back_pool = pool;
    }else{
        fdata = zc_calloct(Session);
        fdata->front_conn = conn;
        fdata->back_conn  = bconn;
        fdata->back_pool = pool;
        conn->data = fdata;
    }
    
    backconn_send(bconn, conn->rbuf->data, conn->rbuf->pos, fdata);

    return ZC_OK;
}


int frontconn_read_head(zcAsynIO *conn, const char *data, int len)
{
    int headlen = 0;
   
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    zc_asynio_read_bytes(conn, headlen, frontconn_read_body);
    
    return ZC_OK;
}

int frontconn_connected(zcAsynIO *conn) 
{
    ZCINFO("connected!");
    zc_socket_linger(conn->sock, 1, 0); 
    zc_asynio_read_bytes(conn, 4, frontconn_read_head);

    return ZC_OK;
}




/*int 
frontconn_send(zcAsynIO *conn, const char *data, int len)
{
    char head[16] = {0};
    int newlen = b32toh(len); 
    memcpy(head, &newlen, sizeof(int));

    zc_buffer_append(conn->wbuf, (void*)head, 4); 
    zc_buffer_append(conn->wbuf, (void*)data, len);
        
    zc_asynio_write_start(conn->client);
    
    zc_asynio_read_bytes(conn, 4, thriftconn_read_head);
    
    return ZC_OK; 
}
*/









