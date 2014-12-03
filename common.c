#include "ttm.h"

xmlChar *findDefaultNamespaceHref( const xmlDocPtr doc )
{
	xmlNodePtr pXmlNode = NULL;
	if( doc == NULL )
		return(NULL);
	pXmlNode = xmlDocGetRootElement(doc);

	return((xmlChar *)pXmlNode->ns->href);
}

void getDataByXpath( xmlXPathContextPtr xpathCtx, xmlChar * xmlPath, char* pcDataBuf, int iBufSize )
{
	xmlXPathObjectPtr xpathObj;
	int size = 0;

	xpathObj = xmlXPathEvalExpression(xmlPath, xpathCtx);
	if( xmlXPathNodeSetIsEmpty(xpathObj->nodesetval) )
	{
		hzb_log_info( __FILE__ , __LINE__ , "xpath=[%s],获取节点信息为空", xmlPath );
		xmlXPathFreeObject(xpathObj);
		return;
	}
	size = (xpathObj != NULL && xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
	if (size > 0) 
	{
		if( xpathObj->nodesetval->nodeTab[0]->children != NULL)
			strcpy( pcDataBuf, (char*)xpathObj->nodesetval->nodeTab[0]->children->content );
	}
	xmlXPathFreeObject(xpathObj);
	return;
}

int check_num( char *number )
{
	int	i = 0;
	int	inum = 0;
	int	irc = 0;
	if( strlen(number) <= 0 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "字符串长度小于0" );
		return	-1;
	}
	
	for( i = 0; i < strlen(number); i++ ) 
	{
		irc = isdigit(number[i]);
		if( irc == 0 ) 
		{
			hzb_log_error( __FILE__ , __LINE__ , "[%s],字符串中含有非数字", number );
			return	-1;
		}	
	}
	inum = atoi(number);
	return inum; 
}


