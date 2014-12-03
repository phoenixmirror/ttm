#include "ttm.h"

stDefprocinfo *g_processinfo[MAX_PROCESSES] = { 0 };	/* 进程信息 */
char g_ip_addr[30];
int  g_listen_port; 

int serverLoop()
{
	int	i = 0 ;
	int	irc = 0 ;
	int	maxfd = 0;
	int	process_index = 0 ;
	int	accept_addrlen = 0 ;
	int	accept_sock = 0 ;
	int	listen_sock = 0 ;
	int	port = 0 ;
	char	ip[30];
	time_t	t1 = 0, t2 = 0;
	fd_set	readfds ;
	struct sigaction act ;
	struct sockaddr	accept_addr ;
	struct timeval	tv ;
	stDefproccfg	stservice;
	stDefsercfg	*psttmpsercfg = NULL;
	
	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	/* 预分配进程空间 */
	irc = allocate_resource();
	if ( irc )
	{
		printf( "预分配服务空间错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "allocate_resource fail" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "allocate_resource succ" );
	
	/* 起管理监听 */
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		printf( "获取node.ttm配置错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}
	
	hzb_log_info( __FILE__ , __LINE__ , "listen ip=[%s],port=[%d]", ip, port );
	irc = socket_listen( ip, port, &listen_sock );
	if ( irc )
	{
		printf( "ttm管理进程监听错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "socket_listen fail" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_listen succ" );

	printf( "\n守护进程管理启动\n" );

	memset( g_ip_addr, 0x00, sizeof(g_ip_addr) );
	strcpy( g_ip_addr, ip );
	g_listen_port = port;
	
	memset( &act, 0x00, sizeof(struct sigaction) );
	act.sa_handler = SIG_DFL ;
	sigemptyset( &act.sa_mask );
	act.sa_flags = SA_RESTART ;
	sigaction( SIGCHLD, &act, NULL );

	/* while()循环监听事件 */
	while(1)
	{
		FD_ZERO( &readfds );
		FD_SET( listen_sock , &readfds );

		maxfd = listen_sock;
		for( i = 0; i < MAX_PROCESSES; i++ )
		{
			if( g_processinfo[i]->pid > 0 )
			{
			/*hzb_log_info( __FILE__ , __LINE__ , "轮询开始 进程[%ld]等待时间[%d]", g_processinfo[i]->pid, g_processinfo[i]->procfg.wait_time );*/
			FD_SET( g_processinfo[i]->response_pipes[0], &readfds );
			if( g_processinfo[i]->response_pipes[0] > maxfd )
				maxfd = g_processinfo[i]->response_pipes[0];
			}
		}
		
		time( &t1 );
		tv.tv_sec = 1 ;
		tv.tv_usec = 0 ;
		irc = select( maxfd+1, &readfds, NULL, NULL, &tv );
		if( irc == -1 )
		{
			hzb_log_info( __FILE__ , __LINE__ , "select failed , errno=[%d]" , errno );
			break;
		}
		else if( irc > 0 )
		{
			if( FD_ISSET( listen_sock, &readfds ) )
			{
				/* 接受新客户端连接 */
				hzb_log_info( __FILE__ , __LINE__ , "接收socket连接" );
				accept_addrlen = sizeof(struct sockaddr) ;
				memset(&accept_addr, 0x00, accept_addrlen);
				accept_sock = accept( listen_sock, &accept_addr, &accept_addrlen ) ;
				if( accept_sock == -1 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "accept failed , errno=[%d]", errno );
					break;
				}

				psttmpsercfg = ( stDefsercfg * )malloc( sizeof(stDefsercfg) );
				if( psttmpsercfg == NULL )
				{
					hzb_log_error( __FILE__ , __LINE__ , "malloc fail" );
					break;			
				}
				memset( (char *)psttmpsercfg, 0x00, sizeof(stDefsercfg) );

				hzb_log_info( __FILE__ , __LINE__ , "recv sock data" );
				/* 接收管理命令  新增 关闭 查看 调整参数 */
				irc = recvn( accept_sock, sizeof(stDefsercfg), (char *)psttmpsercfg, TIMEOUT );
				if( irc ) 
				{
					hzb_log_error( __FILE__ , __LINE__ , "recv failed , errno=[%d]", errno );
					close( accept_sock );
					free( psttmpsercfg );
					break;
				}

				hzb_log_info( __FILE__ , __LINE__ , "service_name=[%s]", psttmpsercfg->service_name );
				hzb_log_info( __FILE__ , __LINE__ , "so_name=[%s]", psttmpsercfg->so_name );
				hzb_log_info( __FILE__ , __LINE__ , "service_func=[%s]", psttmpsercfg->service_func );
				hzb_log_info( __FILE__ , __LINE__ , "arguments=[%s]", psttmpsercfg->arguments );
				hzb_log_info( __FILE__ , __LINE__ , "trigger_interval=[%s]", psttmpsercfg->trigger_interval );
				hzb_log_info( __FILE__ , __LINE__ , "trigger_interval_value=[%s]", psttmpsercfg->trigger_interval_value );
				hzb_log_info( __FILE__ , __LINE__ , "trigger_type=[%s]", psttmpsercfg->trigger_type );
				hzb_log_info( __FILE__ , __LINE__ , "trigger_start_time=[%s]", psttmpsercfg->trigger_start_time );
				hzb_log_info( __FILE__ , __LINE__ , "trigger_end_time=[%s]", psttmpsercfg->trigger_end_time );
				hzb_log_info( __FILE__ , __LINE__ , "hostname=[%s]", psttmpsercfg->hostname );
				hzb_log_info( __FILE__ , __LINE__ , "service_number=[%d]", psttmpsercfg->service_number );
				hzb_log_info( __FILE__ , __LINE__ , "interval=[%d]", psttmpsercfg->interval );	
				hzb_log_info( __FILE__ , __LINE__ , "timeout=[%d]", psttmpsercfg->timeout );	
				hzb_log_info( __FILE__ , __LINE__ , "max_cnt=[%d]", psttmpsercfg->max_cnt );	

				/* 新增 关闭 调整 */
				if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_ADD ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收新增服务命令" );
					irc = add_service( psttmpsercfg, accept_sock );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "excute ADD cmd fail" );
					}
					hzb_log_info( __FILE__ , __LINE__ , "excute ADD cmd complete" );
				}
				else if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_STOP ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收停止服务命令" );
					irc = stop_service( psttmpsercfg, accept_sock );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "excute STOP cmd fail" );
					}
					hzb_log_info( __FILE__ , __LINE__ , "excute STOP cmd complete" );
				}
				else if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_FORCE ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收强制停止服务命令" );
					irc = force_stop_service( psttmpsercfg, accept_sock );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "excute FORCE STOP cmd fail" );
					}
					hzb_log_info( __FILE__ , __LINE__ , "excute FORCE STOP cmd complete" );
				}
				else if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_RELOAD ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收重载服务命令" );
					irc = mod_service( psttmpsercfg, accept_sock );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "excute reload cmd fail" );
					}
					hzb_log_info( __FILE__ , __LINE__ , "excute RELOAD cmd complete" );
				}
				else if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_VIEW ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收查看服务命令" );
					irc = view_service( psttmpsercfg, accept_sock );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "excute VIEW cmd fail" );
					}
					hzb_log_info( __FILE__ , __LINE__ , "excute VIEW cmd complete" );
				}
				else if( strcmp( psttmpsercfg->cmd, SERVICE_CMD_CLOSE ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "管理进程接收退出命令" );
					close( accept_sock );
					accept_sock = 0;
					free( psttmpsercfg );
					break;
				}
				else
				{
					hzb_log_error( __FILE__ , __LINE__ , "recv invalid cmd [%s]", psttmpsercfg->cmd );
				}

				close( accept_sock );
				accept_sock = 0;
				free( psttmpsercfg );
				
			}
			else
			{
				/* 子进程返回信息 更新子进程信息 */
				for( process_index = 0; process_index < MAX_PROCESSES; process_index++ )
				{
					if( g_processinfo[process_index]->pid <= 0 )
						continue;	

					if( FD_ISSET( g_processinfo[process_index]->response_pipes[0], &readfds ) )
					{
						/* 读取结果 */
						memset( (char*)&stservice, 0x00, sizeof(stservice) );
                                                irc = read( g_processinfo[process_index]->response_pipes[0], (char*)&stservice, sizeof(stDefproccfg) ) ;
                                                if( irc == 0 )
                                                {
							if( g_processinfo[process_index]->response_pipes[0] > 0 )
							{
								close(g_processinfo[process_index]->response_pipes[0]);	
								g_processinfo[process_index]->response_pipes[0] = 0;
							}
							hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]读管道已关闭 errno[%d]", g_processinfo[process_index]->pid, errno );
							if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
								strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
							kill( g_processinfo[process_index]->pid, SIGKILL );
							continue;
                                                }
                                                else if( irc != sizeof(stDefproccfg) )
                                                {
                                                        hzb_log_error( __FILE__ , __LINE__ , "子进程[%ld]读管道错误 error[%d], errno[%d]" , g_processinfo[process_index]->pid, irc, errno );
							if( g_processinfo[process_index]->response_pipes[0] > 0 )
							{
								close(g_processinfo[process_index]->response_pipes[0]);	
								g_processinfo[process_index]->response_pipes[0] = 0;
							}
							hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]读管道已关闭 errno[%d]", g_processinfo[process_index]->pid, errno );
							if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
								strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
							kill( g_processinfo[process_index]->pid, SIGKILL );
							continue;
                                                }

						if( stservice.result == RESULT_END )
						{
							hzb_log_info( __FILE__ , __LINE__ , "进程[%ld]服务[%s][%s]处理结束" ,g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.so_name, g_processinfo[process_index]->procfg.service_func );
							continue;
						}

						if( stservice.result != 0 )
						{
							hzb_log_error( __FILE__ , __LINE__ , "进程[%ld]服务[%s][%s]处理失败[%d]", g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.so_name, g_processinfo[process_index]->procfg.service_func, stservice.result );
						}
						else
						{
							hzb_log_info( __FILE__ , __LINE__ , "进程[%ld]服务[%s][%s]处理成功,耗时[%lf]" ,g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.so_name, g_processinfo[process_index]->procfg.service_func, stservice.spend_time );
							g_processinfo[process_index]->procfg.succ_cnt ++  ;
							g_processinfo[process_index]->procfg.spend_time += stservice.spend_time ;
						}
						g_processinfo[process_index]->procfg.cur_cnt ++  ;
						g_processinfo[process_index]->procfg.status = SERVICE_STATUS_IDLE ;
						g_processinfo[process_index]->procfg.wait_time = 0 ;
						g_processinfo[process_index]->procfg.work_time = 0 ;
						memset( g_processinfo[process_index]->procfg.serv_start_time, 0x00, sizeof(g_processinfo[process_index]->procfg.serv_start_time) );
					}

				}
				
			}

			/* 空闲服务等待时间累加 */
			time( &t2 );
			wait_time_increase( t2-t1 );
			work_time_increase( t2-t1 );

		}
		else if( irc == 0 )
		{
			/* select达到超时时间 是否对子进程做进程超时时间控制 */
			/*hzb_log_info( __FILE__ , __LINE__ , "轮询时间到" );*/
			
			/* 空闲服务等待时间累加 */
			wait_time_increase( 1 );
			/* 工作时间累加 */
			work_time_increase( 1 );
			
			/* 是否有进程空闲 分配任务 */
			irc = allocate_idle_service();
			if( irc )
			{
				hzb_log_error( __FILE__ , __LINE__ , "分配空间进程任务失败" );
				break;
			}
			
			/* 超时控制 */
			/* 判断子进程是否超时, 超时直接发信号终止??? */
			for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
			{
				if( g_processinfo[process_index]->pid > 0 )
				{
					if( g_processinfo[process_index]->procfg.work_time > g_processinfo[process_index]->procfg.timeout )		
					{
						hzb_log_error( __FILE__ , __LINE__ , "进程[%ld]服务[%s][%s]超时 强行结束",g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.so_name , g_processinfo[process_index]->procfg.service_name );
						/* 是否强行直接结束???  */
						kill( g_processinfo[process_index]->pid, SIGKILL );
						strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_KILL );
					}
				}
			}
			
		}
		else
		{
			hzb_log_error( __FILE__ , __LINE__ , "其他错误 irc[%d] errno=[%d]", irc, errno );
			return -1;
		}
		
		/* 回收子进程 */
		irc = reclaim_subprocess();
		if( irc )
		{
			hzb_log_error( __FILE__ , __LINE__ , "reclaim_subprocess fail" );
			break;
		}
	}

	hzb_log_info( __FILE__ , __LINE__ , "关闭所有服务" );
	/* 关闭所有子进程 */
	/*stop_all();*/
	
	hzb_log_info( __FILE__ , __LINE__ , "管理进程结束" );
	printf( "守护进程管理结束\n" );
	return 0;
}


