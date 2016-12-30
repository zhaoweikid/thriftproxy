#include "endian_swap.h"
#include "backend.h"
#include "config.h"

int frontconn_delete(zcAsynIO *conn)
{
    zc_mem_print();
    ZCDEBUG("front conn delete");
    if (conn->data) {
        Session   *s = (Session*)conn->data;
        ZCDEBUG("pub backend conn, pool:%p, conn:%p", s->back_pool, s->back_conn);
        if (s->back_pool) {
            backinfo_put_backend_conn(g_run->binfo, s->back_pool, s->back_conn);
        }
        zc_free(conn->data);
    }
    return ZC_OK;
}

int frontconn_read_body(zcAsynIO *conn, const char *data, int len)
{
    int namelen = 0;

    ZCDEBUG("read body: %d", len);
    print_hex(data, len, 1);

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
    ZCDEBUG("back conn:%p back pool:%p", bconn, pool);

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
    
    backconn_send(bconn, conn->rbuf->data, conn->rbuf->pos+len, fdata);
    zc_buffer_clear(conn->rbuf);

    zc_mem_print();

    return ZC_OK;
}

int frontconn_send(zcAsynIO *conn, const char *data, int len, Session *s)
{
    ZCDEBUG("front to client: %d, rbuf:%p, wbuf:%p", len, conn->rbuf, conn->wbuf);
    print_hex(data, len, 1);

    zc_buffer_append(conn->wbuf, (char*)data, len);
    zc_asynio_write_start(conn);
    
    backinfo_put_backend_conn(g_run->binfo, s->back_pool, s->back_conn);

    s->back_conn = NULL;
    s->back_pool = NULL;
    
    zc_mem_print();

    return ZC_OK;
}


int frontconn_read_head(zcAsynIO *conn, const char *data, int len)
{
    int headlen = 0;
    
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    ZCDEBUG("read head: %d %d", len, headlen); 
    print_hex(data, len, 1);


    zc_asynio_read_bytes(conn, headlen, frontconn_read_body);
    conn->rbuf->pos += 4;
    
    return ZC_OK;
}

int frontconn_connected(zcAsynIO *conn) 
{
    ZCINFO("connected!");
    zc_socket_linger(conn->sock, 1, 0); 
    zc_asynio_read_bytes(conn, 4, frontconn_read_head);
    conn->rbuf_auto_compact = 0;
    conn->p.handle_close = frontconn_delete;
 

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









