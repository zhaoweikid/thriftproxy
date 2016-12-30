#pragma once

#include <stdio.h>
#include <zocle/zocle.h>
#include "backend.h"

int  frontconn_delete(zcAsynIO *conn);
int	 frontconn_connected(zcAsynIO *conn);
int	 frontconn_read_head(zcAsynIO *conn, const char *data, int len);
int	 frontconn_read_body(zcAsynIO *conn, const char *data, int len);
int  frontconn_send(zcAsynIO *conn, const char *data, int len, Session *s);