int allocate_resource()
{
	int	i = 0;
	for( i = 0; i < MAX_PROCESSES; i++ )
	{
		g_processinfo[i] = ( stDefprocinfo * )malloc( sizeof(stDefprocinfo) );
		if( g_processinfo[i] == NULL )
		{
			hzb_log_error( __FILE__ , __LINE__ , "malloc fail" );
			return	-1;
		}
		memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
	}
	return	0;
}

/* 首次启动服务 */
int start_process( int argc, char *argv[] )
{
	int	pid=0;
	int	sig=0,fd=0;
	
	pid=fork();
	switch( pid )
	{
		case -1:
			return -1;
		case 0:
			break;
		default	:
			exit( 0 );	
			break;
	}
	
	signal(SIGHUP,  SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGIO,   SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGPIPE, SIG_IGN );
	
	setsid();
	serverLoop();
	
	return	0;
}


/* 增加服务 */
int add_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_add" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	
	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	memset( cfgfile, 0x00, sizeof(cfgfile) );

	if( argv[0] == NULL )
	{
		printf( "缺少服务配置档\n" );
		return -1;
	}

	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg [%s]", argv[0] );
	strcpy( cfgfile, argv[0] );
	/* 读取配置档 每个服务配置档分开 */
	irc = get_service_cfg( cfgfile, &stsercfg );
	if ( irc )
	{
		printf( "读取服务配置档错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "get_service_cfg fail" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg succ" );

	/* 检查服务配置 */
	irc = check_service_cfg( &stsercfg );
	if ( irc )
	{
		printf( "检查服务配置档错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "check_service_cfg fail" );
		return	irc;
	}
	strcpy( stsercfg.cmd, SERVICE_CMD_ADD );
	hzb_log_info( __FILE__ , __LINE__ , "check_service_cfg succ" );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn fail" );
		return	irc;
	}

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	irc = recvn( connect_sock, sizeof(stBuf.buffer_len), stBuf.buffer_len, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data len failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	
	irc = recvn( connect_sock, atoi(stBuf.buffer_len), stBuf.buffer, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}

	printf( "执行结果: %s\n", stBuf.buffer );
	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "add_process end" );
	return irc;
}


