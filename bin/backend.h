#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <zocle/zocle.h>

typedef struct backendconn_t
{
	Backend		*backend;
	zcSocket	*client;
}BackendConn;

typedef struct backendpool_t
{
	zcPool	*conn_pool;
}BackendPool;

BackendConn*	backconn_new(Backend *);
void			backconn_delete(void *);



#endif
