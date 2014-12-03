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
	/* �����ڴ�ռ� */
	rnet = IB2AllocEnvironment( &penv ) ;
	if ( rnet != 0 )
        {
		hzb_log_error(__FILE__, __LINE__, "�����ڴ�ռ���� IB2AllocEnvironment rnet=%d", rnet) ;
		exit(1);
        }
	hzb_log_info(__FILE__, __LINE__, "����IB2�ڴ�ռ�ɹ�" ) ; 

	/* ���������ļ� */
	rnet = IB2LoadClientConfig( penv , NULL ) ;
	if ( rnet != 0 )
        {
		hzb_log_error(__FILE__, __LINE__, "���������ļ����� IB2LoadClientConfig rnet=%d", rnet) ;
		exit(2) ;
        }

	/* �����ݿ� */
	rnet = BusinessDataBaseOpen() ;
        if ( rnet )
        {
                hzb_log_error(__FILE__, __LINE__, "�������ݿ�ʧ��, rnet=%d", rnet) ;
                exit(3) ;
        }	
	hzb_log_info(__FILE__, __LINE__, "�����ݿ����ӳɹ�" ) ;

	
	/* ȡ�õ�ǰ���� */
	memset( last_date, 0x00, sizeof( last_date ) ) ;
	GetCurrentSystemDate( last_date );

	hzb_log_info(__FILE__, __LINE__, "ϵͳ����[%s]", last_date ) ;	

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
			/* ���ݿ����� */
			memset( now_date, 0x00, sizeof( now_date ) ) ;
			GetCurrentSystemDate( now_date ) ;				
			if ( strcmp( now_date, last_date ) > 0 )
			{
				/* �ر����ݿ� */
				rnet = BusinessDataBaseClose() ;
				if( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__, "BusinessDataBaseClose error rnet=%d", rnet) ;
				}
				
				sleep(1);

				/* �����ݿ� */
				rnet = BusinessDataBaseOpen() ;
				if( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__, "BusinessDataBaseOpen error rnet=%d",rnet) ;

				}
				else
				{
					strcpy( last_date, now_date );
					hzb_log_info(__FILE__, __LINE__, "�����������ݿ�ɹ�" ) ;	
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
						hzb_log_error(__FILE__, __LINE__,"�ܵ��ѹر�");
					else
						hzb_log_error(__FILE__, __LINE__, "write err errno[%d][%s]",errno,strerror(errno) ) ;
					break;
				}

				if( !strcmp( procfg.cmd, SERVICE_CMD_STOP ) ||
			  	    !strcmp( procfg.cmd, SERVICE_CMD_RESTART ) ||
				    !strcmp( procfg.cmd, SERVICE_CMD_RELOAD ) )
				{
					hzb_log_info(__FILE__, __LINE__, "�յ�������[%s]����",procfg.cmd ) ;
					procfg.result = RESULT_END ;
					rnet = write( pipe_rsp, &procfg, sizeof( procfg ) );
					if( rnet != sizeof(procfg) )
					{
						hzb_log_error(__FILE__, __LINE__, "д�ܵ�����[%s]",strerror(errno)) ;
					}
					else
					{
						hzb_log_info(__FILE__, __LINE__,"���ͻ�Ӧresult[%d]�ɹ�", procfg.result);
					}
					break ;
				}

				/* �ҽ��û�ҵ������� */
				rnet = LoadUserBusinessRll();
				if ( rnet != 0 )
				{
					hzb_log_error(__FILE__, __LINE__,"�ҽ��û�ҵ�������ʧ��rnet=%d",rnet) ;
					goto END ;
				}

				gettimeofday( &start, NULL );
				hzb_log_info(__FILE__, __LINE__,"�����û�ҵ������");
				
				/* ҵ���� */		
				rnet = pfuncBusinessProcess( penv , procfg.arguments );
				if( rnet && rnet != 4 )
				{
					hzb_log_error(__FILE__, __LINE__,"pfuncBusinessProcess error[%d]",rnet ) ;
				}

				hzb_log_info(__FILE__, __LINE__,"�����û�ҵ������");
				gettimeofday( &end, NULL );

				span = end.tv_sec - start.tv_sec + ( end.tv_usec - start.tv_usec )/1000000.0;
				hzb_log_info(__FILE__, __LINE__,"�����ʱ:%lf", span);

				/* �Ͽ���� */
				if ( phandle )
				{
					UnloadPhandle() ;
				}
				
END:
				/* pipe��Ӧ */
				/* ����ֵ4��ʾ�޼�¼ */
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
						hzb_log_error(__FILE__, __LINE__,"�ܵ��ѹر�\n");
					else
						hzb_log_error(__FILE__, __LINE__, "write err errno[%d][%s]\n",errno,strerror(errno) ) ;
					break;
				}
				else
					hzb_log_info(__FILE__, __LINE__,"���ͻ�Ӧresult[%d]�ɹ�\n", procfg.result);
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

	/* �ر����ݿ� */
        BusinessDataBaseClose() ;	

	/* �ͷ��ڴ�ռ� */
        IB2FreeEnvironment( &penv ) ;	

	hzb_log_info(__FILE__, __LINE__, "�ӽ��̳ɹ���ֹ") ;

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
		hzb_log_error(__FILE__, __LINE__, "��ҵ��������ļ�[%s]ʧ��\n" , filename ) ; 
                return RETCODE_ERROR_OPENRllFILE_FAILED ;	
	}

        pfuncBusinessProcess = GetFunctionAddress( phandle , procfg.service_func )  ;
        if( pfuncBusinessProcess == NULL )
        {
		hzb_log_error(__FILE__, __LINE__, "    dlerror()[%s]\n" , dlerror() ) ; 
		hzb_log_error(__FILE__, __LINE__, "��λҵ�����������[%s]ʧ��\n", procfg.service_func ) ; 
                CloseLinkLibrary( phandle ) ;
                return RETCODE_ERROR_GETFUNCPTR_FAILED ;
        }

	hzb_log_info(__FILE__, __LINE__, "�ҽ�ҵ�������[%s]����[%s]�ɹ�" , filename, procfg.service_func ) ; 

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


