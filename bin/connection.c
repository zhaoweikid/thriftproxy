#include "connection.h"
#include "endian_swap.h"
#include "backend.h"


BackendConn*
find_backend_conn(char *name)
{
    BackendInfo *binfo = g_run->binfo;

    BackendGroup *bg = zc_dict_get_str(binfo->method_map, name, NULL);
    if (NULL == bg) {
        ZCDEBUG("not found backendgroup with name:%s", name);
        return ZC_ERR;
    }

    ZCDEBUG("backgroup %s pool size:%d", name, bg->pools->size);
    if (bg->pools->size == 0) {
        ZCWARN("backgounp error!");     
        return ZC_ERR;
    }

    if (bg->cur == NULL) {
        bg->cur = zc_list_at(bg->pools, 0, NULL);
        if (bg->cur == NULL) {
            ZCDEBUG("backgroup cur error!");
            return ZC_ERR;
        }
    }

    BackendPool *p = bg->cur;

    BackendConn *conn;
    if (p->pool->size == 0) {
        struct ev_loop *loop = ev_default_loop (0);
        conn = backconn_new(p->bconf, loop);
    }else{
        conn = zc_list_at(p->pool, 0, NULL);
    }
    
    return conn;    
}

int conn_read_head(zcAsynIO *a, const char *data, int len)
{
    int headlen = 0;
   
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    zc_asynio_read_bytes(a, headlen, conn_read_body);
    
    return ZC_OK;
}


int conn_read_body(zcAsynIO *a, const char *data, int len)
{
    int namelen = 0;

    memcpy(&namelen, data+4, 4);
    namelen = htob32(namelen);
    ZCDEBUG("namelen:%d", namelen);

    char name[64] = {0};
    strncpy(name, data+8, namelen);

    ZCDEBUG("name:%s", name); 
    
    BackendConn *conn = find_backend_conn(name);
    backconn_send(conn, data, len, a);
    
    return ZC_OK;
}

int thriftconn_read_head(zcAsynIO *a, const char *data, int len)
{
    int headlen = 0;
   
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    zc_asynio_read_bytes(a, headlen, thriftconn_read_body);
    
    return ZC_OK;
}


int thriftconn_read_body(zcAsynIO *a, const char *data, int len)
{
    BackData *b = (BackData*)a->data;

    backconn_send(b->conn, data, len);

    return ZC_OK;
} 

zcAsynIO*	
zc_thriftconn_new_client(const char *host, int port, int timeout, 
				struct ev_loop *loop, BackData *data)
{
    zcAsynIO *conn = zc_asynio_new_tcp_client(host, port, timeout, NULL, loop, 0, 0);
    conn->data = (void*)data;

    return conn;
}

void
zc_thriftconn_delete(void *x)
{
    zcAsynIO *conn = (zcAsynIO*)x;
    
    //zc_free(conn->data);
    zc_asynio_delete(conn); 
}

int 
zc_thriftconn_send(zcAsynIO *conn, const char *data, int len)
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










