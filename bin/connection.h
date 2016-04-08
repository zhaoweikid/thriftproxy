#pragma once

#include <stdio.h>
#include <zocle/zocle.h>

int conn_read_head(zcAsynIO *a, const char *data, int len);
int conn_read_body(zcAsynIO *a, const char *data, int len);

