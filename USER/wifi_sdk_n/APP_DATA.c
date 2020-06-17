#include "APP_DATA.h"

char machinePowerOff = POWER_ON;
char eeprom_data_update_status;//数据更新是否完成
char binSwitch;//尘桶开关
char machEnBin;     //电机允许运行

char machMd; //0--4
double baV;  //24.9V
char baSt;   //A--G
char fiErr;  //A,B
char maErr;  //A--D
char mmo,rmo,bmo,dbn;

//以下数据将存入EEPROM中
char newMach;//‘A’ 为已经更新过数据，否则将进入数据更新
char agiSt;  //0,1
char mms;    //0,1

char lockSt; //0,1

long  tMin;   //0--65535

uint8_t LUTime[20];//2018/01/02 18:29:10

int molw;

double tarea;   //100平方米

double avgSpeed;// 0.5 m/s

long aMin[9];//n 10

long TMIHM;   //1000min

uint8_t DOFU[20];//

long Tdays;

char AP_STA_mode;

char online;

char WIFI_SSID[15];

char WIFI_PWD[15];

char WIFI_KEY[15];

long tMin_sec; //s
long tMihm_sec;


char http_read_queue[5] = {0};

char http_config_port = 0;
char http_config_queue[150];

char http_write_port[2] = {0};
char http_write_queue0[50] = {0};
char http_write_queue1[50] = {0};

