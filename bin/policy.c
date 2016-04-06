#include "policy.h"


GroupConf* groupconf_new()
{
    GroupConf *p = zc_calloct(GroupConf);

    p->server = zc_list_new();
    p->server->del = zc_free_func;

    p->method = zc_list_new();
    p->method->del = zc_free_func;
    

    return p;
}

void groupconf_delete(void *x)
{
    GroupConf *p = (GroupConf*)x;

    zc_list_delete(p->server);
    zc_list_delete(p->method);
    
    zc_free(p);
}



