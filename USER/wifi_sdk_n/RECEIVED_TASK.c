#include "TASK_COM.h"

extern char key[];
char cryption_flag = 0;
char send_ok_flag = 0;
extern char send_to_server_flag;
char server_connect = 0;

void send_data_to_wifi(char port, char * data0, char *data1, char cryption);

void send_data_to_server(char * str);


void send_server_process(char * str);

void response_success(char port, char * response_str);

extern int online_timeout;

extern struct Data_Time timer;

extern char online;		//BUTTON_TASK有定义

char time_get_flag,last_mms;		//BUTTON_TASK有定义

void process_time_str(char * databuf);

char time_sync = 0;

void received_task(void)
{
    int i,j;

    char response_str[1024] = {0};
    char data_buf[20];

    char * pos;

    cryption_flag = 1;
/*
    for(i=0; i<5; i++)
    {
        http_read_queue[i] = 0;
        http_config_port = 0;
    }
	http_write_port[0] = 0;
	http_write_port[1] = 0;
*/

        //////////read queue/////////
        for(i=0; i<5; i++)
        {
            if(http_read_queue[i] != 0)
            {
                online = 1;
                online_timeout = 0;
                string_process(response_str);
		if(cryption_flag == 1)
                {
                    send_data_to_wifi(http_read_queue[i]-0x30,RESPONSE_HEAD_ENCRYPTED_STRING,
                                      response_str,1);
                }

                http_read_queue[i] = 0;

            }
        }
        //////////config queue/////////
        if(http_config_port != 0)
        {
            online = 1;
            online_timeout = 0;
            wifi_printf("recv：%s end",http_config_queue);
            log_printf("recv：%s end",http_config_queue);
            if(strstr((char*)http_config_queue,"SSID")!=NULL &&
                    strstr((char*)http_config_queue,"PASSWORD")!=NULL &&
                    strstr((char*)http_config_queue,"ENCRYPT_KEY")!=NULL)
            {
                pos = strstr((char*)http_config_queue,"SSID");
                for(i=0; i<40; i++)
                {
                    WIFI_SSID[i] = *(pos + 5 + i);
                    if(WIFI_SSID[i] == '&')
                    {
                        WIFI_SSID[i] = 0;
                        break;
                    }
                }

                pos = strstr((char*)http_config_queue,"PASSWORD");
                for(i=0; i<40; i++)
                {
                    WIFI_PWD[i] = *(pos + 9 + i);
                    if(WIFI_PWD[i] == '&')
                    {
                        WIFI_PWD[i] = 0;
                        break;
                    }
                }

                pos = strstr((char*)http_config_queue,"ENCRYPT_KEY");
                for(i=0; i<16; i++)
                {
                    WIFI_KEY[i] = *(pos + 12 + i);
                }
				WIFI_KEY[16] = 0;
                key[0] = 0;
                strcpy(key,WIFI_KEY);

                response_success(http_config_port-0x30,response_str);

                wifi_delay_ms(1000);

                AP_STA_mode = STA_MODE_ON;
            }
            else {
                send_data_to_wifi(http_config_port-0x30,RESPONSE_HEAD_NOENCRYPTED_STRING,
                                  "{\"response\":\"ERROR\"}",0);
            }

            http_config_port = 0;

        }
        //////////write queue/////////
        for(j=0; j<2; j++)
        {
            if(http_write_port[j] != 0)
            {
                online = 1;
                online_timeout = 0;
                switch(j)
                {
                case 0:
                    wifi_printf("write : %s\r\n",http_write_queue0);
                    response_str[0] = 0;
                    strcpy(response_str,http_write_queue0);
                    break;
                case 1:
                    wifi_printf("write : %s\r\n",http_write_queue1);
                    response_str[0] = 0;
                    strcpy(response_str,http_write_queue1);
                    break;
                }

				if(strstr((char*)response_str,"TimeSync")!=NULL || strstr((char*)response_str,"timesync")!=NULL)
				{
					if(strstr((char*)response_str,"TimeSync")!=NULL)
						pos = strstr((char*)response_str,"TimeSync");
					if(strstr((char*)response_str,"timesync")!=NULL)
					{
						pos = strstr((char*)response_str,"timesync");
						time_sync = 2;
					}

					i = 0;
					for(i=0; i<20; i++)
					{
						data_buf[i] = 0;
					}
					for(i=0; i<20; i++)
					{
						data_buf[i] = *(pos + 9 + i);
						if((data_buf[i] >= '0' && data_buf[i] <= '9')
								|| data_buf[i] == '/' ||data_buf[i] == '_'
								|| data_buf[i] == ':')
						{
						}
						else break;
					}
					
					if(strlen(data_buf) == 16 || strlen(data_buf) == 17 )
					{
						response_success(http_write_port[j]-0x30,response_str);
						while(time_get_flag == 1 ||time_get_flag == 2);
						time_get_flag = 3;
						//time set
						process_time_str(data_buf);
						////Time_Update(timer.w_year,timer.w_month,timer.w_date,
						////			timer.hour,timer.min,timer.sec);
						time_get_flag = 0;
						if(time_sync == 2)
							time_sync = 1;
					}
					else
					{
						
						send_data_to_wifi(http_write_port[j]-0x30,RESPONSE_HEAD_NOENCRYPTED_STRING,
										  "{\"response\":\"ERROR\"}",0);
					}
					http_write_port[j] = 0;
					response_str[0] = 0;
				} else if(strstr((char*)response_str,"LockSt=0")!=NULL && online == 1)
				{
					lockSt = UNLOCK;
					last_mms = 33;
					response_success(http_write_port[j]-0x30,response_str);
				}
				else if(strstr((char*)response_str,"LockSt=1")!=NULL && online == 1)
				{
					lockSt = LOCK;
					response_success(http_write_port[j]-0x30,response_str);
				}
				http_write_port[j] = 0;
				wifi_delay_ms(1000);
			}

        }
        if(send_to_server_flag == 1)
        {
            send_data_to_server(response_str);
        }
        wifi_delay_ms(10);
}

