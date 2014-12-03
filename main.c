#include "ttm.h"

void usage()
{
	printf( "USAGE : \n" );
	printf( "                   ttm start | reload | close | view | stop \n" );
	printf( "范例:\n");
	printf( "启动:              ttm start \n" );
	printf( "重载配置:          ttm reload 服务配置档名称 \n" );
	printf( "增加服务:          ttm add    服务配置档名称 \n" );
	printf( "关闭指定服务:      ttm close  服务配置档名称 \n" );
	printf( "关闭全部服务:      ttm close  all \n" );
	printf( "强制关闭指定服务:  ttm force  服务配置档名称 \n" );
	printf( "强制关闭全部服务:  ttm force  all \n" );
	printf( "查看指定服务:      ttm view   服务配置档名称 \n" );
	printf( "查看全部服务:      ttm view   all \n" );
	printf( "查看管理状态:      ttm view   status \n" );
	printf( "停止:              ttm stop \n" ); 
	return ;	
}


int main( int argc , char *argv[] )
{
	
	if( argc == 1 )
	{
		usage();
		exit(0);
	}
	
	argv++; argc--;

	/* 启动服务 */
	if (strcmp(*argv, "start") == 0) 
	{
		argv++; argc--;
		start_process(argc, argv);
	}
	else if (strcmp(*argv, "add") == 0) 
	{
		/* 新增服务 */
		argv++; argc--;
		add_process(argc, argv);
	} 
	else if (strcmp(*argv, "reload") == 0) 
	{
		/* 重载服务 */
		argv++; argc--;
		mod_process(argc, argv);
	}
	else if (strcmp(*argv, "close") == 0) 
	{
		/* 关闭服务 */
		argv++; argc--;
		stop_process(argc, argv);
	} 
	else if (strcmp(*argv, "force") == 0) 
	{
		/* 关闭服务 */
		argv++; argc--;
		force_stop_process(argc, argv);
	} 
	else if (strcmp(*argv, "view") == 0) 
	{
		/* 查看服务 */
		argv++; argc--;
		view_process(argc, argv);
	} 
	else if (strcmp(*argv, "stop") == 0) 
	{
		/* 停止服务 */
		argv++; argc--;
		close_process(argc, argv);
	} 
	else 
	{
		printf("无效的参数:%s\n", *argv);
		usage();
	}
	
	return 0;
	
}



