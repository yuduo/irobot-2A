#ifndef __WIFI_DATA_H_
#define __WIFI_DATA_H_

#define WIFI_SET_DEF_A_P_MODE_STR "AT+CWMODE=2"

#define WIFI_SET_DEF_STA_MODE_STR "AT+CWMODE=1"

#define WIFI_SEND_GET_STA_MAC_ADD_STR "AT+CIFSR"

#define WIFI_SEND_AP_CMD_STR "AT+CWSAP="

#define WIFI_SEND_AP_SSID_STR "\"WIFIVACUUMCLEANER-"

#define WIFI_SEND_AP_PWD_STR "\",\"00000000\",11,0"

#define WIFI_TCP_SERVER_OPEN_80_STR "AT+CIPSERVER=1,80"

#define WIFI_TCP_SET_CIPMUX_STR "AT+CIPMUX=1"

#define RESPONSE_HEAD_ENCRYPTED_STRING "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nEncrypted:1\r\ncache-Control: no-cache\r\n\r\n"

#define RESPONSE_HEAD_NOENCRYPTED_STRING "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nEncrypted:0\r\ncache-Control: no-cache\r\n\r\n"


//·þÎñÆ÷µØÖ·
#define SERVER_ADDRESS "simplyfimgmt.candy-hoover.com"

#define SEND_TO_SERVER_HEAD_STRING "GET "
#define SEND_SERVER_END " HTTP/1.1\r\nHost:simplyfimgmt.candy-hoover.com\r\n\r\n"

#define SEND_TO_SERVER_FILE_STRING "/api/av1/listen.json?"

#define SERVER_TIMEOUT 30

#endif

