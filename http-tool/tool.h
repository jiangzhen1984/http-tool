// **********************************************************************
//
// Copyright (c) 2003-2010 THP, Inc. All rights reserved.
//
// This copy of THP is licensed to you under the terms described in the
// THP_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef H_TOOL_H
#define H_TOOL_H


#define THREAD_COUNT 4

#define RECORDS_PER_THREAD 50

typedef enum {
    NOT_OPENED,
    OPENED, 
} SOCK_STATE;

typedef enum {
    UNAVAILABLE,
    AVAILABLE,
    NO_SUCH_CONTEXT,
} HOST_STATE;

typedef enum {
    NOT_START,
    TESTING,
    TESTED
} TEST_STATE;


typedef struct {
    int fd;
    unsigned char  is_opened;
    unsigned char is_avl;
    unsigned char is_tested;
    char * host_name;
    char * context;
    char * send_url;
    char * dst_url;
    char * host;
} URL_RECORD;




int set_url_records(int count);

void set_source_url(char * source);


void release_url_records();

void add_record(char * record);



void start_test();


void start_shoot();



#endif