/* 修改服务 */
int mod_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_reload" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	
	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	memset( cfgfile, 0x00, sizeof(cfgfile) );

	if( argv[0] == NULL )
	{
		printf( "缺少服务配置档\n" );
		return -1;
	}

	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg [%s]", argv[0] );
	strcpy( cfgfile, argv[0] );
	/* 读取配置档 每个服务配置档分开 */
	irc = get_service_cfg( cfgfile, &stsercfg );
	if ( irc )
	{
		printf( "读取服务配置档错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "get_service_cfg fail" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg succ" );

	/* 检查服务配置 */
	irc = check_service_cfg( &stsercfg );
	if ( irc )
	{
		printf( "检查服务配置档错误\n" );
		hzb_log_error( __FILE__ , __LINE__ , "check_service_cfg fail" );
		return	irc;
	}
	strcpy( stsercfg.cmd, SERVICE_CMD_RELOAD );
	hzb_log_info( __FILE__ , __LINE__ , "check_service_cfg succ" );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn fail" );
		return	irc;
	}

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	irc = recvn( connect_sock, sizeof(stBuf.buffer_len), stBuf.buffer_len, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data len failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	
	irc = recvn( connect_sock, atoi(stBuf.buffer_len), stBuf.buffer, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}

	printf( "执行结果: %s\n", stBuf.buffer );
	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "mod_process end" );
	return irc;
}


/* 停止服务 */
int stop_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_stop" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	
	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	memset( cfgfile, 0x00, sizeof(cfgfile) );

	if( argv[0] == NULL )
	{
		printf( "缺少服务配置档\n" );
		return -1;
	}

	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg [%s]", argv[0] );

	if( strcmp( argv[0], "all" ) == 0 )
	{
		strcpy( stsercfg.arguments, "all" );
		hzb_log_info( __FILE__ , __LINE__ , "查看全部服务信息" );
	}
	else
	{
		strcpy( cfgfile, argv[0] );
		/* 读取配置档 每个服务配置档分开 */
		irc = get_service_cfg( cfgfile, &stsercfg );
		if ( irc )
		{
			hzb_log_error( __FILE__ , __LINE__ , "get_service_cfg fail" );
			printf( "读取服务配置档错误\n" );
			return	irc;
		}
		hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg succ" );

		/* 不需检查配置档 */	
		/* 检查服务配置 */
		/*irc = check_service_cfg( &stsercfg );
		if ( irc )
		{
			printf( "检查服务配置档错误\n" );
			hzb_log_error( __FILE__ , __LINE__ , "check_service_cfg fail" );
			return	irc;
		}*/
		strcpy( stsercfg.arguments, "one" );
	}

	strcpy( stsercfg.cmd, SERVICE_CMD_STOP );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn fail" );
		return	irc;
	}

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	irc = recvn( connect_sock, sizeof(stBuf.buffer_len), stBuf.buffer_len, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data len failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	
	irc = recvn( connect_sock, atoi(stBuf.buffer_len), stBuf.buffer, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}

	printf( "执行结果: %s\n", stBuf.buffer );
	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "stop_process end" );
	return irc;
}



/* 强制停止服务 */
int force_stop_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_force_stop" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	
	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	memset( cfgfile, 0x00, sizeof(cfgfile) );

	if( argv[0] == NULL )
	{
		printf( "缺少服务配置档\n" );
		return -1;
	}

	hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg [%s]", argv[0] );

	if( strcmp( argv[0], "all" ) == 0 )
	{
		strcpy( stsercfg.arguments, "all" );
		hzb_log_info( __FILE__ , __LINE__ , "查看全部服务信息" );
	}
	else
	{
		strcpy( cfgfile, argv[0] );
		/* 读取配置档 每个服务配置档分开 */
		irc = get_service_cfg( cfgfile, &stsercfg );
		if ( irc )
		{
			hzb_log_error( __FILE__ , __LINE__ , "get_service_cfg fail" );
			printf( "读取服务配置档错误\n" );
			return	irc;
		}
		hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg succ" );

		/* 不需检查配置档 */	
		/* 检查服务配置 */
		/*irc = check_service_cfg( &stsercfg );
		if ( irc )
		{
			printf( "检查服务配置档错误\n" );
			hzb_log_error( __FILE__ , __LINE__ , "check_service_cfg fail" );
			return	irc;
		}*/
		strcpy( stsercfg.arguments, "one" );
	}

	strcpy( stsercfg.cmd, SERVICE_CMD_FORCE );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn fail" );
		return	irc;
	}

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	irc = recvn( connect_sock, sizeof(stBuf.buffer_len), stBuf.buffer_len, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data len failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	
	irc = recvn( connect_sock, atoi(stBuf.buffer_len), stBuf.buffer, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}

	printf( "执行结果: %s\n", stBuf.buffer );
	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "stop_process end" );
	return irc;
}


