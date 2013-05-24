// **********************************************************************
//
// Copyright (c) 2003-2010 THP, Inc. All rights reserved.
//
// This copy of THP is licensed to you under the terms described in the
// THP_LICENSE file included in this distribution.
//
// **********************************************************************


#include "tool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef LINUX
    #include <netdb.h>
    #include <errno.h>
#elif WIN32
#endif


#define SERVPORT 80
URL_RECORD * g_head = NULL;

static int g_count = 0;

static int current_idx = 0; 

static void create_socket(URL_RECORD * pR, struct hostent *host);

int set_url_records(int count)
{
	
		release_url_records();

	   g_head = (URL_RECORD *) malloc(sizeof(URL_RECORD) * count);
	   memset(g_head, 0, sizeof(URL_RECORD) * count);
	   	if(g_head == NULL)
		{
			return 1;
		}		

	g_count = count;
	
	return 0;
}


void release_url_records()
{
	if (g_head != NULL)
	{
        URL_RECORD * head = g_head;
		while(g_count-- > 0)
		{
			URL_RECORD *  p_record = head++;
			if(  p_record  != NULL)
			{
				if( p_record->host_name != NULL)
				{
					free(p_record->host_name);
                    p_record->host_name = NULL;
				}
				if( p_record->send_url != NULL)
				{
					free(p_record->send_url);
                    p_record->send_url = NULL;
				}
				if( p_record->dst_url != NULL)
				{
					free( p_record->dst_url);
                    p_record->dst_url = NULL;
				}
				if( p_record->context != NULL)
				{
					free(p_record->context);
                    p_record->context = NULL;
				}

                if(p_record->fd>=0)
                {
                    close(p_record->fd);
                    p_record->fd = -1;
                }
			}
		}
        free(g_head);
	}
	g_head = NULL;
	g_count = 0;
	current_idx = 0;
}

void add_record(char * record)
{
	if (record == NULL || *record =='\0')
	{
		return;
	}

	if(current_idx >= g_count)
	{
		printf("[ERROR] can't add new reocrd, because over cache size.\n");
		return;
	}

    char * send_url = NULL;
    char * dst_url = NULL;
	char * tmp = record;
    int start_pos = 0;
    int end_pos = 1;
    int len = 0;
	while( *tmp++ != '\0')
	{
        switch(*tmp)
        {
            case ' ':
               if(end_pos - start_pos > 5)
               {
                    len = end_pos - start_pos +2;
                    char * url = (char *) malloc(len);
                    memcpy(url, &record[start_pos],len);
                    url[len-1] ='\0';
                    send_url = url;
                     
               }
               start_pos = end_pos;
               break;
            case '\n':
            {
                len = end_pos - start_pos +1;
                if(len > 5) {
                char * url = (char *) malloc(len);
                memcpy(url, &record[start_pos],len-2);
                url[len-1] ='\0';
                dst_url = url;
                }
                break;
            }
                
        }
        
        end_pos ++;
	}

	g_head[current_idx].send_url = send_url;
    // TODO get hostname
	g_head[current_idx].dst_url = dst_url;

    char  url[255];
    memset(url, 0, 255);
    memcpy(url, send_url, strlen(send_url));
    char * pch = NULL;
    pch = strstr(url, "http://");

    if(pch !=  NULL && *pch !='\0')
    {
        pch += 7;
        char * phstart = pch ;
        pch = strstr(pch, "/");
        *pch ='\0';
        int len = strlen(phstart);
        char * hostname = (char * ) malloc(len+1);
        memset(hostname, 0, len+1);
        memcpy(hostname, phstart, len);
        hostname[len] ='\0';
        
	    g_head[current_idx].host_name = hostname;

        *pch='/';
        len = strlen(pch);
        char * context = (char * )malloc(len +1);
        memset(context, 0, len+1);
        memcpy(context, pch+1, len-1);
        context[len] = '\0';
	    g_head[current_idx].context = context;
    }


    current_idx++;
}

// this function run understand pthread
static void * examine_host(URL_RECORD * pR)
{
    if (pR == NULL || pR->host_name == NULL || *(pR->host_name) =='\0')
    {
        printf("[ERROR] incorrect record\n");
        return NULL;
    }
    
    if (pR->is_tested == TESTED || pR->is_tested == TESTING)
    {
        return NULL;
    }
    pR->is_tested = TESTING;

#ifdef LINUX
    struct hostent *host;
    host =  gethostbyname(pR->host_name);
//TODO calculate cost
    if( host == NULL)
    {
        printf(" test result:%d\n", errno);
    }
    else
    {
        //TODO create socket
        create_socket(pR, host);
    }
#endif
    pR->is_tested = TESTED;
    
    return NULL;
}


void start_test()
{
   int c = g_count; 
    while(c-->0)
    {
       int i = 1;
        for(;i%20 != 0; i++,c--)
        {
            pthread_t tid;
            pthread_create(&tid, NULL, examine_host,(void *) &g_head[c]);
        } 
#ifdef LINUX
        sleep(2);
#endif
    }
}



static void create_socket(URL_RECORD * pR, struct hostent *host)
{
    if(pR == NULL || host == NULL)
    {
        printf("[ERROR] incorrect parameter\n");
        return;
    }
    int sockfd;
#ifdef LINUX
    struct sockaddr_in serv_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
        printf("can't crreate socket error :%d\n", errno);
        pR->is_opened = NOT_OPENED;
        pR->is_avl = UNAVAILABLE;
        return;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVPORT);

    serv_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
//    const char *ip = inet_ntoa(serv_addr.sin_addr);
 //   printf("connect to %s  ", ip);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, \
               sizeof(struct sockaddr)) == -1) {
       printf("can't connect to url:%d\n",errno);
       return;
    }

    printf("saved socd %d  ", sockfd);
    pR->fd = sockfd;
    pR->is_opened = OPENED;
    pR->is_avl = AVAILABLE;

#endif
}
