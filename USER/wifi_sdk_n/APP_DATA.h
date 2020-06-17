#ifndef __APP_DATA_H_
#define __APP_DATA_H_


#include "sys.h"


//当前位置表示是何种机器
//两档位、三档位、老版本机器

//#define LEVEL2_MACH 1
#define LEVEL3_MACH 1
//#define OLD_BOARD   1

#ifdef LEVEL2_MACH
	#define MACH_HIGH_SPEED 1195
	#define MACH_LOW_SPEED  420
#endif

//#define LEVEL3_DISABLE 1
#define RES_200MR      1

//s#define TEST_ASSNO     1

#ifdef LEVEL3_MACH
	#ifdef LEVEL3_DISABLE
		#define MACH_HIGH_SPEED 0 //%99
		#define MACH_LOW_SPEED  0 //70%
		#define MACH_MUTE_SPEED 0 //35%
	#endif
	
	#ifndef LEVEL3_DISABLE
//		#define MACH_HIGH_SPEED 300 //%99
//		#define MACH_LOW_SPEED  300 //70%
//		#define MACH_MUTE_SPEED 300 //35%

		#define MACH_HIGH_SPEED 1195 //%99
		#define MACH_LOW_SPEED  840 //70%
		#define MACH_MUTE_SPEED 420 //35%
//		#define MACH_HIGH_SPEED 1195 //%99
//		#define MACH_LOW_SPEED  1195 //70%
//		#define MACH_MUTE_SPEED 1195 //35%
	#endif
#endif

#ifdef OLD_BOARD
	#define MACH_HIGH_SPEED 1999
	#define MACH_LOW_SPEED  1000
#endif

//////////////////////////////////////////
#define WIFISSID "hellomjq"
#define WIFIPWD  "wsmjq3210"
#define WIFIKEY  "LIUKAIDEMO"

#define AP_MODE_ON   0xaa
#define AP_MODE_OFF  0xbb
#define STA_MODE_ON  0xcc
#define STA_MODE_OFF 0xdd
#define WIFI_MODE_RESET 0xee

#define SUCCESS 0xaa
#define FAILD   0xbb
#define PROCESS 0xcc

#define HIGH 1
#define LOW  0
#define MUTE 2

#define LOCK 1
#define UNLOCK 0

#define POWER_OFF 10
#define POWER_ON  20

extern char machinePowerOff;
extern char eeprom_data_update_status;//数据更新是否完成
extern char binSwitch;//尘桶开关
extern char machEnBin;     //电机允许运行

extern char machMd; //0--4
extern double baV;  //24.9V
extern char baSt;   //A--G
extern char fiErr;  //A,B
extern char maErr,mmo,rmo,bmo,dbn;  //A--D

//以下数据将存入EEPROM中
extern char newMach;//‘A’ 为已经更新过数据，否则将进入数据更新
extern char agiSt;  //0,1
extern char mms;    //0,1

extern char lockSt; //0,1

extern long  tMin;   //0--65535

extern uint8_t LUTime[];//2018/01/02 18:29:10

extern int molw;

extern double tarea;   //100平方米

extern double avgSpeed;// 0.5 m/s

extern long aMin[];//n 10

extern long TMIHM;   //1000min

extern uint8_t DOFU[];//

extern long Tdays;

extern char AP_STA_mode;

extern char online;

extern char WIFI_SSID[];

extern char WIFI_PWD[];

extern char WIFI_KEY[];

extern long tMin_sec;

extern long tMihm_sec;

extern char http_read_queue[];

extern char http_config_port;
extern char http_config_queue[];

extern char http_write_port[];
extern char http_write_queue0[];
extern char http_write_queue1[];

#define V100_HIGH   22.0
#define V75_HIGH    20.5  //21.2
#define V50_HIGH    19.1  //20.4
#define V25_HIGH    18.6  //18.9

#define V100_LOW   22.7   
#define V75_LOW    21.4   //21.2
#define V50_LOW    20.6   //20.4
#define V25_LOW    19.08  //18.9

#define V100_MUTE   23.3  
#define V75_MUTE    21.9  //21.1
#define V50_MUTE    20.9  //20.3
#define V25_MUTE    20.0  //18.9

#define VLOW   18.5
#define V5     18

#endif
