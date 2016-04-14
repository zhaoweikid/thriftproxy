#pragma once

#include <stdio.h>
#include <zocle/zocle.h>

int conn_read_head(zcAsynIO *a, const char *data, int len);
int conn_read_body(zcAsynIO *a, const char *data, int len);

typedef struct frontconn_t
{
	AsynIO	*client;
}FrontConn;

typedef struct thriftconn_t
{
	BackendConn	*conn;
	int			(*callback)(zcAsynIO *);
}BackData;

zcAsynIO*	zc_thriftconn_new_client(const char *host, int port, int timeout, 
				struct ev_loop *loop, BackData *data);
void		zc_thriftconn_delete(void*);
int			zc_thriftconn_send(zcAsynIO *conn, const char *data, int len);



