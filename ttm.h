
#ifndef _H_TTM_
#define _H_TTM_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "ib2api.h"
#include "ib2type.h"
#include "ib2limits.h"
#include "businessdb.h"
#include "hzb_log.h"
#include "puberr.h"

#define	BUFFER_NUM	20

typedef struct {
	char	service_name[50];		/* 服务名称 */
	char	so_name[50];			/* 动态链接库名称  */
	char	service_func[50];		/* 入口函数 */
	char	arguments[200];			/* 参数 */
/* 触发服务参数 */
	char	trigger_interval[10];		/* 触发间隔  WEEK 或者 DAY 或者 空 */
	char	trigger_interval_value[10];	/* 触发间隔值 */
	char	trigger_type[10];		/* 触发类型 CLOCK 或者 TIME 或者 空 */
	char	trigger_start_time[100];	/* 触发开始时间 */
	char	trigger_end_time[100];		/* 触发结束时间 */
	char	trigger_date[BUFFER_NUM][20];	/* 触发日期 */
	char	time_buff[BUFFER_NUM][20];	/* 缓存触发时刻 */
/* 以上为触发服务参数 */
	char	cmd[20];			/* 命令行 */
	int	service_number;			/* 服务个数 */
	int	interval;			/* 服务间隔 */
	int	timeout;			/* 超时时间 */
	int	work_time;			/* 已运行时间 */
	int	wait_time;			/* 已等待时间 */
	char	serv_start_time[20];		/* 服务开始时间 */
	int	status;				/* 状态 */
	int	result;				/* 结果 */
	int	cur_cnt;			/* 运行次数 */
	int	succ_cnt;			/* 成功次数 */
	int	max_cnt;			/* 执行最大次数 */
	double	spend_time;			/* 处理时间 */
} stDefproccfg;

typedef struct {
	int	service_index;			/* 服务序号 */
	pid_t	pid;				/* 子进程pid */
	char	start_date[20];			/* 启动时间 */
	int	command_pipes[2] ;
	int	response_pipes[2] ;
	stDefproccfg procfg; 
} stDefprocinfo;

typedef struct {
	char	service_name[50];		/* 服务名称 */
	char	so_name[50];			/* 动态链接库名称  */
	char	service_func[50];		/* 入口函数 */
	char	arguments[200];			/* 参数 */
/* 触发服务参数 */
	char	trigger_interval[10];		/* 触发间隔 */
	char	trigger_interval_value[10];	/* 触发间隔值 */
	char	trigger_type[10];		/* 触发类型 */
	char	trigger_start_time[100];	/* 触发开始时间 */
	char	trigger_end_time[100];		/* 触发结束时间 */
/* 以上为触发服务参数 */
	char	cmd[20];			/* 命令行 */
	char	hostname[30];			/* 运行主机名称 */
	int	service_number;			/* 服务个数 */
	int	interval;			/* 服务间隔 */
	int	timeout;			/* 超时时间 */
	int	max_cnt;			/* 执行最大次数 */
} stDefsercfg;


#define	BUFFER_SIZE				2048000
typedef struct {
	char	buffer_len[20];			/* 缓冲区长度 */
	char	buffer[BUFFER_SIZE];
} stDefbuf;



#define MAX_PROCESSES				2000
#define SERVICE_STATUS_IDLE			0
#define SERVICE_STATUS_WORK			1
#define	BUFFER_LEN				4096
#define TIMEOUT					30
#define RESULT_END				999

#define SERVICE_CMD_VIEW	"VIEW"
#define SERVICE_CMD_ADD		"ADD"
#define SERVICE_CMD_RESTART	"RESTART"
#define SERVICE_CMD_CLOSE	"CLOSE"
#define SERVICE_CMD_FORCE	"FORCE"
#define SERVICE_CMD_STOP	"STOP"
#define SERVICE_CMD_RELOAD	"RELOAD"
#define SERVICE_CMD_KILL	"KILL"
#define SERVICE_CMD_PIPE	"PIPE"

#define	TRIGGER_INTERVAL_DAY	"DAY"
#define	TRIGGER_INTERVAL_WEEK	"WEEK"

#define	TRIGGER_TYPE_CLOCK	"CLOCK"
#define	TRIGGER_TYPE_TIME	"TIME"

int start_process( int argc, char *argv[] );
int stop_process( int argc, char *argv[] );
int force_stop_process( int argc, char *argv[] );
int view_process( int argc, char *argv[] );
int add_process( int argc, char *argv[] );
int mod_process( int argc, char *argv[] );
int view_service( stDefsercfg *pstsercfg, int sock );
int add_service( stDefsercfg *pstsercfg, int sock );
int stop_service( stDefsercfg *pstsercfg, int sock );
int force_stop_service( stDefsercfg *pstsercfg, int sock );
int mod_service( stDefsercfg *pstsercfg, int sock );
void trimSpace( char *pcData );
int GetCurrentSystemDate( char *szCurDate );

#endif





