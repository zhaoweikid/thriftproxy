#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <zocle/zocle.h>
#include "config.h"
#include "policy.h"

typedef struct backendconn_t
{
	BackendConf	*bconf;
	//zcSocket	*client;
	zcAsynIO	*client;
}BackendConn;

// BackendPool => BackendConn
typedef struct backendpool_t
{
	zcList		*pool;
	BackendConn	*cur;
	BackendConf	*bconf;
	GroupConf	*pconf;
}BackendPool;


typedef struct backendgroup_t
{
	zcList		*pools;
	BackendPool	*cur;
}BackendGroup;

// method => backendinfo
typedef struct backendinfo_t
{
	zcDict	*server_map;  // server_name => BackendPool
	zcDict	*method_map;  // method_name => BackendGroup
}BackendInfo;

typedef struct runner_t
{
	BackendInfo	*binfo;
}Runner;

extern Runner *g_run;

BackendConn*	backconn_new(BackendConf *,  struct ev_loop *loop);
void			backconn_delete(void*);

BackendPool*	backpool_new(GroupConf *, BackendConf *);
void			backpool_delete(void*);

BackendGroup*	backgroup_new();
void			backgroup_delete(void*);
int				backgroup_add_pool(BackendGroup*, BackendPool *);

BackendInfo*	backinfo_new();
void			backinfo_delete(void*);

int				runner_create();

#endif