/* 查看服务 */
int view_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_view" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	
	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	memset( cfgfile, 0x00, sizeof(cfgfile) );

	if( argv[0] == NULL )
	{
		printf( "缺少参数\n" );
		return -1;
	}


	if( strcmp( argv[0], "all" ) == 0 )
	{
		strcpy( stsercfg.arguments, "all" );
		hzb_log_info( __FILE__ , __LINE__ , "查看全部服务信息" );
	}
	else if( strcmp( argv[0], "status" ) == 0 )
	{
		strcpy( stsercfg.arguments, "status" );
		hzb_log_info( __FILE__ , __LINE__ , "查看管理进程状态" );
	}
	else
	{
		hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg [%s]", argv[0] );
		strcpy( cfgfile, argv[0] );
		/* 读取配置档 每个服务配置档分开 */
		irc = get_service_cfg( cfgfile, &stsercfg );
		if ( irc )
		{
			printf( "读取服务配置档错误\n" );
			hzb_log_error( __FILE__ , __LINE__ , "get_service_cfg fail" );
			return	irc;
		}
		hzb_log_info( __FILE__ , __LINE__ , "get_service_cfg succ" );

		/* 不需检查配置档 */
		/* 检查服务配置 */
		/*irc = check_service_cfg( &stsercfg );
		if ( irc )
		{
			printf( "检查服务配置档错误\n" );
			hzb_log_error( __FILE__ , __LINE__ , "check_service_cfg fail" );
			return	irc;
		}*/
		strcpy( stsercfg.arguments, "one" );
	}

	strcpy( stsercfg.cmd, SERVICE_CMD_VIEW );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	hzb_log_info( __FILE__ , __LINE__ , "send view cmd" );
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "send view cmd fail" );
		close( connect_sock );
		return	irc;
	}

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	irc = recvn( connect_sock, sizeof(stBuf.buffer_len), stBuf.buffer_len, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data len failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "recv view_process data len [%s] ", stBuf.buffer_len );

	if( atoi(stBuf.buffer_len) == 0 )
	{
		printf( "无服务信息\n" );
		return	irc;
	}	
	
	hzb_log_info( __FILE__ , __LINE__ , "recv view_process data" );
	irc = recvn( connect_sock, atoi(stBuf.buffer_len), stBuf.buffer, TIMEOUT );
	if( irc ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recv data failed , errno=[%d]", errno );
		close( connect_sock );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "%s", stBuf.buffer );

	printf( "查看结果:\n" );
	printf( "%s\n", stBuf.buffer );

	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "view_process end" );
	return irc;
}



/* 关闭管理进程 */
int close_process( int argc, char *argv[] )
{
	stDefbuf	stBuf;
	stDefsercfg	stsercfg;
	char		cfgfile[50];
	char		ip[30];
	int		port = 0;
	int		connect_sock = 0;
	int		irc = 0;

	irc = hzb_log_init();
	if( irc  )
	{
		printf( "hzb_log_init failed[%d]\n" , irc );
		return -1;
	}
	
	irc = hzb_log_set_category( "ttm_close" );
	if( irc )
	{
		printf( "hzb_log_set_category failed[%d]\n" , irc );
		return -1;
	}

	memset( (char *)&stsercfg, 0x00, sizeof(stDefsercfg) );
	strcpy( stsercfg.cmd, SERVICE_CMD_CLOSE );
	
	memset( ip, 0x00, sizeof(ip) );
	irc = get_sock_cfg( ip, &port );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "get_sock_cfg fail" );
		return	irc;
	}

	irc = socket_connect( ip, port, &connect_sock );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket_connect fail" );
		printf( "连接不到服务\n" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket_connect succ" );
	
	irc = sendn( connect_sock, sizeof(stDefsercfg), (char *)&stsercfg, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "close view cmd fail" );
		close( connect_sock );
		return	irc;
	}

	close( connect_sock );
	hzb_log_info( __FILE__ , __LINE__ , "close_process end" );
	return irc;
}




int reclaim_subprocess()
{
	pid_t		pid = 0 ;
	int		iExitStat = 0 ;
	int		i = 0 ;
	int		irc = 0 ;
	int		irestart = 0;		
	
	while( ( pid = waitpid( -1, &iExitStat, WNOHANG ) ) > 0 )
	{
		hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]结束状态 iExitStat[%d] WIFEXITED[%d] WEXITSTATUS[%d]", 
		pid, iExitStat, WIFEXITED(iExitStat), WEXITSTATUS(iExitStat) );

		for( i = 0 ; i < MAX_PROCESSES ; i++ )
		{
			if( g_processinfo[i]->pid == pid )
			{
				/* 被信号中断 */
				if( WIFSIGNALED(iExitStat) )
					irestart = 1;

				if( WEXITSTATUS(iExitStat) == 1 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s] 分配IB2环境变量错误  关闭进程", pid, g_processinfo[i]->procfg.service_name, g_processinfo[i]->procfg.cmd );
					memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
					break;
				}
				if( WEXITSTATUS(iExitStat) == 2 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s] 载入IB2配置文件错误  关闭进程", pid, g_processinfo[i]->procfg.service_name, g_processinfo[i]->procfg.cmd );
					memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
					break;
				}
				if( WEXITSTATUS(iExitStat) == 3 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s] 连接数据库错误 关闭进程", pid, g_processinfo[i]->procfg.service_name, g_processinfo[i]->procfg.cmd );
					memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
					break;
				}
				
				if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_RESTART ) == 0 ||
					strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_RELOAD ) == 0 ||
					strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_KILL ) == 0 ||
					strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_PIPE ) == 0 
				)
				{
					if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_RESTART ) == 0 )
						hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到重启命令", pid, g_processinfo[i]->procfg.service_name );
					if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_RELOAD ) == 0 )
						hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到重载命令", pid, g_processinfo[i]->procfg.service_name );
					if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_KILL ) == 0 )
						hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到超时强制退出命令", pid, g_processinfo[i]->procfg.service_name );

					if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_PIPE ) == 0 )
						hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到管道异常强制退出命令", pid, g_processinfo[i]->procfg.service_name );


