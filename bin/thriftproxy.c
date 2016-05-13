#include <zocle/zocle.h>
#include <ev.h>
#include "config.h"
#include "connection.h"
#include "backend.h"

Runner *g_run;

typedef struct runner_t
{
	BackendInfo	*binfo;
}Runner;

int
runner_create()
{
    g_run = zc_calloct(Runner);
    g_run->binfo = backinfo_new();

    return ZC_OK;
}



int thriftproxy_connected(zcAsynIO *conn) 
{
    ZCINFO("connected!");
    zc_socket_linger(conn->sock, 1, 0); 
    zc_asynio_read_bytes(conn, 4, conn_read_head);

    return ZC_OK;
}


int serve()
{
    zcProtocol p;

    zc_protocol_init(&p);
    p.handle_connected = thriftproxy_connected;
    
    struct ev_loop *loop = ev_default_loop (0);

    ZCNOTICE("server started at %s:%d timeout=%d", g_conf->ip, g_conf->port, g_conf->timeout);
    zcAsynIO *conn = zc_asynio_new_tcp_server(g_conf->ip, g_conf->port, g_conf->timeout, 
            &p, loop, 16384, 16384);
    if (NULL == conn) {
        ZCERROR("server create error");
        return 0;
    }   

    ev_run (loop, 0); 
    ZCINFO("stopped");
    
    zc_asynio_delete(conn);

    return 0;
}


int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("usage: \n\tthriftproxy configfile\n\n");
        return 0;
    }

    char *filename = argv[1];
    config_load(filename);

    zc_log_new(g_conf->logfile, g_conf->loglevel);
    config_print();

    int ret = runner_create();
    if (ret < 0) {
        ZCNOTE("runner create error! %d", ret);
        return 0;
    }
    
    serve();

    return 0;
}
