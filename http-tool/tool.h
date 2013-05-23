// **********************************************************************
//
// Copyright (c) 2003-2010 THP, Inc. All rights reserved.
//
// This copy of THP is licensed to you under the terms described in the
// THP_LICENSE file included in this distribution.
//
// **********************************************************************



typedef struct {
	int fd;
	int is_opened;
	char * host_name;
	
	char * send_url;
	
	char * dst_url;

} URL_RECORD;


int set_url_records(int count);


void release_url_records();

void add_record(char * record);