RESTART:
					g_processinfo[i]->pid = -1;
					memset( g_processinfo[i]->start_date, 0x00, sizeof(g_processinfo[i]->start_date) );

					if( g_processinfo[i]->command_pipes[1] > 0 )
						close( g_processinfo[i]->command_pipes[1] );
					if( g_processinfo[i]->response_pipes[0] > 0 )
						close( g_processinfo[i]->response_pipes[0] );

					g_processinfo[i]->command_pipes[0] = 0;
					g_processinfo[i]->command_pipes[1] = 0;
					g_processinfo[i]->response_pipes[0] = 0;
					g_processinfo[i]->response_pipes[1] = 0;
					g_processinfo[i]->procfg.wait_time = 0;	/* 已等待时间 */
					g_processinfo[i]->procfg.work_time = 0;	/* 已运行时间 */			
					memset( g_processinfo[i]->procfg.serv_start_time, 0x00, sizeof(g_processinfo[i]->procfg.serv_start_time) );	/* 服务开始时间 */
					memset( g_processinfo[i]->procfg.cmd, 0x00, sizeof(g_processinfo[i]->procfg.cmd) );
					g_processinfo[i]->procfg.status = SERVICE_STATUS_IDLE;	/* 状态 */
					g_processinfo[i]->procfg.result = 0;	/* 结果 */
					g_processinfo[i]->procfg.cur_cnt = 0;	/* 运行次数 */
					g_processinfo[i]->procfg.succ_cnt = 0;	/* 成功运行次数 */
					g_processinfo[i]->procfg.spend_time = 0.0;	/* 平均耗时 */
					
					/* 启动服务 */
					irc = create_one_process( i );
					if( irc )
					{
						hzb_log_error( __FILE__ , __LINE__ , "create_one_process fail" );
						return -1;
					}
					g_processinfo[i]->service_index = i;
					get_time( g_processinfo[i]->start_date );		

					hzb_log_info( __FILE__ , __LINE__ , "启动子进程服务[%s]", g_processinfo[i]->procfg.service_name );
				}
				else if( strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_STOP ) == 0 ||
					strcmp( g_processinfo[i]->procfg.cmd, SERVICE_CMD_FORCE ) == 0
				)
				{
					hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到关闭命令", pid, g_processinfo[i]->procfg.service_name );
					memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
				}
				else
				{
					if( irestart )
					{
						hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]被信号中断 重启子进程", pid );
						goto RESTART;
					}
					hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]服务[%s]收到未知命令[%s] 关闭进程", pid, g_processinfo[i]->procfg.service_name, g_processinfo[i]->procfg.cmd );
					memset( g_processinfo[i], 0x00, sizeof(stDefprocinfo) );
				}
				break;
			}
		}
	}
	return 0;
}

/* 关闭所有子进程 */
int stop_all()
{
	int	irc = 0;
	int	process_index = 0;
	int 	sleep_cnt = 120;
	/* 关闭所有子进程 结束 */
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid > 0 )
		{		
			memset( g_processinfo[process_index]->procfg.cmd, 0x00, sizeof(g_processinfo[process_index]->procfg.cmd) );
			strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_STOP );
			irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
			if( irc == -1 )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write cmd error[%d], errno[%d]", irc, errno );
                        }
                        else if( irc != sizeof(stDefproccfg) )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write error[%d], errno[%d]", irc, errno );
                        }
		}
	}
	sleep(5);
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ ) 
	{
		if( g_processinfo[process_index]->pid > 0 )
		{
			irc = kill( g_processinfo[process_index]->pid, 0 );
			if ( irc == 0 ) 
			{
				process_index -= 1;
				sleep_cnt--;
				sleep(1);		
			}
			else
			{
				g_processinfo[process_index]->pid = 0;
			}
			if( sleep_cnt <= 0 )
				break;
		}
	}
	
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ ) 
	{
		if( g_processinfo[process_index]->pid > 0 )
		{
			hzb_log_info( __FILE__ , __LINE__ , "强行关闭子进程" );
			kill( g_processinfo[process_index]->pid, SIGKILL );
		}
	}
	hzb_log_info( __FILE__ , __LINE__ , "子进程全部关闭" );
	
	return	0;
}


int create_one_process( int index )
{
	int	irc = 0;
	int	i = 0;
	char	logfile[100];
	
	hzb_log_info( __FILE__ , __LINE__ , "开始启动子进程 index=%d", index );
	/* 创建管道 */
	irc = pipe( g_processinfo[index]->command_pipes );
	if( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "pipe failed[%d] , errno[%d]", irc, errno );
		return -1;
	}
	
	/* 创建响应管道 */
	irc = pipe( g_processinfo[index]->response_pipes );
	if( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "pipe failed[%d] , errno[%d]", irc, errno );
		return -1;
	}
	
	/* 创建任务进程 */
	g_processinfo[index]->pid = fork();
	if( g_processinfo[index]->pid < 0 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "fork failed, errno[%d]", errno );
		return -1;
	}
	else if( g_processinfo[index]->pid == 0 )
	{
		memset( logfile, 0x00, sizeof(logfile));
		sprintf( logfile, "ttm_%s_%d", g_processinfo[index]->procfg.service_name, getpid() );
		irc = hzb_log_set_category( logfile );
		if( irc )
		{
			printf( "hzb_log_set_category failed[%d]\n" , irc );
			return -1;
		}
		
		hzb_log_info( __FILE__ , __LINE__ , "子进程[%ld]启动" , (long)getpid() );
		
		close( g_processinfo[index]->command_pipes[1] );
		close( g_processinfo[index]->response_pipes[0] );

		/* 关闭不使用的管道 */
		for( i = 0; i < MAX_PROCESSES; i++ )
		{
			if( g_processinfo[i]->pid > 0 )
			{
				close( g_processinfo[i]->command_pipes[1] );
				close( g_processinfo[i]->response_pipes[0] );
			}
		}
		
		signal( SIGTERM , SIG_DFL );
		signal( SIGCHLD , SIG_DFL );
		
		hzb_log_info( __FILE__ , __LINE__ , "任务开始" );
		worker( g_processinfo[index]->command_pipes[0], g_processinfo[index]->response_pipes[1] ); 
		hzb_log_info( __FILE__ , __LINE__ , "任务结束\n\n" );
		exit(0);
		
	}
	else
	{
		close( g_processinfo[index]->command_pipes[0] );
		close( g_processinfo[index]->response_pipes[1] );
	}	
	
	hzb_log_info( __FILE__ , __LINE__ , "启动子进程[%ld]成功", g_processinfo[index]->pid );
	return	irc;
}