int get_service_cfg( char *cfg, stDefsercfg *pstsercfg )
{
	int	irc = 0;
	xmlChar	*ns_DefNamespce = NULL;
	xmlDocPtr	doc;
	xmlXPathContextPtr	xpathCtx;
	struct stat filestat;
	char	filename[256];
	char	buffer[1024];
	
	hzb_log_info( __FILE__ , __LINE__ , "parse cfg xml [%s]", cfg );
	memset( filename, 0x00, sizeof(filename) );
	memset( buffer, 0x00, sizeof(buffer) );
	memset( (char*)&filestat, 0x00, sizeof(struct stat) );
	
	sprintf( filename, "%s/etc/%s", (char*)(getenv("HOME")), cfg );
	irc = stat( filename, &filestat );
	if( irc != 0 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "[%s] file is not exsit", filename );
		return	-1;
	}
	
	doc = xmlParseFile( filename );
	if( doc == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "unable to parse [%s]", filename );
		return	-1;
	}
	
	xpathCtx = xmlXPathNewContext( doc );
	if( xpathCtx == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "unable to create new XPath context" );
		return	-1;
	}

	ns_DefNamespce = findDefaultNamespaceHref( doc );
	if (ns_DefNamespce != NULL) 
	{
		if( xmlXPathRegisterNs(xpathCtx, BAD_CAST "c", ns_DefNamespce) != 0 ) 
		{
			hzb_log_error( __FILE__ , __LINE__ , "unable to create new XPath context" );
			return	-1;
		}
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:service_name", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->service_name, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "service_name=[%s]", pstsercfg->service_name );
	}
	else
	{
		printf("service_name不能为空\n");
		return -1;
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:so_name", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->so_name, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "so_name=[%s]", pstsercfg->so_name );
	}
	else
	{
		printf("so_name不能为空\n");
		return -1;
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:service_func", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->service_func, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "service_func=[%s]", pstsercfg->service_func );
	}
	else
	{
		printf("service_func不能为空\n");
		return -1;
	}

	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:arguments", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->arguments, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "arguments=[%s]", pstsercfg->arguments );
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:trigger_interval", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		if( strcmp( buffer, TRIGGER_INTERVAL_DAY ) != 0 && 
			strcmp( buffer, TRIGGER_INTERVAL_WEEK ) != 0 )
		{
			hzb_log_error( __FILE__ , __LINE__ , "trigger_interval值不符合规定[%s]", buffer );
			printf("trigger_interval值不符合规定[%s]", buffer);
			return	-1;
		}
		strcpy( pstsercfg->trigger_interval, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_interval=[%s]", pstsercfg->trigger_interval );
	}

	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:trigger_interval_value", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->trigger_interval_value, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_interval_value=[%s]", pstsercfg->trigger_interval_value );
	}
	else
	{
		if( strlen( pstsercfg->trigger_interval ) > 0 )
		{
			hzb_log_error( __FILE__ , __LINE__ , "trigger_interval有值 trigger_interval_value不能为空" );
			printf("trigger_interval有值 trigger_interval_value不能为空");
			return	-1;
		}
	}

	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:trigger_type", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		if( strcmp( buffer, TRIGGER_TYPE_TIME ) != 0  &&
			strcmp( buffer, TRIGGER_TYPE_CLOCK ) != 0 )
		{
			hzb_log_error( __FILE__ , __LINE__ , "trigger_type值不符合规定[%s]", buffer );
			printf("trigger_type值不符合规定[%s]", buffer);
			return	-1;
		}
		strcpy( pstsercfg->trigger_type, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_type=[%s]", pstsercfg->trigger_type );
	}

	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:trigger_start_time", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->trigger_start_time, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_start_time=[%s]", pstsercfg->trigger_start_time );
	}
	else
	{
		if( strlen( pstsercfg->trigger_type ) > 0 )
		{
			hzb_log_error( __FILE__ , __LINE__ , "trigger_type有值 trigger_start_time不能为空" );
			printf("trigger_type有值, trigger_start_time不能为空");
			return	-1;
		}
	}

	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:trigger_end_time", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->trigger_end_time, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_end_time=[%s]", pstsercfg->trigger_end_time );
	}
	else
	{
		if( strcmp( pstsercfg->trigger_type, TRIGGER_TYPE_TIME ) == 0 )
		{
			hzb_log_error( __FILE__ , __LINE__ , "trigger_type值是时间区间 trigger_end_time不能为空");
			printf("trigger_type值是时间区间 trigger_end_time不能为空");
			return	-1;
		}
	}
		
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:hostname", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		strcpy( pstsercfg->hostname, buffer );
		hzb_log_info( __FILE__ , __LINE__ , "hostname=[%s]", pstsercfg->hostname );
	}
	else
	{
		printf("主机名不能为空\n");
		return	-1;
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:service_number", buffer, sizeof(buffer) - 1);
	irc = check_num(buffer);
	if( irc <= 0 )
	{
		printf("服务个数不能小于0\n");
		return	-1;
	}
	else
	{
		pstsercfg->service_number = irc;
		hzb_log_info( __FILE__ , __LINE__ , "service_number=[%d]", pstsercfg->service_number );	
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:interval", buffer, sizeof(buffer) - 1);
	irc = check_num(buffer);
	if( irc <= 0 )
	{
		printf("服务运行间隔时间不能小于0\n");
		return	-1;
	}
	else
	{
		pstsercfg->interval = irc;
		hzb_log_info( __FILE__ , __LINE__ , "interval=[%d]", pstsercfg->interval );	
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:timeout", buffer, sizeof(buffer) - 1);
	irc = check_num(buffer);
	if( irc <= 0 )
	{
		printf("服务运行超时时间不能小于0\n");
		return	-1;
	}
	else
	{
		pstsercfg->timeout = irc;
		hzb_log_info( __FILE__ , __LINE__ , "timeout=[%d]", pstsercfg->timeout );	
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:max_cnt", buffer, sizeof(buffer) - 1);
	irc = check_num(buffer);
	if( irc <= 0 )
	{
		printf("服务运行最大次数不能小于0\n");
		return	-1;
	}
	else
	{
		pstsercfg->max_cnt = irc;
		hzb_log_info( __FILE__ , __LINE__ , "max_cnt=[%d]", pstsercfg->max_cnt );	
	}

	xmlXPathFreeContext( xpathCtx );
	xmlFreeDoc( doc );
	xmlCleanupParser();
	return	0;
}


int check_service_cfg( stDefsercfg *pstsercfg )
{
	int	irc = 0;
	struct	stat filestat;
	char	filename[256];
	char	hostname[50];
        struct	IB2Env  *penv = NULL  ;
	OBJECTHANDLE	*phandle = NULL  ;
	
	memset( filename, 0x00, sizeof(filename) );
	memset( (char*)&filestat, 0x00, sizeof(struct stat) );
	
	sprintf( filename, "%s/modules/%s", (char*)(getenv("HOME")), pstsercfg->so_name );
	irc = stat( filename, &filestat );
	if( irc != 0 )
	{
		printf( "[%s]找不到",filename );
		hzb_log_error( __FILE__ , __LINE__ , "[%s] so file is not exsit", filename );
		return	-1;
	}

	phandle = OpenLinkLibrary( filename ) ;
        if( phandle == NULL )
        {
                hzb_log_error(__FILE__, __LINE__, "dlerror()[%s]\n" , dlerror() ) ;
                hzb_log_error(__FILE__, __LINE__, "打开业务处理组件文件[%s]失败\n" , filename ) ;
		printf( "打开动态链接库[%s]失败\n", filename );
		return	-1;
        }
	CloseLinkLibrary( phandle ) ;

	memset( hostname, 0x00, sizeof(hostname) );
	gethostname( hostname, sizeof(hostname) );
	if( strcmp( hostname, pstsercfg->hostname ) != 0 )
	{
		printf( "配置档主机名[%s]与机器主机名[%s]不一致\n", pstsercfg->hostname, hostname );
		hzb_log_error( __FILE__ , __LINE__ , "[%s][%s] hostname diff", hostname, pstsercfg->hostname );
		return	-1;
	}


	/* 检查IB2配置等 */
        irc = IB2AllocEnvironment( &penv ) ;
        if ( irc != 0 )
        {       
                hzb_log_error(__FILE__, __LINE__, "分配内存空间错误 IB2AllocEnvironment irc=%d", irc) ;
		printf( "分配IB2环境变量错误\n" );
		return	-1;
        }
        
        /* 加载配置文件 */ 
        irc = IB2LoadClientConfig( penv , NULL ) ;
        if ( irc != 0 )
        {
                hzb_log_error(__FILE__, __LINE__, "加载配置文件错误 IB2LoadClientConfig irc=%d", irc) ;
		printf( "加载IB2配置文件错误\n" );
		return	-1;
        }
                
        /* 打开数据库 */
        irc = BusinessDataBaseOpen() ;
        if ( irc )
        {       
		hzb_log_error(__FILE__, __LINE__, "开启数据库失败, irc=%d", irc) ;
		printf( "数据库连接失败\n" );
		return	-1;
        }
	/* 关闭数据库 */
        BusinessDataBaseClose() ;
        /* 释放内存空间 */
        IB2FreeEnvironment( &penv ) ;

	return	0;
}


void cfg_to_proc( stDefsercfg *pstsercfg, stDefproccfg *pstprocfg )
{
	char	*p = NULL;
	char	buffer[20];
	char	curdate[20];
	int	i = 0;

	strcpy( pstprocfg->service_name, pstsercfg->service_name );
	strcpy( pstprocfg->so_name, pstsercfg->so_name );
	strcpy( pstprocfg->service_func, pstsercfg->service_func );
	strcpy( pstprocfg->arguments, pstsercfg->arguments );
	strcpy( pstprocfg->trigger_interval, pstsercfg->trigger_interval );
	strcpy( pstprocfg->trigger_interval_value, pstsercfg->trigger_interval_value );
	strcpy( pstprocfg->trigger_type, pstsercfg->trigger_type );
	strcpy( pstprocfg->trigger_start_time, pstsercfg->trigger_start_time );
	strcpy( pstprocfg->trigger_end_time, pstsercfg->trigger_end_time );
	pstprocfg->service_number = pstsercfg->service_number;
	pstprocfg->interval = pstsercfg->interval;
	pstprocfg->timeout = pstsercfg->timeout;
	pstprocfg->max_cnt = pstsercfg->max_cnt;

	memset( curdate, 0x00, sizeof(curdate) );
	GetCurrentSystemDate( curdate );


	if( strcmp( pstsercfg->trigger_type, TRIGGER_TYPE_CLOCK ) ==  0 )
	{
		if( strlen( pstsercfg->trigger_start_time ) > 0 )
		{
			p = strtok( pstsercfg->trigger_start_time, " " );
			while( p )
			{
				memset( buffer, 0x00, sizeof(buffer) );
				strcpy( buffer, p );
				trimSpace( buffer );			
				if( strlen( buffer ) > 0 )
					strcpy( pstprocfg->time_buff[i], buffer );
				hzb_log_info(__FILE__, __LINE__, "缓存触发时刻 time_buffer[%d]=[%s]", i, buffer ) ;
				strcpy( pstprocfg->trigger_date[i], curdate );
				i++;
				p = strtok( NULL, " " );	
			}
		}
	}

}



int QueryIpByHostName( char *hostname , char *ip , long ip_bufsize )
{
	struct hostent	*phe = NULL ;
	char	**ip_addr = NULL , *ip_ptr = NULL ;
	
	phe = gethostbyname( hostname ) ;
	if( phe == NULL )
		return -1;
	
	switch( phe->h_addrtype )
	{
		case AF_INET :
		case AF_INET6 :
			ip_addr = phe->h_addr_list ;
#ifdef inet_ntop
			inet_ntop( phe->h_addrtype, *ip_addr , ip , ip_bufsize );
#else
			ip_ptr = inet_ntoa( *(struct in_addr*)(*ip_addr) ) ;
			if( (long)strlen(ip_ptr) > ip_bufsize - 1 )
				return -2;
			else
				strcpy( ip , ip_ptr );
#endif
			
		break;
		default :
			return -3;
	}

	return 0;
}


int get_sock_cfg( char *ip, int *port )
{
	int	irc = 0;
	struct stat filestat;
	char	filename[256];
	char	buffer[1024];
	xmlChar	*ns_DefNamespce = NULL;
	xmlDocPtr	doc;
	xmlXPathContextPtr	xpathCtx;
	
	hzb_log_info( __FILE__ , __LINE__ , "parse host cfg" );
	memset( filename, 0x00, sizeof(filename) );
	memset( buffer, 0x00, sizeof(buffer) );
	memset( (char*)&filestat, 0x00, sizeof(struct stat) );
	
	sprintf( filename, "%s/etc/node.ttm", (char*)(getenv("HOME")) );
	irc = stat( filename, &filestat );
	if( irc != 0 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "[%s] is not exsit", filename );
		return	-1;
	}
	
	doc = xmlParseFile( filename );
	if( doc == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "unable to parse [%s]", filename );
		return	-1;
	}
	
	xpathCtx = xmlXPathNewContext( doc );
	if( xpathCtx == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "unable to create new XPath context" );
		return	-1;
	}

	ns_DefNamespce = findDefaultNamespaceHref( doc );
	if (ns_DefNamespce != NULL) 
	{
		if( xmlXPathRegisterNs(xpathCtx, BAD_CAST "c", ns_DefNamespce) != 0 ) 
		{
			hzb_log_error( __FILE__ , __LINE__ , "unable to create new XPath context" );
			return	-1;
		}
	}
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:ip", buffer, sizeof(buffer) - 1);
	if( strlen(buffer) > 0 )
	{
		hzb_log_info( __FILE__ , __LINE__ , "ip=[%s]", buffer );
	}
	else
	{
		printf("IP不能为空\n");
		return -1;
	}
	
	irc = QueryIpByHostName( buffer, ip, 30 );
	if( irc != 0 )
	{
		if( irc == -1 )
		{
			strcpy( ip, buffer );
		}
	}
	hzb_log_info( __FILE__ , __LINE__ , "ip=[%s]", ip );
	
	memset( buffer, 0x00, sizeof(buffer) );
	getDataByXpath(xpathCtx, BAD_CAST "/c:hzbank/c:ttm/c:port", buffer, sizeof(buffer) - 1);
	irc = check_num(buffer);
	if( irc <= 0 )
	{
		printf("端口不能小于0\n");
		return	-1;
	}
	else
	{
		*port = irc;
		hzb_log_info( __FILE__ , __LINE__ , "port=[%d]", *port );	
		irc = 0;
	}

	xmlXPathFreeContext( xpathCtx );
	xmlFreeDoc( doc );
	xmlCleanupParser();
	return	irc;
}



