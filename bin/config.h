#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>

typedef struct config
{
	char	ip[16];
	int		port;
	int		timeout;
	char	logfile[256];
	int		loglevel;

}Config;

extern Config *g_conf;

Config*	config_load(char *filename);

#endif