int allocate_idle_service()
{
	int	irc = 0;
	int	process_index = 0;

	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid <= 0 )
			continue;

		/*hzb_log_info( __FILE__ , __LINE__ , "遍历进程[%ld]等待时间[%d]", g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.wait_time );*/

		if( g_processinfo[process_index]->procfg.status != SERVICE_STATUS_IDLE )
			continue;

		/* 判断执行次数是否超过最大次数 */
		if( g_processinfo[process_index]->procfg.cur_cnt >= g_processinfo[process_index]->procfg.max_cnt )
		{
			hzb_log_info( __FILE__ , __LINE__ , "进程[%ld]执行次数达到最大次数", g_processinfo[process_index]->pid );
			hzb_log_info( __FILE__ , __LINE__ , "运行次数统计 cur_cnt=[%d],max_cnt[%d]", g_processinfo[process_index]->procfg.cur_cnt, g_processinfo[process_index]->procfg.max_cnt );
			/* 执行超过最大次数 通知进程结束 发送RESTART命令  如果确保进程结束??? */
			memset( g_processinfo[process_index]->procfg.cmd, 0x00, sizeof(g_processinfo[process_index]->procfg.cmd));
			strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_RESTART );
			irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
			if( irc == 0 )
			{
				hzb_log_warn( __FILE__ , __LINE__ , "写管道关闭 errno[%d]",  errno );
				if( g_processinfo[process_index]->command_pipes[1] > 0 )
				{
					close(g_processinfo[process_index]->command_pipes[1]);
					g_processinfo[process_index]->command_pipes[1] = 0;
				}
				if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
					strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
				kill( g_processinfo[process_index]->pid, SIGKILL );
			}
			else if( irc != sizeof(stDefproccfg) )
			{
				hzb_log_error( __FILE__ , __LINE__ , "写管道错误 write error[%d], errno[%d]", irc, errno );
				if( g_processinfo[process_index]->command_pipes[1] > 0 )
				{
					close(g_processinfo[process_index]->command_pipes[1]);
					g_processinfo[process_index]->command_pipes[1] = 0;
				}
				if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
					strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
				kill( g_processinfo[process_index]->pid, SIGKILL );
			}
			continue;
		}
		
		/* 判断是否到达等待时间 已到则分配服务 没到则继续等待 */
		if( g_processinfo[process_index]->procfg.wait_time < g_processinfo[process_index]->procfg.interval )
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "进程[%ld]未到执行时间 服务[%s] 需等待[%d],已等待[%d]", 
			g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.service_name, g_processinfo[process_index]->procfg.interval, g_processinfo[process_index]->procfg.wait_time );*/
			continue;
		}

		/* 判断触发时刻 */
		irc = check_trigger_interval( &(g_processinfo[process_index]->procfg) );
		if( irc )
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "未满足触发时刻不分配任务" );*/
			continue;
		}
		
		hzb_log_info( __FILE__ , __LINE__ , "进程[%ld]已到执行时间 服务[%s] 需等待[%d],已等待[%d]", 
		g_processinfo[process_index]->pid, g_processinfo[process_index]->procfg.service_name, g_processinfo[process_index]->procfg.interval, g_processinfo[process_index]->procfg.wait_time );
		hzb_log_info( __FILE__ , __LINE__ , "分配服务[%s][%s],已执行次数[%d]", g_processinfo[process_index]->procfg.so_name, g_processinfo[process_index]->procfg.service_func, g_processinfo[process_index]->procfg.cur_cnt );
		
		irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
		if( irc == 0 )
		{
			hzb_log_warn( __FILE__ , __LINE__ , "分配任务写管道关闭 errno[%d]", irc, errno );
			if( g_processinfo[process_index]->command_pipes[1] > 0 )
			{
				close(g_processinfo[process_index]->command_pipes[1]);
				g_processinfo[process_index]->command_pipes[1] = 0;
			}
			if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
				strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
			kill( g_processinfo[process_index]->pid, SIGKILL );
		}
		else if( irc != sizeof(stDefproccfg) )
		{
			hzb_log_error( __FILE__ , __LINE__ , "分配任务写管道错误 irc[%d], errno[%d]", irc, errno );
			if( g_processinfo[process_index]->command_pipes[1] > 0 )
			{
				close(g_processinfo[process_index]->command_pipes[1]);
				g_processinfo[process_index]->command_pipes[1] = 0;
			}
			if( strlen(g_processinfo[process_index]->procfg.cmd) <= 0 )
				strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_PIPE );
			kill( g_processinfo[process_index]->pid, SIGKILL );
		}

		/* 状态 时间初始化 */
		g_processinfo[process_index]->procfg.status = SERVICE_STATUS_WORK;
		g_processinfo[process_index]->procfg.work_time = 0;
		g_processinfo[process_index]->procfg.wait_time = 0;
		get_time( g_processinfo[process_index]->procfg.serv_start_time );

	}
	return	0;	
}


int add_service( stDefsercfg *pstsercfg, int sock )
{
	int		process_index = 0;
	int		i = 0;
	int		irc = 0;
	stDefbuf	stBuf;

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	/* 根据配置档信息新增服务 */
	hzb_log_info( __FILE__ , __LINE__ , "新增服务开始" );

	
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid <= 0 )
			continue;

		if( strcmp( pstsercfg->service_name, g_processinfo[process_index]->procfg.service_name ) == 0  )
		{
			hzb_log_error( __FILE__ , __LINE__ , "服务已存在[%d]", process_index );
			sprintf( stBuf.buffer, "[%s]服务已存在", pstsercfg->service_name );
			goto SEND;
		}
	}

	for( i = 0 ; i < pstsercfg->service_number ; i++ )
	{
		for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
		{
			if( g_processinfo[process_index]->pid <= 0 )
			{
				hzb_log_info( __FILE__ , __LINE__ , "服务序号[%d]", process_index );
				cfg_to_proc( pstsercfg, &(g_processinfo[process_index]->procfg) );
				irc = create_one_process( process_index );
				if( irc )
				{
					hzb_log_error( __FILE__ , __LINE__ , "create_one_process fail" );
					break;
				}
				g_processinfo[process_index]->service_index = process_index;
				get_time( g_processinfo[process_index]->start_date );
				hzb_log_info( __FILE__ , __LINE__ , "新增成功 pid=[%ld]", g_processinfo[process_index]->pid );
				break;
			}
		}

		if( irc )
			break;
	}

	if( irc )
		strcpy( stBuf.buffer, "新增服务异常" );
	else
		strcpy( stBuf.buffer, "新增服务成功" );

SEND:
	sprintf( stBuf.buffer_len, "%d", strlen(stBuf.buffer) );		
	irc = sendn( sock, sizeof(stBuf.buffer_len)+strlen(stBuf.buffer), (char *)&stBuf, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn add_service fail" );
		return	irc;
	}
	hzb_log_info( __FILE__ , __LINE__ , "新增服务结束" );
	return	irc;
}