int socket_listen( char *ip, int port, int *listen_sock )
{
	int	irc = 0;
	struct sockaddr_in	inaddr ;
	/* 创建侦听socket */
	*listen_sock = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP ) ;
	if( *listen_sock == -1 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket failed, errno[%d]" , errno );
		return -1;
	}
	int reuse_addr = 1;
	setsockopt( *listen_sock , SOL_SOCKET , SO_REUSEADDR , (void *) & reuse_addr , sizeof(reuse_addr) );

	memset( &inaddr, 0x00, sizeof(struct sockaddr_in) );
	inaddr.sin_family = AF_INET ;

	if( inet_addr( ip ) == -1 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "inet_addr error [%u]" ,inet_addr( ip ) );
		printf( "主机名或IP错误\n" );
		return	-1;
	}

	inaddr.sin_addr.s_addr = inet_addr( ip ) ;
	inaddr.sin_port = htons( (unsigned short)port ) ;
	irc = bind( *listen_sock, (struct sockaddr *) &inaddr, sizeof(struct sockaddr) ) ;
	if( irc == -1 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "bind failed ip[%s] port[%ld], errno[%d]" ,ip, port, errno );
		return -1;
	}
	
	irc = listen( *listen_sock, 5 ) ;
	if( irc == -1 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "listen failed , errno[%d]" , errno );
		return -1;
	}	
	
	return	irc;	
}


