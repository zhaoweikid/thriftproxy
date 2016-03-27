#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

typedef struct config_t
{
	char	ip[16];
	int		port;
	int		timeout;
	char	logfile[256];
	int		loglevel;
	uint8_t daemon;
	uint8_t data_flush_mode;
	char	datafile[256];
	zcDict  *server;
	zcDict  *group;
	zcDict	*method;
}Config;

extern Config *g_conf;

Config*	config_load(char *filename);
void	config_print();

#endif