int stop_service( stDefsercfg *pstsercfg, int sock )
{
	int		process_index = 0;
	int		irc = 0;
	int		iflag = 0;
	stDefbuf	stBuf;

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	/* 根据配置档信息新增服务 */
	hzb_log_info( __FILE__ , __LINE__ , "停止服务开始" );
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid <= 0 )
			continue;
		
		if( strcmp( pstsercfg->arguments, "all" ) == 0 )
		{
			iflag = 1;	
		}
		else
		{
			if( strcmp( g_processinfo[process_index]->procfg.service_name, pstsercfg->service_name ) == 0 )
				iflag = 1;
			else
				iflag = 0;
		}

		if( iflag ) 
		{
			hzb_log_info( __FILE__ , __LINE__ , "服务序号[%d]", process_index );
					
			memset( g_processinfo[process_index]->procfg.cmd, 0x00, sizeof(g_processinfo[process_index]->procfg.cmd) );
			strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_STOP );
			irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
			if( irc == -1 )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write cmd error[%d], errno[%d]", irc, errno );
				break;
                        }
                        else if( irc != sizeof(stDefproccfg) )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write error[%d], errno[%d]", irc, errno );
				break;
                        }
			irc = 0;
			hzb_log_info( __FILE__ , __LINE__ , "停止成功 pid=[%ld]", g_processinfo[process_index]->pid );
		}
		iflag = 0;
	}
	
	if( irc )
		strcpy( stBuf.buffer, "发送关闭命令异常" );
	else
		strcpy( stBuf.buffer, "发送关闭命令成功" );
	
	sprintf( stBuf.buffer_len, "%d", strlen(stBuf.buffer) );		
	irc = sendn( sock, sizeof(stBuf.buffer_len)+strlen(stBuf.buffer), (char *)&stBuf, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn stop_service fail" );
		return	irc;
	}

	hzb_log_info( __FILE__ , __LINE__ , "停止服务结束" );
	return	irc;
}



int force_stop_service( stDefsercfg *pstsercfg, int sock )
{
	int		process_index = 0;
	int		irc = 0;
	int		iflag = 0;
	stDefbuf	stBuf;

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	/* 根据配置档信息停止服务 */
	hzb_log_info( __FILE__ , __LINE__ , "强制停止服务开始" );
	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid <= 0 )
			continue;
		
		if( strcmp( pstsercfg->arguments, "all" ) == 0 )
		{
			iflag = 1;	
		}
		else
		{
			if( strcmp( g_processinfo[process_index]->procfg.service_name, pstsercfg->service_name ) == 0 )
				iflag = 1;
			else
				iflag = 0;
		}

		if( iflag ) 
		{
			hzb_log_info( __FILE__ , __LINE__ , "服务序号[%d]", process_index );
					
			memset( g_processinfo[process_index]->procfg.cmd, 0x00, sizeof(g_processinfo[process_index]->procfg.cmd) );
			strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_FORCE );
			/*irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
			if( irc == -1 )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write cmd error[%d], errno[%d]", irc, errno );
				break;
                        }
                        else if( irc != sizeof(stDefproccfg) )
                        {
                                hzb_log_error( __FILE__ , __LINE__ , "write error[%d], errno[%d]", irc, errno );
				break;
                        }
			irc = 0;*/
			kill( g_processinfo[process_index]->pid, SIGKILL );
			hzb_log_info( __FILE__ , __LINE__ , "强制停止成功 pid=[%ld]", g_processinfo[process_index]->pid );
		}
		iflag = 0;
	}
	
	if( irc )
		strcpy( stBuf.buffer, "发送强制关闭命令异常" );
	else
		strcpy( stBuf.buffer, "发送强制关闭命令成功" );
	
	sprintf( stBuf.buffer_len, "%d", strlen(stBuf.buffer) );		
	irc = sendn( sock, sizeof(stBuf.buffer_len)+strlen(stBuf.buffer), (char *)&stBuf, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn stop_service fail" );
		return	irc;
	}

	hzb_log_info( __FILE__ , __LINE__ , "停止服务结束" );
	return	irc;
}







int mod_service( stDefsercfg *pstsercfg, int sock )
{
	int		process_index = 0;
	int		irc = 0;
	int		i = 0;
	int		ifound = 0;
	stDefbuf	stBuf;

	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );
	/* 根据配置档信息修改服务 */
	hzb_log_info( __FILE__ , __LINE__ , "重载服务开始" );

	for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
	{
		if( g_processinfo[process_index]->pid <= 0 )
			continue;

		if( strcmp( g_processinfo[process_index]->procfg.service_name, pstsercfg->service_name ) == 0 )
		{
			hzb_log_info( __FILE__ , __LINE__ , "服务序号[%d]", process_index );
					
			/* 清除原配置 */
			memset( &(g_processinfo[process_index]->procfg), 0x00, sizeof(g_processinfo[process_index]->procfg) );
			strcpy( g_processinfo[process_index]->procfg.cmd, SERVICE_CMD_RELOAD );

			cfg_to_proc( pstsercfg, &(g_processinfo[process_index]->procfg) );

			irc = write( g_processinfo[process_index]->command_pipes[1], (char*)&(g_processinfo[process_index]->procfg), sizeof(stDefproccfg) ) ;
			if( irc == -1 )
			{
				hzb_log_error( __FILE__ , __LINE__ , "write cmd error[%d], errno[%d]", irc, errno );
				break;
			}
			else if( irc != sizeof(stDefproccfg) )
			{
				hzb_log_error( __FILE__ , __LINE__ , "write error[%d], errno[%d]", irc, errno );
				break;
			}
			irc = 0;
			ifound = 1;
			hzb_log_info( __FILE__ , __LINE__ , "重载成功 pid=[%ld]", g_processinfo[process_index]->pid );
		}

	}

	if( irc )
		strcpy( stBuf.buffer, "发送重载命令失败" );
	else
		strcpy( stBuf.buffer, "发送重载命令成功" );

	if( !ifound )
	{
		memset( stBuf.buffer, 0x00, sizeof(stBuf.buffer) );	
		sprintf( stBuf.buffer, "未找到服务[%s]", pstsercfg->service_name );
	}
	
	sprintf( stBuf.buffer_len, "%d", strlen(stBuf.buffer) );		
	irc = sendn( sock, sizeof(stBuf.buffer_len)+strlen(stBuf.buffer), (char *)&stBuf, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn reload_service fail" );
		return	irc;
	}

	hzb_log_info( __FILE__ , __LINE__ , "重载服务结束" );
	return	irc;
}


