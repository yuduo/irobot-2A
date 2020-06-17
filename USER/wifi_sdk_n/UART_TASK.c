#include "TASK_COM.h"

extern char key[];

uint8_t CommandValue=0xff;
char mac_getted = 0;
uint8_t machmac[15] = {0};
extern char uart_recv_data_flag;
extern char send_ok_flag;
extern char wifi_got_ip;
extern char wifi_got_OK;
extern char wifi_disconnect;
extern char server_connect;
extern char send_to_server_flag;
extern char send_ok_server;

uint8_t CommandProcess(uint8_t *str)
{
    if(strstr((char*)str,"CMD1")!=NULL) CommandValue=1;
    else if(strstr((char*)str,"CMD2")!=NULL) CommandValue=2;
    else if(strstr((char*)str,"ready")!=NULL) CommandValue=3;
    else if(strstr((char*)str,"MAC,\"")!=NULL) CommandValue=4;
    else if(strstr((char*)str,"+IPD")!=NULL) CommandValue=5;
    else if(strstr((char*)str,"SEND OK")!=NULL) CommandValue=6;
    else if(strstr((char*)str,"FI GOT I")!=NULL) CommandValue=7;
    else if(strstr((char*)str,"OK")!=NULL && wifi_got_ip == 1) CommandValue=8;
    else if(strstr((char*)str,"WIFI DISCONNECT")!=NULL) CommandValue=9;
    else if(strstr((char*)str,"CONNECT")!=NULL && send_to_server_flag == 1) CommandValue=10;
    else if(strstr((char*)str,"ERROR\r\nCLOSED")!=NULL && send_to_server_flag == 1) CommandValue=11;

    return CommandValue;
}

void LowerToCap(uint8_t *str,uint8_t len)
{
    uint8_t i;
    for(i=0; i<len; i++)
    {
        if((96<str[i])&&(str[i]<123))	//????
            str[i]=str[i]-32;				//?????
    }
}
extern void send_data_to_wifi(char port, char * data0, char *data1, char cryption);

