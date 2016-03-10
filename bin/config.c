#include "config.h"
#include <zocle/zocle.h>

Config *g_conf;

Config* config_load(char *filename)
{
    ZCINFO("load config file: %s\n", filename);
    g_conf = zc_calloct(Config);

    strcpy(g_conf->ip, "127.0.0.1");
    g_conf->port = 10022;
    g_conf->timeout = 1000;

    


    return g_conf;
}