int socket_connect( char *ip, int port, int *connect_sock )
{
	int	irc = 0;
	struct sockaddr_in stRmtAddr;
	
	*connect_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( *connect_sock < 0 )
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket failed , errno[%d][%s]" , errno, strerror(errno) );
		return	-1;
	}
	
	int nodelay=1;
	if( setsockopt( *connect_sock, IPPROTO_TCP, TCP_NODELAY, (void *)&nodelay, sizeof(nodelay)) ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "setsockop failed , errno[%d][%s]" , errno, strerror(errno) );
		return	-1;
	}
	memset( &stRmtAddr, 0x00, sizeof(struct sockaddr_in) );
	stRmtAddr.sin_family = AF_INET;
	stRmtAddr.sin_port = htons((unsigned short) port);
	stRmtAddr.sin_addr.s_addr = inet_addr(ip);

	hzb_log_info( __FILE__ , __LINE__ , "socket connect server ip[%s],port[%d]",ip,port );
	if( connect( *connect_sock, (struct sockaddr*) &stRmtAddr, sizeof(stRmtAddr)) ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "socket connect failed , errno[%d][%s]" , errno, strerror(errno) );
		return(-1);
	}
	hzb_log_info( __FILE__ , __LINE__ , "socket connect server succ" );
	
	return	irc;
}


