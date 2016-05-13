#include "backend.h"
#include "connection.h"

int backclient_read_head(zcAsynIO *a, const char *data, int len)
{
    int headlen = 0;
   
    memcpy(&headlen, data, 4);
    headlen = htob32(headlen);

    zc_asynio_read_bytes(a, headlen, thriftconn_read_body);
    
    return ZC_OK;
}


int backclient_read_body(zcAsynIO *a, const char *data, int len)
{
    BackData *b = (BackData*)a->data;
    backconn_send(b->conn, data, len);

    return ZC_OK;
} 


BackendConn*	
backconn_new(BackendConf *backend, struct ev_loop *loop, zcAsynIO *fromconn)
{
    BackendConn *bconn = zc_calloct(BackendConn);
    bconn->bconf = backend;

    BackData *data = zc_calloct(BackData);
    data->conn = fromconn;

    //bconn->client = zc_socket_new_client_tcp(backend->ip, backend->port, backend->timeout);
    bconn->client = zc_asynio_new_tcp_client(backend->ip, 
                backend->port,
                backend->timeout,
                NULL,
                loop, 0, 0
                );

    bconn->client->data = (void*)data;

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
backconn_send(BackendConn *conn, const char *data, int len)
{
    return zc_thriftconn_send(conn->client, data, len);
}


BackendPool*    
backpool_new(GroupConf *pconf, BackendConf *bconf)
{
    BackendPool *p = zc_calloct(BackendPool);
    p->pool  = zc_list_new();
    p->pconf = pconf;
    p->bconf = bconf;

    return p;
}


void    
backpool_delete(void *x)
{
    BackendPool *p = (BackendPool*)x;

    zc_list_delete(p->pool);
    zc_free(p);
}

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

BackendInfo*    
backinfo_new()
{
    BackendInfo *info = zc_calloct(BackendInfo);

    info->server_map = zc_dict_new_full(1000, 0, zc_free_func, backpool_delete);
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



