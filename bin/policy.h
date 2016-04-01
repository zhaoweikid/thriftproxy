#ifndef __POLICY_H__
#define __POLICY_H__

#include <stdio.h>
#include <stdint.h>
#include <zocle/zocle.h>

// 同步模式, 必须同步等待结果返回
#define MODE_SYNC		1
// 异步模式, 不等待结果返回, 仅放入自己缓冲区
#define MODE_ASYNC		2

// 轮询
#define POLICY_RR		1
// 随机
#define POLICY_RAND		3	
// 带权重随机
#define POLICY_RAND_W	4	

typedef struct backend_t
{
	char	name[128];
	char	ip[16];
	int		port;
	int		timeout;
	int		weight;
	int		max_conn; 
}Backend;

typedef struct group_t
{
    char        name[128];
	uint16_t	policy;
	uint16_t	long_conn;
	uint16_t	mode;
	uint16_t	copy_num;
	int			server_weight; // weight sum
	zcList		*server; // all server
	//Backend		*cur; // 
    zcList      *method; // all method
}Group;

Group*	group_new();
void	group_delete(void*);

typedef struct backendinfo_t
{
	Backend		*server;

}BackendInfo;

typedef struct policy_t
{
	BackendInfo	*server_info;
	Backend		*cur;   // current backend server, for server select
}Policy;



#endif