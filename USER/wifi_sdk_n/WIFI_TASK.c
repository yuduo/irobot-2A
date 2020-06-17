#include "TASK_COM.h"

extern char mac_getted;
extern uint8_t machmac[];
extern char key[],start_rtc_cali;
char wifi_got_ip = 0;
char wifi_got_OK = 0;
int online_timeout = 0;
char wifi_disconnect = 0;
char send_to_server_flag = 0;

static char wifi_mode_retry = 0;
static char temp = 0;

void wifi_mode_task(void)
{
	int i;
	char mac_buf4[5];
	char buf[20];
	char buf1[20];
	char length = 0;
	online_timeout = 0;

	if(AP_STA_mode == STA_MODE_ON)
	{
		if(online_timeout >= SERVER_TIMEOUT*1000) //35s timeout
		{
			online = 0;
			send_to_server_flag = 1;
			while(send_to_server_flag != 0);

			wifi_printf("AT+CIPCLOSE\r\n");
			wifi_delay_ms(1000);

			wifi_printf("%s\r\n",WIFI_TCP_SET_CIPMUX_STR);
			wifi_delay_ms(1000);
			wifi_printf("%s\r\n",WIFI_TCP_SERVER_OPEN_80_STR);
			wifi_delay_ms(1000);

			wifi_printf("%s\r\n",WIFI_SEND_GET_STA_MAC_ADD_STR);
			wifi_delay_ms(1000);

			wifi_disconnect = 0;
			online_timeout = 0;
			send_to_server_flag = 0;
		}
	}

	if((temp == AP_STA_mode) && (0 == wifi_disconnect) && (0 == wifi_mode_retry))
		return;

	////while(eeprom_data_update_status != SUCCESS);

	if(AP_STA_mode == AP_MODE_OFF || AP_STA_mode == STA_MODE_OFF)
	{
		wifi_delay_ms(500);
		wifi_printf("AT+RESTORE\r\n"); 
		wifi_delay_ms(500);
		if(AP_STA_mode == AP_MODE_OFF)
			temp = AP_MODE_OFF;
		else
			temp = STA_MODE_OFF;
	}
	else if((AP_STA_mode == AP_MODE_ON) || (AP_MODE_ON == wifi_mode_retry))
	{
		temp = AP_MODE_ON;
		if(AP_MODE_ON == wifi_mode_retry)
			wifi_mode_retry = 0;
		//配置为AP模式，获取mac地址并存储，设置WiFi名称
		wifi_delay_ms(200);
		log_printf("AT+RESTORE\r\n");
		wifi_printf("AT+RESTORE\r\n");
		wifi_delay_ms(1500);
		log_printf("ATE0\r\n");
		wifi_printf("ATE0\r\n");
		wifi_delay_ms(200);
		log_printf("%s\r\n",WIFI_SET_DEF_A_P_MODE_STR);
		wifi_printf("%s\r\n",WIFI_SET_DEF_A_P_MODE_STR);
		wifi_delay_ms(200);
		log_printf("%s\r\n",WIFI_SEND_GET_STA_MAC_ADD_STR);
		wifi_printf("%s\r\n",WIFI_SEND_GET_STA_MAC_ADD_STR);
		wifi_delay_ms(1000);
		if(mac_getted == 0)
		{
			wifi_mode_retry = AP_MODE_ON;
			return;
		}
		log_printf("AT+CIPAP=\"192.168.0.1\"\r\n");
		wifi_printf("AT+CIPAP=\"192.168.0.1\"\r\n");
		wifi_delay_ms(500);

		mac_buf4[0] = machmac[8];
		mac_buf4[1] = machmac[9];
		mac_buf4[2] = machmac[10];
		mac_buf4[3] = machmac[11];
		mac_buf4[4] = 0;

		length = strlen(WIFI_SEND_AP_SSID_STR);
		buf[0] = WIFI_SEND_AP_SSID_STR[length-4];
		buf[1] = WIFI_SEND_AP_SSID_STR[length-3];
		buf[2] = WIFI_SEND_AP_SSID_STR[length-2];
		buf[3] = WIFI_SEND_AP_SSID_STR[length-1];
		buf[4] = 0;
		strcat(buf,mac_buf4);
		buf1[0] = 0;
		strcpy(buf1,buf);
		strcat(buf,buf1);
		key[0] = 0;
		strcpy(key,buf);
		log_printf("%s%s%s%s\r\n",WIFI_SEND_AP_CMD_STR,WIFI_SEND_AP_SSID_STR,
		mac_buf4,WIFI_SEND_AP_PWD_STR);
		wifi_printf("%s%s%s%s\r\n",WIFI_SEND_AP_CMD_STR,WIFI_SEND_AP_SSID_STR,
		mac_buf4,WIFI_SEND_AP_PWD_STR);

		wifi_delay_ms(200);
		log_printf("encryption key:%s\r\n",key);
		wifi_printf("encryption key:%s\r\n",key);

		wifi_delay_ms(500);
		log_printf("%s\r\n",WIFI_TCP_SET_CIPMUX_STR);
		wifi_printf("%s\r\n",WIFI_TCP_SET_CIPMUX_STR);
		wifi_delay_ms(500);
		log_printf("%s\r\n",WIFI_TCP_SERVER_OPEN_80_STR);
		wifi_printf("%s\r\n",WIFI_TCP_SERVER_OPEN_80_STR);
		wifi_delay_ms(500);
	}
	else if((AP_STA_mode == STA_MODE_ON) || (STA_MODE_ON == wifi_mode_retry) || (1 == wifi_disconnect))
	{
		temp = STA_MODE_ON;
		//配置为STA模式，获取mac地址并存储，设置WiFi名称
		if(STA_MODE_ON == wifi_mode_retry)
			wifi_mode_retry = 0;

		if(1 == wifi_disconnect)
			wifi_disconnect = 0;

		wifi_delay_ms(200);
		wifi_printf("AT+RESTORE\r\n");
		wifi_delay_ms(500);
		wifi_printf("ATE0\r\n");
		wifi_delay_ms(200);
		wifi_printf("%s\r\n",WIFI_SET_DEF_STA_MODE_STR);
		wifi_delay_ms(200);
		wifi_printf("%s\r\n",WIFI_SEND_GET_STA_MAC_ADD_STR);
		wifi_delay_ms(2000);
		if(mac_getted == 0)
		{
			wifi_mode_retry = STA_MODE_ON;
			return;
		}
		wifi_printf("%s\r\n",WIFI_SET_DEF_STA_MODE_STR);
		wifi_delay_ms(1000);

		wifi_got_ip = 0;
		wifi_got_OK = 0;

		wifi_printf("AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PWD);

		for(i=0; i<100; i++)
		{
			wifi_delay_ms(100);
			if(AP_STA_mode == STA_MODE_OFF)
			{
				wifi_mode_retry = STA_MODE_OFF;
				return;
			}
			if(wifi_got_ip == 1)
				break;
		}
		if(i == 100)
		{
			wifi_mode_retry = STA_MODE_ON;
			return;
		}

		for(i=0; i<100; i++)
		{
			wifi_delay_ms(100);
			if(AP_STA_mode == STA_MODE_OFF)
			{
				wifi_mode_retry = STA_MODE_OFF;
				return;
			}
			if(wifi_got_OK == 1)
				break;
		}
		if(i == 100)
		{
			wifi_mode_retry = STA_MODE_ON;
			return;
		}

		wifi_got_ip = 0;
		wifi_got_OK = 0;

		key[0] = 0;
		strcpy(key, WIFI_KEY);

		wifi_printf("encryption key:%s\r\n",key);
		wifi_delay_ms(500);

		wifi_printf("AT+CIPCLOSE\r\n");
		wifi_delay_ms(1000);

		wifi_printf("%s\r\n",WIFI_TCP_SET_CIPMUX_STR);
		wifi_delay_ms(1000);
		wifi_printf("%s\r\n",WIFI_TCP_SERVER_OPEN_80_STR);
		wifi_delay_ms(1000);

		wifi_printf("%s\r\n",WIFI_SEND_GET_STA_MAC_ADD_STR);
		wifi_delay_ms(1000);

		wifi_disconnect = 0;
		online_timeout = 0;
	}
	send_to_server_flag = 0;
}

