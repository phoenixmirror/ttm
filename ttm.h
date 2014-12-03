
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
	char	service_name[50];		/* �������� */
	char	so_name[50];			/* ��̬���ӿ�����  */
	char	service_func[50];		/* ��ں��� */
	char	arguments[200];			/* ���� */
/* ����������� */
	char	trigger_interval[10];		/* �������  WEEK ���� DAY ���� �� */
	char	trigger_interval_value[10];	/* �������ֵ */
	char	trigger_type[10];		/* �������� CLOCK ���� TIME ���� �� */
	char	trigger_start_time[100];	/* ������ʼʱ�� */
	char	trigger_end_time[100];		/* ��������ʱ�� */
	char	trigger_date[BUFFER_NUM][20];	/* �������� */
	char	time_buff[BUFFER_NUM][20];	/* ���津��ʱ�� */
/* ����Ϊ����������� */
	char	cmd[20];			/* ������ */
	int	service_number;			/* ������� */
	int	interval;			/* ������ */
	int	timeout;			/* ��ʱʱ�� */
	int	work_time;			/* ������ʱ�� */
	int	wait_time;			/* �ѵȴ�ʱ�� */
	char	serv_start_time[20];		/* ����ʼʱ�� */
	int	status;				/* ״̬ */
	int	result;				/* ��� */
	int	cur_cnt;			/* ���д��� */
	int	succ_cnt;			/* �ɹ����� */
	int	max_cnt;			/* ִ�������� */
	double	spend_time;			/* ����ʱ�� */
} stDefproccfg;

typedef struct {
	int	service_index;			/* ������� */
	pid_t	pid;				/* �ӽ���pid */
	char	start_date[20];			/* ����ʱ�� */
	int	command_pipes[2] ;
	int	response_pipes[2] ;
	stDefproccfg procfg; 
} stDefprocinfo;

typedef struct {
	char	service_name[50];		/* �������� */
	char	so_name[50];			/* ��̬���ӿ�����  */
	char	service_func[50];		/* ��ں��� */
	char	arguments[200];			/* ���� */
/* ����������� */
	char	trigger_interval[10];		/* ������� */
	char	trigger_interval_value[10];	/* �������ֵ */
	char	trigger_type[10];		/* �������� */
	char	trigger_start_time[100];	/* ������ʼʱ�� */
	char	trigger_end_time[100];		/* ��������ʱ�� */
/* ����Ϊ����������� */
	char	cmd[20];			/* ������ */
	char	hostname[30];			/* ������������ */
	int	service_number;			/* ������� */
	int	interval;			/* ������ */
	int	timeout;			/* ��ʱʱ�� */
	int	max_cnt;			/* ִ�������� */
} stDefsercfg;


#define	BUFFER_SIZE				2048000
typedef struct {
	char	buffer_len[20];			/* ���������� */
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