int sendn( int sock, const int len, char *buffer, int timeout )
{
	int	irc = 0; 
	int	sndlen = 0; 
	int	onetimelen = 0;
	long	starttime = 0;
	long	curtime = 0;
	struct timeval stTimeout;
	static fd_set fdset;
	
	hzb_log_info( __FILE__ , __LINE__ , "sendn start" );
	if( sock <= 0 || len <= 0 || buffer == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "sendn parameter error" );
		return	-1;
	}
	
	time( &starttime );
	while( len - sndlen )
	{
		onetimelen = ( BUFFER_LEN <= (len - sndlen) ? BUFFER_LEN : (len - sndlen) );

		time( &curtime );
		if( (curtime-starttime) > timeout )
		{
			hzb_log_error( __FILE__ , __LINE__ , "sendn timeout" );
			return	-1;
		}

		FD_ZERO( &fdset );
		FD_SET( sock, &fdset );

		stTimeout.tv_sec = timeout-(curtime-starttime);
		stTimeout.tv_usec = 0;	
		switch( ( irc = select(sock+1, NULL, &fdset, NULL, &stTimeout) ) ) 
		{	

			case 1:
				if ( ( irc = send( sock, &buffer[sndlen], onetimelen, 0 ) ) < 0 ) 
				{
					irc = errno;
					hzb_log_error( __FILE__ , __LINE__ , "socket send error = %d(%s)", errno,strerror(errno) );
					return irc;
				}
				hzb_log_info( __FILE__ , __LINE__ , "socket send succ, send len=[%d]", onetimelen );
				/*hzb_hexlog_info( __FILE__ , __LINE__ , &buffer[sndlen], onetimelen );*/
				sndlen += irc;
				irc = 0;
				break;

			case 0: /* timeout */
				hzb_log_error( __FILE__ , __LINE__ , "socket send timeout" );
				return	-1;

			default: /* unknow error */
				hzb_log_error( __FILE__ , __LINE__ , "socket send unknowerr" );
				return	-1;
		}
			
				
	}
	hzb_log_info( __FILE__ , __LINE__ , "sendn end" );
	return	irc;
}



int recvn( int sock, int len, char *data, int timeout )
{
	struct timeval stTimeout;
	static fd_set fdset;
	int	irc = 0;
	int	recvlen = 0; 
	int	onetimelen = 0;
	int	recvflag = 0;
	long	starttime = 0;
	long	curtime = 0;
	char	buffer[4096];
	memset( buffer, 0x00, sizeof(buffer) );

	if( sock <= 0 || len <= 0 || data == (char *) 0 ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "recvn parameter error" );
		return	-1;
	}
	hzb_log_info( __FILE__ , __LINE__ , "recvn start" );
	recvflag = 1;
	time( &starttime );
	while( recvflag ) 
	{
		time( &curtime );
		if( (curtime - starttime) > timeout )
		{
			hzb_log_error( __FILE__ , __LINE__ , "recvn timeout" );
			return	-1;
		}
			
		FD_ZERO( &fdset );
		FD_SET( sock, &fdset );
		stTimeout.tv_sec = timeout-(curtime-starttime);
		stTimeout.tv_usec = 0;	

		switch( ( irc = select(sock+1, &fdset, NULL, NULL, &stTimeout) ) ) 
		{
			case 1:
				memset( buffer, 0x00, sizeof(buffer) );
				onetimelen = ( sizeof(buffer) > (len - recvlen) ? (len - recvlen) : sizeof(buffer) );
				irc = recv( sock, buffer, onetimelen, 0 );
				if( irc < 0 ) 
				{
					irc = errno;
					hzb_log_error( __FILE__ , __LINE__ , "socket recv() error = %d(%s)" , errno, strerror(errno) );
					recvflag = 0;
				} 
				else if( irc == 0 ) 
				{ 
					hzb_log_error( __FILE__ , __LINE__ , "socket remote peer disconnect" );
					irc = -1;
					recvflag = 0;
				} 
				else 
				{ 
					hzb_log_info( __FILE__ , __LINE__ , "socket recv data succ" );
					/*hzb_hexlog_info( __FILE__ , __LINE__ , buffer, onetimelen );*/
					memcpy( &data[recvlen], buffer, irc );
					recvlen += irc;
					irc = 0;
					if( len != recvlen ) 
						hzb_log_info( __FILE__ , __LINE__ , "continue to recv data,recved=[%d],letflen=[%d]", recvlen, len-recvlen );
					else 
						recvflag = 0;
				}
				break;
			case 0:	/* timeout */
				if( recvlen == 0 ) 
				{
					hzb_log_error( __FILE__ , __LINE__ , "socket recv timeout" );
					irc = -1;
				}
				recvflag = 0;
				break;
			default: /* unknow error */
				hzb_log_error( __FILE__ , __LINE__ , "socket recv unknowerr" );
				irc = -1;
				recvflag = 0;
		}
	}

	hzb_log_info( __FILE__ , __LINE__ , "recvn end" );
	return	irc;
}




