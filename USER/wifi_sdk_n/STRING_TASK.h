#ifndef __STRING_TASK_H_
#define __STRING_TASK_H_

#include "TASK_COM.h"

void string_task(void *pvParameters);

void string_process(char * response_str);

void string_send_to_server(char * response_str);


#endif