int view_service( stDefsercfg *pstsercfg, int sock )
{
	int		process_index = 0;
	int		irc = 0;
	int		iflag = 0;
	int		icnt = 0;
	stDefbuf	stBuf;
	memset( (char *)&stBuf, 0x00, sizeof(stBuf) );

	/* 根据配置档信息查看服务 */
	hzb_log_info( __FILE__ , __LINE__ , "查看服务" );
	
	if( strcmp( pstsercfg->arguments, "status" ) == 0 )
	{
		hzb_log_info( __FILE__ , __LINE__ , "进程状态信息" );
		sprintf( stBuf.buffer+strlen(stBuf.buffer), "管理进程服务运行地址:[%s] \n", g_ip_addr );
		sprintf( stBuf.buffer+strlen(stBuf.buffer), "管理进程服务监听端口:[%d] \n", g_listen_port );
		
		for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
		{
			if( g_processinfo[process_index]->pid <= 0 )
				continue;
			icnt ++;		
		}
		sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务进程个数:[%d] \n", icnt );

	}
	else 
	{
		for( process_index = 0 ; process_index < MAX_PROCESSES ; process_index++ )
		{
			if( g_processinfo[process_index]->pid <= 0 )
				continue;

			if( strcmp( pstsercfg->arguments, "all" ) == 0 )
			{
				iflag = 1;	
			}
			else
			{
				if( strcmp( g_processinfo[process_index]->procfg.service_name, pstsercfg->service_name ) == 0 )
					iflag = 1;
				else
					iflag = 0;
			}

			if( iflag ) 
			{
				hzb_log_info( __FILE__ , __LINE__ , "服务序号[%d]", process_index );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "------------------------------------\n" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务序号         [%d] \n", g_processinfo[process_index]->service_index );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "进程号           [%ld] \n", g_processinfo[process_index]->pid );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务名称         [%s] \n", g_processinfo[process_index]->procfg.service_name );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "动态链接库名称   [%s] \n", g_processinfo[process_index]->procfg.so_name );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务函数名称     [%s] \n", g_processinfo[process_index]->procfg.service_func );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "进程启动时刻     [%s] \n", g_processinfo[process_index]->start_date );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "进程运行状态     [%s] \n", g_processinfo[process_index]->procfg.status == SERVICE_STATUS_IDLE ? "IDLE" : "RUNNING" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务运行时间     [%d] \n", g_processinfo[process_index]->procfg.work_time );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务间隔时间     [%d] \n", g_processinfo[process_index]->procfg.interval );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务已等待时间   [%d] \n", g_processinfo[process_index]->procfg.wait_time );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务运行次数     [%d] \n", g_processinfo[process_index]->procfg.cur_cnt );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务成功次数     [%d] \n", g_processinfo[process_index]->procfg.succ_cnt );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务平均耗时     [%lf] \n", g_processinfo[process_index]->procfg.succ_cnt != 0 ? g_processinfo[process_index]->procfg.spend_time/g_processinfo[process_index]->procfg.succ_cnt : 0 );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务最大运行次数 [%d] \n", g_processinfo[process_index]->procfg.max_cnt );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务超时时间     [%d] \n", g_processinfo[process_index]->procfg.timeout );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "服务触发参数:         \n" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发间隔         [%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_interval) > 0 ? g_processinfo[process_index]->procfg.trigger_interval : "NONE" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发间隔值       [%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_interval_value) > 0 ? g_processinfo[process_index]->procfg.trigger_interval_value : "NONE" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发类型         [%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_type) > 0 ? g_processinfo[process_index]->procfg.trigger_type : "NONE" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发开始时间     [%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_start_time) > 0 ? g_processinfo[process_index]->procfg.trigger_start_time : "NONE" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发结束时间     [%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_end_time) > 0 ? g_processinfo[process_index]->procfg.trigger_end_time : "NONE" );
				sprintf( stBuf.buffer+strlen(stBuf.buffer), "触发日期         [%s][%s][%s][%s][%s][%s][%s][%s][%s][%s] \n", strlen(g_processinfo[process_index]->procfg.trigger_date[0])>0 ? g_processinfo[process_index]->procfg.trigger_date[0] : "NONE", 
strlen(g_processinfo[process_index]->procfg.trigger_date[1])>0 ? g_processinfo[process_index]->procfg.trigger_date[1] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[2])>0 ? g_processinfo[process_index]->procfg.trigger_date[2] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[3])>0 ? g_processinfo[process_index]->procfg.trigger_date[3] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[4])>0 ? g_processinfo[process_index]->procfg.trigger_date[4] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[5])>0 ? g_processinfo[process_index]->procfg.trigger_date[5] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[6])>0 ? g_processinfo[process_index]->procfg.trigger_date[6] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[7])>0 ? g_processinfo[process_index]->procfg.trigger_date[7] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[8])>0 ? g_processinfo[process_index]->procfg.trigger_date[8] : "NONE",
strlen(g_processinfo[process_index]->procfg.trigger_date[9])>0 ? g_processinfo[process_index]->procfg.trigger_date[9] : "NONE" );
			}
			iflag = 0;
		}
	}


	hzb_log_info( __FILE__ , __LINE__ , "data len [%d]", strlen(stBuf.buffer) );
	hzb_log_info( __FILE__ , __LINE__ , "data \n%s", stBuf.buffer );

	sprintf( stBuf.buffer_len, "%d", strlen(stBuf.buffer) );		

	irc = sendn( sock, sizeof(stBuf.buffer_len)+strlen(stBuf.buffer), (char *)&stBuf, TIMEOUT );
	if ( irc )
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn view_service fail" );
		return	irc;
	}

	hzb_log_info( __FILE__ , __LINE__ , "查看服务结束" );
	return	irc;
}



int wait_time_increase( int sec )
{
        int i = 0 ;
        for( i = 0; i < MAX_PROCESSES; i++ )
        {
                if( g_processinfo[i]->pid > 0 )
                {
                        if( g_processinfo[i]->procfg.status == SERVICE_STATUS_IDLE )
                                g_processinfo[i]->procfg.wait_time += sec ;
                }
        }
}


int work_time_increase( int sec )
{
        int i = 0 ;
        for( i = 0; i < MAX_PROCESSES; i++ )
        {
                if( g_processinfo[i]->pid > 0 )
                {
                        if( g_processinfo[i]->procfg.status == SERVICE_STATUS_WORK )
                                g_processinfo[i]->procfg.work_time += sec ;
                }
        }
}






