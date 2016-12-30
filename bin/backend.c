#include "backend.h"
#include "endian_swap.h"
#include "front.h"

int backclient_read_body(zcAsynIO *c, const char *data, int len)
{
    Session *s = (Session*)c->data;

    frontconn_send(s->front_conn, (char*)c->rbuf->data, c->rbuf->pos+len, s);
    zc_buffer_clear(c->rbuf);

    return ZC_OK;
}

int backclient_read_head(zcAsynIO *c, const char *data, int len)
{
    int headlen = 0;
   
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    zc_asynio_read_bytes(c, headlen, backclient_read_body);
    c->rbuf->pos += 4;
    
    return ZC_OK;
}



BackendConn*	
backconn_new(BackendConf *backend, struct ev_loop *loop)
{
    BackendConn *bconn = zc_calloct(BackendConn);
    bconn->bconf = backend;

    zcAsynIO *c = bconn->client;
    c = zc_asynio_new_tcp_client(backend->ip, backend->port, backend->timeout,
                NULL, loop, 16384, 16384);
    c->rbuf_auto_compact = 0;
    zc_asynio_read_bytes(c, 4, backclient_read_head);
    bconn->client = c;

    return bconn;
}


void
backconn_delete(void *x)
{
    BackendConn *conn = (BackendConn*)x;
    zc_asynio_delete(conn->client);
    zc_free(x);
}

int
backconn_send(BackendConn *conn, const char *data, int len, Session *s)
{
    zc_buffer_append(conn->client->wbuf, (char*)data, len);
    conn->client->data = (void*)s;
    zc_asynio_write_start(conn->client);
    return ZC_OK;
}

// --------------------------------------
BackendPool*    
backpool_new(GroupConf *pconf, BackendConf *bconf)
{
    BackendPool *p = zc_calloct(BackendPool);
    p->conn_idle = zc_list_new();
    p->conn_idle->del = backconn_delete;

    p->conn_use = zc_list_new();
    p->conn_use->del = backconn_delete;

    p->pconf = pconf;
    p->bconf = bconf;

    return p;
}


void    
backpool_delete(void *x)
{
    BackendPool *p = (BackendPool*)x;

    zc_list_delete(p->conn_idle);
    zc_list_delete(p->conn_use);
    zc_free(p);
}

// --------------------------------------
BackendGroup*   
backgroup_new()
{
    BackendGroup *bgroup = zc_calloct(BackendGroup);
    bgroup->pools = zc_list_new();
    return bgroup;
}

void
backgroup_delete(void *x)
{
    BackendGroup *bgroup = (BackendGroup*)x;
    zc_list_delete(bgroup->pools);
    zc_free(bgroup);
}

int 
backgroup_add_pool(BackendGroup *bgroup, BackendPool *p)
{
    zc_list_append(bgroup->pools, p);
    return ZC_OK;
}

// --------------------------------------
BackendInfo*    
backinfo_new()
{
    BackendInfo *info = zc_calloct(BackendInfo);

    // server => backpool
    info->server_map = zc_dict_new_full(1000, 0, zc_free_func, backpool_delete);
    // method => backgroup 
    info->method_map = zc_dict_new_full(10000, 0, zc_free_func, backgroup_delete);

    char        *key;
    BackendConf *bconf;
    GroupConf   *pconf;
    BackendPool *bpool;

    BackendGroup *bgroup;
    zcListNode   *thenode;
    zc_dict_foreach_start(g_conf->group, key, pconf)
        ZCDEBUG("group key: |%s|", key);
        bgroup = backgroup_new();
        zc_list_foreach(pconf->server, thenode) { 
            char *server_name = (char*)thenode->data;
            //ZCDEBUG("server name: %s", server_name);
            bpool = zc_dict_get_str(info->server_map, server_name, NULL);
            if (bpool == NULL) {
                bconf = zc_dict_get_str(g_conf->server, server_name, NULL);
                if (NULL == bconf) {
                    ZCWARN("not found server in g_conf->servers: %s", server_name);
                    goto backinfo_error;
                }
                ZCDEBUG("add backendpool |%s| to server_map", server_name);
                bpool = backpool_new(pconf, bconf);
                zc_dict_add_str(info->server_map, server_name, bpool);
            }
            ZCDEBUG("add backendpool to backendgroup %p", bgroup);
            backgroup_add_pool(bgroup, bpool);
        }
       
        zc_list_foreach(pconf->method, thenode) { 
            zcString *method_name = (zcString*)thenode->data;
            ZCDEBUG("add backgroup %p with method |%s| to method_map", bgroup, method_name->data);
            zc_dict_add_str(info->method_map, method_name->data, bgroup);
        }

    zc_dict_foreach_end

    return info;

backinfo_error:
    backinfo_delete(info);
    return NULL;

}

void
backinfo_delete(void *x)
{
    BackendInfo *info = (BackendInfo*)x;
    zc_dict_delete(info->server_map);
    zc_dict_delete(info->method_map);
    zc_free(info);
}

BackendConn*
backinfo_get_backend_conn(BackendInfo *binfo, char *name, BackendPool **bpool)
{
    // method_name => group => server conn pool
    //BackendInfo *binfo = g_run->binfo;

    BackendGroup *bg = zc_dict_get_str(binfo->method_map, name, NULL);
    if (NULL == bg) {
        ZCDEBUG("not found backendgroup with name:%s", name);
        return NULL;
    }

    ZCDEBUG("backgroup %s pool size:%d", name, bg->pools->size);
    if (bg->pools->size == 0) {
        ZCWARN("backgounp error!");     
        return NULL;
    }

    if (bg->cur == NULL) {
        bg->cur = zc_list_at(bg->pools, 0, NULL);
        if (bg->cur == NULL) {
            ZCDEBUG("backgroup cur error!");
            return NULL;
        }
    }

    BackendPool *p = bg->cur;

    BackendConn *conn;
    if (p->conn_idle->size == 0) {
        struct ev_loop *loop = ev_default_loop (0);
        conn = backconn_new(p->bconf, loop);
    }else{
        conn = zc_list_pop(p->conn_idle, 0, NULL);
    }
    zc_list_append(p->conn_use, conn);

    ZCDEBUG("get bpool:%p, pool:%p conn:%p", bpool, p, conn);
    if (bpool) 
        *bpool = p;
    
    return conn;    
}

void
backinfo_put_backend_conn(BackendInfo *binfo, BackendPool *pool, BackendConn *conn)
{
    int ret = zc_list_remove(pool->conn_use, conn);
    if (ret != ZC_OK) {
        ZCWARN("remove conn %p error", conn);
        return;
    }
    zc_list_prepend(pool->conn_idle, conn);
}


