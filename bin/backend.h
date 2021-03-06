#pragma once

#include <zocle/zocle.h>
#include "config.h"
#include "policy.h"

typedef struct backendconn_t
{
	BackendConf	*bconf;
	zcAsynIO	*client;
}BackendConn;

// BackendPool => BackendConn
typedef struct backendpool_t
{
	zcList		*conn_idle;
	zcList		*conn_use;
	BackendConf	*bconf;
	GroupConf	*pconf;
}BackendPool;

// BackendGroup => BackendPool
typedef struct backendgroup_t
{
	zcList		*pools;
    BackendPool *cur;
}BackendGroup;

// method => backendinfo
typedef struct backendinfo_t
{
	zcDict	*server_map;  // server_name => BackendPool
	zcDict	*method_map;  // method_name => BackendGroup
}BackendInfo;

typedef struct runner_t {
    BackendInfo *binfo;
}Runner;

extern Runner *g_run;


typedef struct session
{
    zcAsynIO    *front_conn;
    BackendConn *back_conn;
    BackendPool *back_pool;
}Session;


BackendConn*	backconn_new(BackendConf *, struct ev_loop *loop);
void			backconn_delete(void*);
int             backconn_send(BackendConn *conn, const char *data, int len, Session *s);

BackendPool*	backpool_new(GroupConf *, BackendConf *);
void			backpool_delete(void*);

BackendGroup*	backgroup_new();
void			backgroup_delete(void*);
int				backgroup_add_pool(BackendGroup*, BackendPool *);

BackendInfo*	backinfo_new();
void			backinfo_delete(void*);
BackendConn*    backinfo_get_backend_conn(BackendInfo *, char *name, BackendPool **);
void            backinfo_put_backend_conn(BackendInfo *, BackendPool *, BackendConn *);

