#include "config.h"
#include "policy.h"

Config *g_conf;

Config* config_load(char *filename)
{
    //ZCINFO("load config file: %s\n", filename);
    g_conf = zc_calloct(Config);

    strcpy(g_conf->ip, "127.0.0.1");
    g_conf->port = 11111;
    g_conf->timeout = 1000; // ms
    
    zcConfDict *cfdic = zc_confdict_new(filename);
    if (NULL == cfdic) {
        ZCFATAL("config file error: %s\n", filename);
    }

    int ret = zc_confdict_parse(cfdic);
    if (ret != 0) {
        ZCFATAL("config file %s parse error: %d\n", filename, ret);
    }
    
    //ZCDEBUG("====================\n"); 
    char *value = NULL; 

    value = zc_confdict_get_str(cfdic, "main", "ip", NULL);
    if (NULL == value) {
        ZCFATAL("main.ip parse error!");
    }
    snprintf(g_conf->ip, sizeof(g_conf->ip), "%s", value);
    g_conf->port = zc_confdict_get_int(cfdic, "main", "port", 0);
    if (g_conf->port == 0) {
        ZCFATAL("main.port parse error!");
    }
    
    g_conf->timeout = zc_confdict_get_int(cfdic, "main", "timeout", -1);
    if (g_conf->timeout == -1) {
        ZCFATAL("main.timeout parse error!");
    }

    g_conf->daemon = zc_confdict_get_int(cfdic, "main", "daemon", 0);
    if (g_conf->daemon != 0 && g_conf->daemon != 1) {
        ZCFATAL("main.daemon parse error!");
    }

    g_conf->data_flush_mode = zc_confdict_get_int(cfdic, "main", "data_flush_mode", -2);
    if (g_conf->data_flush_mode == -2) {
        ZCFATAL("main.data_flush_mode parse error!");
    }
    
    value = zc_confdict_get_str(cfdic, "main", "homedir", NULL);
    if (NULL == value) {
        ZCFATAL("main.homedir parse error!");
    }
    snprintf(g_conf->homedir, sizeof(g_conf->homedir), "%s", value);

    value = zc_confdict_get_str(cfdic, "main", "logfile", NULL);
    if (NULL == value) {
        ZCFATAL("main.logfile parse error!");
    }
    if (*value == '/') {
        snprintf(g_conf->logfile, sizeof(g_conf->logfile), "%s", value);
    }else if (strcmp(value, "stdout") != 0){
        snprintf(g_conf->logfile, sizeof(g_conf->logfile), "%s/%s", g_conf->homedir, value);
    }else{
        strcpy(g_conf->logfile, value);
    }

    value = zc_confdict_get_str(cfdic, "main", "loglevel", NULL);
    if (NULL == value) {
        ZCFATAL("main.loglevel parse error!");
    }

    if (strcmp(value, "INFO") == 0 || strcmp(value, "DEBUG") == 0) {
        g_conf->loglevel = ZC_LOG_DEBUG;
    }else if (strcmp(value, "NOTICE") == 0) {
        g_conf->loglevel = ZC_LOG_NOTICE;
    }else if (strcmp(value, "WARN") == 0) {
        g_conf->loglevel = ZC_LOG_WARN;
    }else if (strcmp(value, "ERROR") == 0) {
        g_conf->loglevel = ZC_LOG_ERROR;
    }else{
        ZCFATAL("loglevel error: %s", value);
    }

    value = zc_confdict_get_str(cfdic, "main", "datafile", NULL);
    if (NULL == value) {
        ZCFATAL("main.datafile parse error!");
    }
    if (*value == '/') {
        snprintf(g_conf->datafile, sizeof(g_conf->datafile), "%s", value);
    }else{
        snprintf(g_conf->datafile, sizeof(g_conf->datafile), "%s/%s", g_conf->homedir, value);
    }

    g_conf->server = zc_dict_new_full(10000, 0, zc_free_func, zc_free_func);
    // method -> group
    g_conf->method = zc_dict_new(10000); 
    g_conf->group  = zc_dict_new_full(10000, 0, zc_free_func, groupconf_delete);

    char *nodekey;
    zc_dict_foreach_keys_start(cfdic->groups, nodekey)
        if (strncmp(nodekey, "backend:", 8) == 0) {
            BackendConf *backend = zc_calloct(BackendConf);

            value = zc_confdict_get_str(cfdic, nodekey, "ip", NULL);
            if (NULL == value) {
                ZCFATAL("%s.ip error: %s", nodekey, value);
            }
            snprintf(backend->ip, sizeof(backend->ip), "%s", value);

            backend->port = zc_confdict_get_int(cfdic, nodekey, "port", 0);
            if (backend->port == 0) {
                ZCFATAL("%s.port error", nodekey);
            } 

            backend->timeout = zc_confdict_get_int(cfdic, nodekey, "timeout", -1);
            if (backend->timeout == -1) {
                ZCFATAL("%s.timeout error", nodekey);
            }

            backend->weight = zc_confdict_get_int(cfdic, nodekey, "weight", 1);
            backend->max_conn = zc_confdict_get_int(cfdic, nodekey, "max_conn", 3);
            
            value = nodekey;
            value += 8;  // skip backend:
            snprintf(backend->name, sizeof(backend->name), "%s", value);
            //ZCDEBUG("dict add server %s, %p\n", value, backend);
            zc_dict_add(g_conf->server, value, strlen(value), backend);
        }
    zc_dict_foreach_keys_end 

    zc_dict_foreach_keys_start(cfdic->groups, nodekey) 
        if (strncmp(nodekey, "group:", 6) == 0) {
            //Group* group  = zc_calloct(Group);
            //group->server = zc_list_new();
            //group->method = zc_list_new();

            GroupConf* group  = groupconf_new();
            value = nodekey;
            value += 6; // skip group:
            snprintf(group->name, sizeof(group->name), "%s", value);

            //ZCDEBUG("dict add group %s, %p\n", group->name, group);
            zc_dict_add(g_conf->group, group->name, 0, group);

            // parse policy
            value = zc_confdict_get_str(cfdic, nodekey, "policy", NULL);
            if (NULL == value) {
                ZCFATAL("%s.policy error", nodekey);
            }
            if (strcmp(value, "rr") == 0) {
                group->policy = POLICY_RR;
            }else if (strcmp(value, "rand") == 0) {
                group->policy = POLICY_RAND;
            }else{
                ZCFATAL("%s.policy error: %s", nodekey, value);
            }

            group->long_conn = zc_confdict_get_int(cfdic, nodekey, "long_connection", 0);
            group->copy_num = zc_confdict_get_int(cfdic, nodekey, "copy_num", 1);
            value = zc_confdict_get_str(cfdic, nodekey, "mode", NULL);
            if (NULL == value) {
                ZCFATAL("%s.mode error", nodekey);
            }
            if (strcmp(value, "sync") == 0) {
                group->mode = MODE_SYNC;
            }else if (strcmp(value, "async") == 0) {
                group->mode = MODE_ASYNC;
            }else{
                ZCFATAL("%s.mode error: %s", nodekey, value);
            }

            int i;
            value = zc_confdict_get_str(cfdic, nodekey, "server", NULL);
            if (NULL == value) {
                ZCFATAL("%s.server error", nodekey);
            }

            int vlen = strlen(value);
            for (i=0; i<vlen; i++) {
                if (!isalnum(value[i]) && !isblank(value[i]) && value[i] != ',') {
                    ZCFATAL("%s.server error, server name character must be alphabetic  or numeric or _ or .\n", nodekey);
                }
            }
            
            //ZCDEBUG("server: %s\n", value);
            char server_name[128];
            while (*value) {
                i = 0;
                while (isblank(*value)) value++;
                while (!isblank(*value) && *value != ',' && *value != 0) {
                    server_name[i] = *value;
                    i++;
                    value++;
                }
                //ZCDEBUG("server_name:%s\n", server_name);
                BackendConf *backend = zc_dict_get(g_conf->server, server_name, 0, NULL);
                if (NULL == backend) {
                    ZCFATAL("%s.server error: %s", nodekey, server_name);
                }
                zc_list_append(group->server, backend);
                while (isblank(*value) || *value == ',') value++;
            }

            value = zc_confdict_get_str(cfdic, nodekey, "method", NULL);
            if (NULL == value) {
                ZCFATAL("%s.method error", nodekey);
            }

            vlen = strlen(value);
            for (i=0; i<vlen; i++) {
                if (!isalnum(value[i]) && !isblank(value[i]) && value[i] != ',') {
                    ZCFATAL("%s.method error, server name character must be alphabetic  or numeric or _ or .", nodekey);
                }
            }

            //ZCDEBUG("method: %s", value);
            char method[128];
            while (*value) {
                i = 0;
                while (isblank(*value)) value++;
                while (!isblank(*value) && *value != ',' && *value != 0) {
                    method[i] = *value;
                    i++;
                    value++;
                }
                method[i] = 0;
                zc_list_append(group->method, zc_str_new_char(method, 0));
                //ZCDEBUG("dict add method %s, %p\n", method, group);
                zc_dict_add_str(g_conf->method, method, group);
                while (isblank(*value) || *value == ',') value++;
            }
             
        }
    zc_dict_foreach_keys_end 

    zc_confdict_delete(cfdic); 

    return g_conf;
}


