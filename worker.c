#include "ttm.h"

int             ( *pfuncBusinessProcess)( struct IB2Env *penv , char *param ) ;
OBJECTHANDLE    *phandle = NULL  ;
stDefproccfg	procfg ;

void UnloadPhandle();

int worker( int pipe_req, int pipe_rsp )
{

	int	rnet ;
	int	maxfd ;
	char	now_date[11] ;
	char	last_date[11] ;
	fd_set	fd_read ;
	struct	timeval tv ;
	struct	timeval start, end ;
	struct	IB2Env	*penv = NULL  ;	
	double	span ;

	memset( &procfg, 0x00, sizeof( procfg ) );
	/* 分配内存空间 */
	rnet = IB2AllocEnvironment( &penv ) ;
	if ( rnet != 0 )
        {
		hzb_log_error(__FILE__, __LINE__, "分配内存空间错误 IB2AllocEnvironment rnet=%d", rnet) ;
		exit(1);
        }
	hzb_log_info(__FILE__, __LINE__, "分配IB2内存空间成功" ) ; 

	/* 加载配置文件 */
	rnet = IB2LoadClientConfig( penv , NULL ) ;
	if ( rnet != 0 )
        {
		hzb_log_error(__FILE__, __LINE__, "加载配置文件错误 IB2LoadClientConfig rnet=%d", rnet) ;
		exit(2) ;
        }

	/* 打开数据库 */
	rnet = BusinessDataBaseOpen() ;
        if ( rnet )
        {
                hzb_log_error(__FILE__, __LINE__, "开启数据库失败, rnet=%d", rnet) ;
                exit(3) ;
        }	
	hzb_log_info(__FILE__, __LINE__, "打开数据库连接成功" ) ;

	
	/* 取得当前日期 */
	memset( last_date, 0x00, sizeof( last_date ) ) ;
	GetCurrentSystemDate( last_date );

	hzb_log_info(__FILE__, __LINE__, "系统日期[%s]", last_date ) ;	

	while( 1 )
	{
		FD_ZERO( &fd_read );			
		FD_SET( pipe_req, &fd_read );		
		maxfd = pipe_req ;
		
		tv.tv_sec = 1 ;
		tv.tv_usec = 0 ;
		
		rnet = select( maxfd+1, &fd_read, NULL, NULL, &tv ) ;
		if( rnet == 0 )
		{
			/* 数据库重连 */
			memset( now_date, 0x00, sizeof( now_date ) ) ;
			GetCurrentSystemDate( now_date ) ;				
			if ( strcmp( now_date, last_date ) > 0 )
			{
				/* 关闭数据库 */
				rnet = BusinessDataBaseClose() ;
				if( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__, "BusinessDataBaseClose error rnet=%d", rnet) ;
				}
				
				sleep(1);

				/* 打开数据库 */
				rnet = BusinessDataBaseOpen() ;
				if( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__, "BusinessDataBaseOpen error rnet=%d",rnet) ;

				}
				else
				{
					strcpy( last_date, now_date );
					hzb_log_info(__FILE__, __LINE__, "重新连接数据库成功" ) ;	
				}
			}
		}
		else if( rnet > 0 )
		{
			if( FD_ISSET( pipe_req, &fd_read ) )
			{
				memset( &procfg, 0x00, sizeof( procfg ) );
				rnet = read( pipe_req, &procfg, sizeof(procfg) );		
				if( rnet != sizeof(procfg) )
				{
					if( rnet == 0 )
						hzb_log_error(__FILE__, __LINE__,"管道已关闭");
					else
						hzb_log_error(__FILE__, __LINE__, "write err errno[%d][%s]",errno,strerror(errno) ) ;
					break;
				}

				if( !strcmp( procfg.cmd, SERVICE_CMD_STOP ) ||
			  	    !strcmp( procfg.cmd, SERVICE_CMD_RESTART ) ||
				    !strcmp( procfg.cmd, SERVICE_CMD_RELOAD ) )
				{
					hzb_log_info(__FILE__, __LINE__, "收到父进程[%s]请求",procfg.cmd ) ;
					procfg.result = RESULT_END ;
					rnet = write( pipe_rsp, &procfg, sizeof( procfg ) );
					if( rnet != sizeof(procfg) )
					{
						hzb_log_error(__FILE__, __LINE__, "写管道出错[%s]",strerror(errno)) ;
					}
					else
					{
						hzb_log_info(__FILE__, __LINE__,"发送回应result[%d]成功", procfg.result);
					}
					break ;
				}

				/* 挂接用户业务处理组件 */
				rnet = LoadUserBusinessRll();
				if ( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__,"挂接用户业务处理组件失败rnet=%d",rnet) ;
					goto END ;
				}

				gettimeofday( &start, NULL );
				hzb_log_info(__FILE__, __LINE__,"进入用户业务处理函数");
				
				/* 业务处理 */		
				rnet = pfuncBusinessProcess( penv , procfg.arguments );
				if( rnet && rnet != 4 )
				{
					hzb_log_error(__FILE__, __LINE__,"pfuncBusinessProcess error[%d]",rnet ) ;
				}

				hzb_log_info(__FILE__, __LINE__,"结束用户业务处理函数");
				gettimeofday( &end, NULL );

				span = end.tv_sec - start.tv_sec + ( end.tv_usec - start.tv_usec )/1000000.0;
				hzb_log_info(__FILE__, __LINE__,"处理耗时:%lf", span);

				/* 断开组件 */
				if ( phandle )
				{
					UnloadPhandle() ;
				}
				
END:
				/* pipe回应 */
				/* 返回值4表示无记录 */
				if( rnet && rnet != 4 )
					procfg.result = rnet ;
				else
				{
					procfg.result = 0 ;
					procfg.spend_time = span;
				}

				rnet = write( pipe_rsp, &procfg, sizeof( procfg ) );
				if( rnet != sizeof(procfg) )
				{
					if( rnet == 0 )
						hzb_log_error(__FILE__, __LINE__,"管道已关闭\n");
					else
						hzb_log_error(__FILE__, __LINE__, "write err errno[%d][%s]\n",errno,strerror(errno) ) ;
					break;
				}
				else
					hzb_log_info(__FILE__, __LINE__,"发送回应result[%d]成功\n", procfg.result);
			}
		}	
		else if( rnet == -1 )		
		{
			hzb_log_error(__FILE__, __LINE__, "select is interrupt[%d]", rnet) ;
		}
		else
		{
			hzb_log_error(__FILE__, __LINE__, "select other error [%d]", rnet) ;
		}
	}

	/* 关闭数据库 */
        BusinessDataBaseClose() ;	

	/* 释放内存空间 */
        IB2FreeEnvironment( &penv ) ;	

	hzb_log_info(__FILE__, __LINE__, "子进程成功终止") ;

	return	0;	
}



