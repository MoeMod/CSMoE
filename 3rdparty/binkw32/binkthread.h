// Copyright Epic Games, Inc. All Rights Reserved.
// Licensed under the terms of a valid Unreal Engine license agreement,
//   and the separate 'Unreal Engine End User License Agreement for Publishing'.

struct thread_data;
struct queue_data;

int RAD_start_thread( int thread_num );

int RAD_send_to_client( int thread_num, void * data, unsigned int length );
int RAD_receive_at_client( int thread_num, int us, void * out_data, unsigned int length );

int RAD_send_to_host( int thread_num, void * data, unsigned int length );
int RAD_receive_at_host( int thread_num, int us, void * out_data, unsigned int length );

int RAD_blip_for_host( int thread_num );
void RAD_wait_for_host_room( int thread_num );
int RAD_stop_thread( int thread_num );
int RAD_wait_stop_thread( int thread_num );
int RAD_platform_info( int thread_num, void * output );
unsigned int RAD_get_client_empty( int thread_num );

void RAD_async_main( int thread_num );