void	
config_print()
{
    Config *cf = g_conf;

    ZCNOTE("main.ip     = %s", cf->ip);
    ZCNOTE("main.port   = %d", cf->port);
    ZCNOTE("main.timeout= %d", cf->timeout);
    ZCNOTE("main.daemon = %d", cf->daemon);
    ZCNOTE("main.homedir    = %s", cf->homedir);
    ZCNOTE("main.logfile    = %s", cf->logfile);
    ZCNOTE("main.loglevel   = %d", cf->loglevel);
    ZCNOTE("main.datafile   = %s", cf->datafile);
    ZCNOTE("main.data_flush_mode = %d", cf->data_flush_mode);

    char *nodekey;
    zc_dict_foreach_keys_start(cf->server, nodekey) 
        BackendConf *b = (BackendConf*)node->value;
        ZCNOTE("backend:%s.ip       = %s", nodekey, b->ip);
        ZCNOTE("backend:%s.port     = %d", nodekey, b->port);
        ZCNOTE("backend:%s.timeout  = %d", nodekey, b->timeout);
        ZCNOTE("backend:%s.weight   = %d", nodekey, b->weight);
        ZCNOTE("backend:%s.max_conn = %d", nodekey, b->max_conn);
    zc_dict_foreach_keys_end

    zc_dict_foreach_keys_start(cf->group, nodekey) 
        GroupConf *p = (GroupConf*)node->value;
        ZCNOTE("group:%s.policy     = %d", nodekey, p->policy);
        ZCNOTE("group:%s.long_conn  = %d", nodekey, p->long_conn);
        ZCNOTE("group:%s.mode       = %d", nodekey, p->mode);
        ZCNOTE("group:%s.copy_num   = %d", nodekey, p->copy_num);
        zcListNode *lnode;
        zc_list_foreach(p->server, lnode) {
            BackendConf *b = (BackendConf*)lnode->data;
            ZCNOTE("group:%s.server = %s", nodekey, b->name);
        }
        zc_list_foreach(p->method, lnode) {
            zcString *s = (zcString*)lnode->data;
            ZCNOTE("group:%s.method = %s", nodekey, s->data);
        }
    zc_dict_foreach_keys_end
}



void    
print_hex(const char *s, int len, int ln)
{
    int i;

    for (i=0; i<len; i++) {
        printf("%02X ", s[i]);
    }
    if (ln == 1)
        printf("\n");
}


