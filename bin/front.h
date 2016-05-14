#pragma once

#include <stdio.h>
#include <zocle/zocle.h>

/*typedef struct connpair_t
{
	BackendConn	*backconn;
	FrontConn	*frontconn;
	int			(*callback)(zcAsynIO *);
}ConnPair;*/

int	 frontconn_connected(zcAsynIO *conn);
int	 frontconn_read_head(zcAsynIO *conn, const char *data, int len);
int	 frontconn_read_body(zcAsynIO *conn, const char *data, int len);
//int	 frontconn_send(zcAsynIO *conn, const char *data, int len);