uint8_t uart_task(void)
{
    uint8_t j,i;
    char * pos;

    char recv_cmd[200];

    uint8_t temp;

    uint16_t rx_len = 0;

    uint8_t ut_ret = 0;

    for(i=0; i<5; i++)
    {
        http_read_queue[i] = 0;
        http_write_port[i] = 0;
    }

        if(uart_recv_data_flag == 1)
        {
		rx_len = DMA_GetCurrDataCounter(DMA1_Channel3);
		log_printf("[uart_task]uart_recv_data_flag %d \r\n",rx_len);
		for(uint8_t iii = 0;iii < rx_len;iii++)
		{
			log_printf("[uart 3]%2X\r\n",USART_RX_BUF[iii]);
		}
		CommandValue=CommandProcess(USART_RX_BUF);		//命令解析

            if(CommandValue != 0xff && temp != CommandValue)
            {
			ut_ret = CommandValue;
                temp = CommandValue;
                if(CommandValue == 5)
                {
                    ////////////////////////////////////////////////////////
                    if(strstr((char*)USART_RX_BUF,"http-config.json") != NULL
                            && strstr((char*)USART_RX_BUF,"+IPD,") != NULL
                            && strstr((char*)USART_RX_BUF,"&data=") != NULL)
                    {

                        http_config_queue[0] = 0;

                        pos = strstr((char*)USART_RX_BUF,"&data=");
                        j = 0;
                        while(1)
                        {
                            recv_cmd[j] = *(pos+j+6);
                            if((recv_cmd[j]>='0' && recv_cmd[j]<= '9')
                                    || (recv_cmd[j]>='A' && recv_cmd[j]<= 'F')
									|| (recv_cmd[j]>='a' && recv_cmd[j]<= 'f'))
                                j++;
                            else break;
                        }
                        recv_cmd[j] = 0;
                        base16_decrypt(key,recv_cmd);
                        strcpy(http_config_queue, recv_cmd);
                        log_printf("%s\r\n",recv_cmd);

                        pos = strstr((char*)USART_RX_BUF,"+IPD,");
                        http_config_port = *(pos+5);
                        CommandValue = 0xff;
                        temp = 0xff;
                    }
                    ///////////////////////////////////////////////////////////
                    else if(strstr((char*)USART_RX_BUF,"http-read.json") != NULL
                            && strstr((char*)USART_RX_BUF,"+IPD,") != NULL)
                    {
                        pos = strstr((char*)USART_RX_BUF,"+IPD,");

                        for(j=0; j<5; j++)
                        {
                            if(http_read_queue[j] == 0)
                            {
                                http_read_queue[j] = *(pos+5);
                                break;
                            }
                        }

                        CommandValue = 0xff;
                        temp = 0xff;
                    }
					else if(strstr((char*)USART_RX_BUF,"timesync") != NULL)
					{
						for(i=0; i<2; i++)
                        {
                            if(http_write_port[i] == 0)
                            {
								http_write_queue0[0] = 0;
                                strcpy(http_write_queue0, (char*)USART_RX_BUF);
								http_write_port[i] = 3;
								break;
							}
						}
					}
                    ////////////////////////////////////////////////////////
                    else if(strstr((char*)USART_RX_BUF,"http-write.json") != NULL
                            //&& strstr((char*)USART_RX_BUF,"+IPD,") != NULL
                            && strstr((char*)USART_RX_BUF,"&data=") != NULL)
                    {

                        for(i=0; i<2; i++)
                        {
                            if(http_write_port[i] == 0)
                            {
                                pos = strstr((char*)USART_RX_BUF,"&data=");
                                j = 0;
                                while(1)
                                {
                                    recv_cmd[j] = *(pos+j+6);
                                    if((recv_cmd[j]>='0' && recv_cmd[j]<= '9')
                                            || (recv_cmd[j]>='A' && recv_cmd[j]<= 'F')
											|| (recv_cmd[j]>='a' && recv_cmd[j]<= 'f'))
                                        j++;
                                    else break;
                                }
                                recv_cmd[j] = 0;
                                base16_decrypt(key,recv_cmd);

                                switch(i)
                                {
                                case 0:
                                    http_write_queue0[0] = 0;
                                    strcpy(http_write_queue0, recv_cmd);
                                    break;
                                case 1:
                                    http_write_queue1[0] = 0;
                                    strcpy(http_write_queue1, recv_cmd);
                                    break;
                                }
                                pos = strstr((char*)USART_RX_BUF,"+IPD,");
                                http_write_port[i] = *(pos+5);
                                break;
                            }
                        }

                        CommandValue = 0xff;
                        temp = 0xff;
                    }
					else 
					{
						pos = strstr((char*)USART_RX_BUF,"+IPD,");
						send_data_to_wifi(*(pos+5)-0x30,RESPONSE_HEAD_NOENCRYPTED_STRING,
                                  "{\"response\":\"ERROR\"}",0);
						CommandValue = 0xff;
                        temp = 0xff;
					}
                }
                if(CommandValue == 4)
                {
                    j = 0;
                    pos = strstr((char*)USART_RX_BUF,"MAC,\"");

                    machmac[j++] = *(pos+1+4);
                    machmac[j++] = *(pos+2+4);

                    machmac[j++] = *(pos+4+4);
                    machmac[j++] = *(pos+5+4);

                    machmac[j++] = *(pos+7+4);
                    machmac[j++] = *(pos+8+4);

                    machmac[j++] = *(pos+10+4);
                    machmac[j++] = *(pos+11+4);

                    machmac[j++] = *(pos+13+4);
                    machmac[j++] = *(pos+14+4);

                    machmac[j++] = *(pos+16+4);
                    machmac[j++] = *(pos+17+4);

                    machmac[j++] = 0;

                    LowerToCap(machmac,j-1);
                    log_printf("MAC:%s\r\n",machmac);
                    mac_getted = 1;
                }
                if(CommandValue == 6)
                {
                    send_ok_flag = 1;
                    send_ok_server = 1;
                    CommandValue = 0xff;
                    temp = 0xff;
                }
                if(CommandValue == 7)
                {
                    wifi_got_ip = 1;
                }
                if(CommandValue == 8)
                {
                    wifi_got_OK = 1;
                }
                if(CommandValue == 9)
                {
                    wifi_disconnect = 1;
                }
                if(CommandValue == 10)
                {
                    server_connect = 1;
                }
                if(CommandValue == 11)
                {
                    server_connect = 5;
                }
            }
            uart_recv_data_flag = 0;
            ////USART_RX_STA=0;
            DMA_SetCurrDataCounter(DMA1_Channel3 , 0);
            memset(USART_RX_BUF,0,USART_REC_LEN);			//串口接收缓冲区清零
        }

	return ut_ret;
        //myfree(CommandStr);								//释放内存
        ////wifi_delay_ms(10);
}

