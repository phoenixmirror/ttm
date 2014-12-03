#include "ttm.h"

void usage()
{
	printf( "USAGE : \n" );
	printf( "                   ttm start | reload | close | view | stop \n" );
	printf( "����:\n");
	printf( "����:              ttm start \n" );
	printf( "��������:          ttm reload �������õ����� \n" );
	printf( "���ӷ���:          ttm add    �������õ����� \n" );
	printf( "�ر�ָ������:      ttm close  �������õ����� \n" );
	printf( "�ر�ȫ������:      ttm close  all \n" );
	printf( "ǿ�ƹر�ָ������:  ttm force  �������õ����� \n" );
	printf( "ǿ�ƹر�ȫ������:  ttm force  all \n" );
	printf( "�鿴ָ������:      ttm view   �������õ����� \n" );
	printf( "�鿴ȫ������:      ttm view   all \n" );
	printf( "�鿴����״̬:      ttm view   status \n" );
	printf( "ֹͣ:              ttm stop \n" ); 
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

	/* �������� */
	if (strcmp(*argv, "start") == 0) 
	{
		argv++; argc--;
		start_process(argc, argv);
	}
	else if (strcmp(*argv, "add") == 0) 
	{
		/* �������� */
		argv++; argc--;
		add_process(argc, argv);
	} 
	else if (strcmp(*argv, "reload") == 0) 
	{
		/* ���ط��� */
		argv++; argc--;
		mod_process(argc, argv);
	}
	else if (strcmp(*argv, "close") == 0) 
	{
		/* �رշ��� */
		argv++; argc--;
		stop_process(argc, argv);
	} 
	else if (strcmp(*argv, "force") == 0) 
	{
		/* �رշ��� */
		argv++; argc--;
		force_stop_process(argc, argv);
	} 
	else if (strcmp(*argv, "view") == 0) 
	{
		/* �鿴���� */
		argv++; argc--;
		view_process(argc, argv);
	} 
	else if (strcmp(*argv, "stop") == 0) 
	{
		/* ֹͣ���� */
		argv++; argc--;
		close_process(argc, argv);
	} 
	else 
	{
		printf("��Ч�Ĳ���:%s\n", *argv);
		usage();
	}
	
	return 0;
	
}