int writen( int sock, char *buffer, int len, int timeout )
{
	int	irc = 0; 
	int	sndlen = 0; 
	int	onetimelen = 0;
	
	if( sock <= 0 || len <= 0 || buffer == NULL ) 
	{
		hzb_log_error( __FILE__ , __LINE__ , "writen parameter error" );
		return	-1;
	}
	
	while( len - sndlen )
	{
		onetimelen = ( BUFFER_LEN <= (len - sndlen) ? BUFFER_LEN : (len - sndlen) );

		if ( ( irc = write( sock, &buffer[sndlen], onetimelen ) ) < 0 ) 
		{
			irc = errno;
			hzb_log_error( __FILE__ , __LINE__ , "write error = %d(%s)", errno,strerror(errno) );
			return irc;
		}
		sndlen += irc;
		irc = 0;
				
	}
	return	irc;
}




int readn( int sock, char *data, int len, int timeout )
{
	int	irc = 0;
	int	recvlen = 0; 
	int	onetimelen = 0;
	int	recvflag = 0;
	char	buffer[4096];
	memset( buffer, 0x00, sizeof(buffer) );

	recvflag = 1;
	while( recvflag ) 
	{
		memset( buffer, 0x00, sizeof(buffer) );
		onetimelen = ( sizeof(buffer) > (len - recvlen) ? (len - recvlen) : sizeof(buffer) );
		irc = read( sock, buffer, onetimelen );
		if( irc < 0 ) 
		{
			irc = errno;
			hzb_log_error( __FILE__ , __LINE__ , "read error = %d(%s)" , errno, strerror(errno) );
			recvflag = 0;
		} 
		else if( irc == 0 ) 
		{ 
			hzb_log_error( __FILE__ , __LINE__ , "read 0" );
			irc = -1;
			recvflag = 0;
		} 
		else 
		{ 
			memcpy( &data[recvlen], buffer, irc );
			recvlen += irc;
			irc = 0;
			if( len == recvlen ) 
				recvflag = 0;
		}
	}

	return	irc;
}


int get_time( char *buf )
{
	char	date[40];
	char	time[40];
	struct timeval tv;
	struct timezone tz;
	struct tm *pstTmPtr;
	
	memset( date, 0x00, sizeof(date) );
	memset( time, 0x00, sizeof(time) );

	gettimeofday(&tv, &tz);
	pstTmPtr = localtime(&tv.tv_sec);
	sprintf( date, "%.04d%.2d%.2d", pstTmPtr->tm_year + 1900, pstTmPtr->tm_mon+1, pstTmPtr->tm_mday );
	sprintf( time, "%.2d%.2d%.2d", pstTmPtr->tm_hour, pstTmPtr->tm_min, pstTmPtr->tm_sec );
	sprintf( buf, "%.04d-%.2d-%.2dT%.2d:%.2d:%.2d",
		pstTmPtr->tm_year + 1900, pstTmPtr->tm_mon+1, pstTmPtr->tm_mday,
		pstTmPtr->tm_hour, pstTmPtr->tm_min, pstTmPtr->tm_sec);
	return	0;
}


int get_week()
{
	int week = 0;
	time_t _time_;
	struct tm *local_t;

	time( &_time_ );
	local_t = localtime( &_time_ );

	week =  local_t->tm_wday;
	hzb_log_info( __FILE__ , __LINE__ , "当日是星期[%d]", week );

	return	week;
}

