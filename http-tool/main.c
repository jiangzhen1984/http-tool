// http-tool.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <string.h>
#include "tool.h"


static void printUsage()
{
}


static void load_url_list(char * f_path)
{
	FILE * fp = NULL;
	int count_url = 0;
	char buf[1000];

	memset(buf, 0, 1000);

	if(f_path == NULL || *f_path =='\0') {
		printf("[ERROR] url list path is empty\n");
		return;
	}
	fp = fopen(f_path, "r");
	if (fp == NULL)
	{
		printf(" [ERROR] can't open file:%s\n", f_path);
		return;
	}

	printf(" Start to prepare url cache.\n");
	while( fgets(buf, 1000, fp) != NULL)
	{
		count_url ++ ;
	}

	set_url_records(count_url);
	fseek(fp, 0, SEEK_SET);
	
	
	while( fgets(buf, 1000, fp) != NULL)
	{
		add_record(buf);
		memset(buf, 0, sizeof(buf));
	}

	fclose(fp);
	

	printf(" Finish preparing url cache.\n");
}

int main(int argc, char ** argv)
{	
	char * f_path = NULL;
	char * url = NULL;

	if(argc <3) {		
		printf("[ERROR] No enough parameters\n");
		printUsage();
		exit(1);
	}
	
	f_path = *++argv;
	if(f_path == NULL || *f_path =='\0')
	{
		printf("[ERROR] url list path is empty\n");
		exit(1);
	}

    start_counter_worker();
	load_url_list(f_path);
	
       set_source_url(*++argv);

	//TODO start sending
    start_test();

    start_shoot();
    //release_url_records();
    while(1)
    {
        sleep(0x7fffff);
    }
	
    return 0;
}