char send_success_flag = 0;
void response_success(char port , char * response_str)
{
	send_success_flag = 0;
    response_str[0] = 0;
    strcpy(response_str,"{\"response\":\"SUCCESS\"}");

    send_data_to_wifi(port,RESPONSE_HEAD_NOENCRYPTED_STRING,
                      response_str,1);
	send_success_flag = 1;

}

void send_data_to_wifi(char port, char * data0, char *data1, char cryption)
{
    int temp;
    int j;

    temp = strlen(data0);

    if(cryption == 0)
        temp += strlen(data1);
    else if(cryption == 1)
    {
        temp += 2 * strlen(data1);
    }

    send_ok_flag = 0;

    wifi_printf("AT+CIPSEND=%d,%d\r\n",port,temp);

    wifi_delay_ms(500);

    wifi_printf("%s",data0);

    if(cryption == 0)
    {
        wifi_printf("%s",data1);
    }
    else if(cryption == 1)
    {
        base16_encryption(key,data1);
    }

    for(j=0; j<40; j++)
    {
        wifi_delay_ms(100);
        if(send_ok_flag == 1)
            break;
    }

    wifi_printf("AT+CIPCLOSE=%d\r\n",port);
}

extern uint8_t machmac[];
void send_data_to_server(char * str)
{
    int i;
    i = 0;
    wifi_printf("AT+CIPCLOSE=5\r\n");
    wifi_delay_ms(500);
    wifi_printf("AT+CIPSERVER=0\r\n");
    wifi_delay_ms(500);
    wifi_printf("AT+CIPMUX=0\r\n");
    wifi_delay_ms(500);
    server_connect = 0;
    wifi_printf("AT+CIPSTART=\"TCP\",\"%s\",80,1\r\n",
           SERVER_ADDRESS);
    for(i=0; i<100; i++)
    {
        wifi_delay_ms(100);
        if(server_connect == 1 || server_connect == 5)
            break;
    }
    if(i < 100 && server_connect == 1)
    {
        send_server_process(str);
    }

    send_to_server_flag = 0;

    for(i=0; i<5; i++)
    {
        http_read_queue[i] = 0;
        http_config_port = 0;
        http_write_port[0] = 0;http_write_port[1] = 0;
    }
	
}
char send_ok_server = 0;
void send_server_process(char * str)
{
    int temp;
    int i;

    string_send_to_server(str);

    temp = 0;
    temp = strlen(SEND_TO_SERVER_HEAD_STRING);
    temp += strlen(SEND_TO_SERVER_FILE_STRING);
    temp += strlen("encrypted=1");
    temp += strlen("&macAddress=");
    temp += strlen((const char *)machmac);
    temp += strlen("&data=");
	temp += strlen(SEND_SERVER_END);
    if(cryption_flag == 1)
        temp += 2*strlen(str);
    else 
	temp += strlen(str);

    wifi_printf("AT+CIPSEND=%d\r\n",temp);
    wifi_delay_ms(500);
    send_ok_server = 0;

    if(cryption_flag == 1)
    {
        wifi_printf("%s%s%s%s%s%s",SEND_TO_SERVER_HEAD_STRING,
                   SEND_TO_SERVER_FILE_STRING,"encrypted=1","&macAddress=",machmac,
                   "&data=");
        base16_encryption(key,str);
		wifi_printf("%s",SEND_SERVER_END);
    }

    for(i=0; i<60; i++)
    {
        wifi_delay_ms(100);
        if(send_ok_server == 1)
            break;
    }
}

void process_time_str(char * data_buf)
{
/*
    int i;
    char data_buf1[10];

    for(i=0; i<4; i++)
    {
        data_buf1[i] = data_buf[i];
    }
    data_buf1[4] = 0;
    timer.w_year = atol(data_buf1);

    for(i=0; i<2; i++)
    {
        data_buf1[i] = data_buf[i+5];
    }
    data_buf1[2] = 0;
    timer.w_month = atol(data_buf1);

    for(i=0; i<2; i++)
    {
        data_buf1[i] = data_buf[i+8];
    }
    data_buf1[2] = 0;
    timer.w_date = atol(data_buf1);

    for(i=0; i<2; i++)
    {
        data_buf1[i] = data_buf[i+11];
    }
    data_buf1[2] = 0;
    timer.hour = atol(data_buf1);

    for(i=0; i<2; i++)
    {
        data_buf1[i] = data_buf[i+14];
    }
    data_buf1[2] = 0;
    timer.min = atol(data_buf1);

    timer.sec = 0;
 */
}

