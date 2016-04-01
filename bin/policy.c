#include "policy.h"


Group* group_new()
{
    Group *p = zc_calloct(Group);

    p->server = zc_list_new();
    p->server->del = zc_free_func;

    p->method = zc_list_new();
    p->method->del = zc_free_func;
    

    return p;
}

void group_delete(void *x)
{
    Group *p = (Group*)x;

    zc_list_delete(p->server);
    zc_list_delete(p->method);
    
    zc_free(p);
}