int LoadUserBusinessRll()
{
	int rnet ;
	char filename[ 512 + 1 ] ;
		
	memset( filename , 0x00, sizeof(filename) ) ;
	sprintf( filename, "%s/modules/%s", getenv("HOME"), procfg.so_name ) ;

	phandle = OpenLinkLibrary( filename ) ;
	if( phandle == NULL )
	{
		hzb_log_error(__FILE__, __LINE__, "    dlerror()[%s]\n" , dlerror() ) ; 
		hzb_log_error(__FILE__, __LINE__, "打开业务处理组件文件[%s]失败\n" , filename ) ; 
                return RETCODE_ERROR_OPENRllFILE_FAILED ;	
	}

        pfuncBusinessProcess = GetFunctionAddress( phandle , procfg.service_func )  ;
        if( pfuncBusinessProcess == NULL )
        {
		hzb_log_error(__FILE__, __LINE__, "    dlerror()[%s]\n" , dlerror() ) ; 
		hzb_log_error(__FILE__, __LINE__, "定位业务处理组件函数[%s]失败\n", procfg.service_func ) ; 
                CloseLinkLibrary( phandle ) ;
                return RETCODE_ERROR_GETFUNCPTR_FAILED ;
        }

	hzb_log_info(__FILE__, __LINE__, "挂接业务处理组件[%s]函数[%s]成功" , filename, procfg.service_func ) ; 

	return 0 ;
}

void UnloadPhandle()
{
        CloseLinkLibrary( phandle ) ;
        phandle = NULL  ;

        return ;
}

int GetCurrentSystemDate( char *szCurDate )
{
	int rnet ;

	time_t _time_;
	struct tm *local_t;

	if( szCurDate == NULL )
	{
		hzb_log_error(__FILE__, __LINE__, "szCurDate is NULL" );
		return -1;	
	}

	time( &_time_ );
	local_t = localtime( &_time_ );		

	sprintf( szCurDate, "%04d-%02d-%02d", local_t->tm_year+1900, local_t->tm_mon+1,local_t->tm_mday );	

	return 0;
}


