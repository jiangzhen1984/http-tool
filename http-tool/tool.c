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

URL_RECORD * g_head = NULL;

static int g_count = 0;

static int current_idx = 0;

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
		while(g_count-- > 0)
		{
			URL_RECORD *  p_record = g_head++;
			if(  p_record  != NULL)
			{
				if( p_record ->host_name != NULL)
				{
					free( p_record ->host_name);
				}
				free( p_record );
			}
		}
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

	char * tmp = record;
	int idx = 0;
	int send_flag  = 0;
	int dst_flag  = 0;
	while( *tmp++ != '\0')
	{
	
		
	}

	g_head[current_idx];
}