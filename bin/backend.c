#include "backend.h"


BackendConn*	
backconn_new(Backend *backend)
{
    BackendConn *bconn = zc_calloct(BackendConn);
    bconn->backend = backend;

    return bconn;
}


void
backconn_delete(void *)
{
    zc_free(x);
}