int get_tdate( char *tdate )
{
        time_t _time_;
        struct tm *local_t;

	if( strlen(tdate) > 0 )
		memset( tdate, 0x00, strlen(tdate) );

        time( &_time_ );

        _time_ = _time_ + 86400;

        local_t = localtime( &_time_ );

        sprintf( tdate, "%04d-%02d-%02d", local_t->tm_year+1900, local_t->tm_mon+1,local_t->tm_mday );
	hzb_log_info( __FILE__ , __LINE__ , "下一天日期[%s]", tdate );

        return 0;
}



int check_trigger_time( stDefproccfg *pstprocfg )
{
	int	irc = 0;
	int	icnt = 0;
	char	buffer[100];
	char	nowtime[100];
	char	curdate[20];
	char	tdate[20];

	memset( curdate, 0x00, sizeof(curdate) );
	GetCurrentSystemDate( curdate );

	/* 判断trigger_type */
	if( strlen( pstprocfg->trigger_type ) > 0 )
	{
		if( strcmp( pstprocfg->trigger_type, TRIGGER_TYPE_CLOCK ) == 0 )
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "触发条件:时刻[%s]", pstprocfg->trigger_start_time );*/
			for( icnt = 0; icnt > BUFFER_NUM; icnt++ )			
			{
				if( strlen( pstprocfg->time_buff[icnt] ) > 0 )
					hzb_log_info( __FILE__ , __LINE__ , "缓存中的触发时刻[%s] 触发日期[%s]", pstprocfg->time_buff[icnt], pstprocfg->trigger_date[icnt] );
			}

			memset( buffer, 0x00, sizeof(buffer) );
			get_time( buffer );
			memset( nowtime, 0x00, sizeof(nowtime) );
			memcpy( nowtime, &buffer[11], 8 );

			for( icnt = 0; icnt < BUFFER_NUM; icnt++ )
			{
				if( strlen( pstprocfg->time_buff[icnt] ) <= 0 )
					continue;
				if( strcmp( nowtime, pstprocfg->time_buff[icnt] ) >= 0 && 
					strcmp( pstprocfg->trigger_date[icnt], curdate ) == 0 )
				{
					hzb_log_info( __FILE__ , __LINE__ , "符合触发条件 时刻[%s] 日期[%s]", nowtime, curdate );
					/* 获取下一日日期 */
					memset( tdate, 0x00, sizeof(tdate) );
					get_tdate( tdate );
					memset( pstprocfg->trigger_date[icnt], 0x00, sizeof(pstprocfg->trigger_date[icnt]) );
					strcpy( pstprocfg->trigger_date[icnt], tdate );
					return	0;
				}
			}
			/*hzb_log_info( __FILE__ , __LINE__ , "当前时刻[%s] 触发条件不满足[%s]", nowtime, pstprocfg->trigger_start_time );*/
			return	-1;
		}
		else if( strcmp( pstprocfg->trigger_type, TRIGGER_TYPE_TIME ) == 0 )
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "触发条件:开始时间[%s] 结束时间[%s]", pstprocfg->trigger_start_time, pstprocfg->trigger_end_time );*/
			memset( buffer, 0x00, sizeof(buffer) );
			get_time( buffer );
			memset( nowtime, 0x00, sizeof(nowtime) );
			memcpy( nowtime, &buffer[11], 8 );
			if( strcmp( nowtime,  pstprocfg->trigger_start_time ) >= 0 &&
				strcmp( nowtime, pstprocfg->trigger_end_time ) <= 0
			)
			{
				hzb_log_info( __FILE__ , __LINE__ , "触发条件:开始时间[%s] 结束时间[%s]", pstprocfg->trigger_start_time, pstprocfg->trigger_end_time );
				hzb_log_info( __FILE__ , __LINE__ , "符合触发条件:当前时间[%s]", nowtime );
				return	0;
			}
			else
			{
				/*hzb_log_error( __FILE__ , __LINE__ , "当前时间[%s] 触发条件不满足[%s][%s]", nowtime, pstprocfg->trigger_start_time, pstprocfg->trigger_end_time );*/
				return	-1;
			}
		}
		else
		{
			hzb_log_info( __FILE__ , __LINE__ , "触发条件错误 [%s]", pstprocfg->trigger_type );
			return	-1;
		}
	}

	return	irc;
}



int check_trigger_interval( stDefproccfg *pstprocfg )
{
	int	irc = 0;
	int	week = 0;
	char	buffer[100];
	char	nowdate[10];

	/* 判断trigger_interval */
	if( strlen( pstprocfg->trigger_interval ) > 0 )
	{
		/*hzb_log_info( __FILE__ , __LINE__ , "trigger_interval = [%s]", pstprocfg->trigger_interval );
		hzb_log_info( __FILE__ , __LINE__ , "trigger_interval_value = [%s]", pstprocfg->trigger_interval_value );*/
		if( strcmp( pstprocfg->trigger_interval, TRIGGER_INTERVAL_WEEK ) == 0 )			
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "触发条件:星期[%s]", pstprocfg->trigger_interval_value );*/
			week = get_week();
			if( week != atoi(pstprocfg->trigger_interval_value) )
			{
				/*hzb_log_error( __FILE__ , __LINE__ , "触发条件不满足 当天星期[%d],[%s]", week, pstprocfg->trigger_interval_value );*/
				return	-1;
			}
			else
			{
				irc = check_trigger_time( pstprocfg );
				if( irc )	
				{
					/*hzb_log_error( __FILE__ , __LINE__ , "触发条件不满足" );*/
					return	-1;
				}
				else
					hzb_log_info( __FILE__ , __LINE__ , "触发条件:星期[%s]", pstprocfg->trigger_interval_value );
					
			}
		}
		else if( strcmp( pstprocfg->trigger_interval, TRIGGER_INTERVAL_DAY ) == 0 )
		{
			/*hzb_log_info( __FILE__ , __LINE__ , "触发条件:日期天数[%s]", pstprocfg->trigger_interval_value );*/
			memset( buffer, 0x00, sizeof(buffer) );
			get_time( buffer );
			memset( nowdate, 0x00, sizeof(nowdate) );
			memcpy( nowdate, &buffer[8], 2 );
			if( atoi(nowdate) != atoi(pstprocfg->trigger_interval_value) )
			{
				/*hzb_log_error( __FILE__ , __LINE__ , "触发条件不满足 当前日期天数[%s] [%s]", nowdate, pstprocfg->trigger_interval_value );*/
				return	-1;
			}
			else
			{
				/*hzb_log_info( __FILE__ , __LINE__ , "符合当前日期天数[%s]", nowdate );*/
				irc = check_trigger_time( pstprocfg );
				if( irc )	
				{
					/*hzb_log_error( __FILE__ , __LINE__ , "触发条件不满足" );*/
					return	-1;
				}
				else
				{
					hzb_log_info( __FILE__ , __LINE__ , "触发条件:日期天数[%s]", pstprocfg->trigger_interval_value );
					hzb_log_info( __FILE__ , __LINE__ , "符合当前日期天数[%s]", nowdate );
				}

			}
		}
		else
		{
			hzb_log_info( __FILE__ , __LINE__ , "触发条件错误 [%s]", pstprocfg->trigger_interval );
			return	-1;
		}
	}
	else
	{
		irc = check_trigger_time( pstprocfg );
		if( irc )	
		{
			/*hzb_log_error( __FILE__ , __LINE__ , "触发条件不满足" );*/
			return	-1;
		}
	}	

	return	irc;
}


void trimTailSpace(char *pcData)
{
	int iLen;

	for(iLen=strlen(pcData)-1;iLen>=0;iLen--)
	{
		if(pcData[iLen]!=' ' && pcData[iLen]!='\0' && pcData[iLen]!='\n' && pcData[iLen]!='\r' && pcData[iLen]!='\t')
		{
			break;
		}
		else
		{
			pcData[iLen] = '\0';
		}
	}
}

void trimNewLine(char *pcData)
{
	int iLen;

	for(iLen=strlen(pcData)-1;iLen>=0;iLen--)
	{
		if(pcData[iLen]!='\0' && pcData[iLen]!='\n' && pcData[iLen]!='\r' && pcData[iLen]!='\t')
		{
			break;
		}
		else
		{
			pcData[iLen] = '\0';
		}
	}
}


/*
 * 移除字段开头半角空白、TAB、换行符('\r','\n')。
 * pcData(I/O)
 */
void trimHeaderSpace(char *pcData)
{
	int iLen,i=0,iBufLen=0;
	char *pcBuf = NULL;

	iLen=strlen(pcData);

	if(iLen > 0)
	{
		pcBuf = (char *)malloc(iLen);
		memset(pcBuf, 0x00, iLen);

		for(i=0;i < iLen ;i++)
		{
			if(pcData[i] !=' ' && pcData[i]!='\0' && pcData[i]!='\n' && pcData[i]!='\r' && pcData[i]!='\t')
			{
				iBufLen=i;
				break;
			}
		}

		if (iBufLen > 0)
		{
			memcpy(pcBuf,&pcData[iBufLen],iLen-iBufLen);
			memset(pcData,0x00,iLen);
			memcpy(pcData,pcBuf,iLen-iBufLen);
		}

		free(pcBuf);
	}
}


/*
 * 移除字段开头及末尾的半角空白、TAB、换行符('\r','\n')。
 * pcData(I/O)
 */
void trimSpace( char *pcData )
{
	trimHeaderSpace(pcData);
	trimTailSpace(pcData);
}





