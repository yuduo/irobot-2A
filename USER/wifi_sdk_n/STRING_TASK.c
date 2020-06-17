#include "sys.h"
#include "STRING_TASK.h"


extern char data_process;

char key[] = {"ame-1234ame-1234"};

void my_strcat(char * response_str, char * head, char * datastr, char * tail);

char bav_temp_buf[5];
char bav_first_in = 0;
unsigned int delay_45min;		//TIME TASK ÓÐ¶¨Òå
int return_power()
{
	int temp = 0;
	int temp1;
	char i;
	double temp_double = 0;
	double V100,V75,V50,V25;
	
	if(machMd != 2)
	{
		switch(mms)
		{
			case HIGH:
				V100 = V100_HIGH;
				V75  = V75_HIGH;
			    V50  = V50_HIGH;
			    V25  = V25_HIGH;
			break;
			
			case LOW:
				V100 = V100_LOW;
				V75  = V75_LOW;
			    V50  = V50_LOW;
			    V25  = V25_LOW;
			break;
			
			case MUTE:
				V100 = V100_MUTE;
				V75  = V75_MUTE;
			    V50  = V50_MUTE;
			    V25  = V25_MUTE;
			break;
			
		}
	}
	
	if(machMd != 2)
	{
		if(baV > V100)       //100%
		{
			temp_double = 100.0;
		}
		else if(baV > V75)  //75 %
		{
			temp_double = 100.0 - ((V100 - baV)/(V100 - V75))*25.0;
		}
		else if(baV > V50)  //50 %
		{
			temp_double = 75.0 - ((V75 - baV)/(V75 - V50))*25.0;
		}
		else if(baV > V25)   //25  %
		{
			temp_double = 50.0 - ((V50 - baV)/(V50 - V25))*25.0;
		}
		else if(baV > VLOW)   //5  %
		{
			temp_double = 25.0 - ((V25 - baV)/(V25 - VLOW))*20.0;
		}
		else temp_double = 0;
	}
	else 
	{
		if(baV > 24.77)       //100%
		{
			temp_double = 100.0;
		}
		else if(baV > 23.25)  //68 %
		{
			temp_double = 100.0 - ((24.77 - baV)/(24.77 - 23.25));
		}
		else if(baV > 22.23)  //35 %
		{
			temp_double = 68.0 - ((23.25 - baV)/(23.25 - 22.23))*33.0;
		}
		else if(baV > 21.46)   //5  %
		{
			temp_double = 35.0 - ((22.23 - baV)/(22.23 - 21.46))*30.0;
		}
		else temp_double = 0;
	}
	temp = temp_double;
	//temp = temp - temp%5;
	if(bav_first_in == 0)
	{
		for(i=0;i<5;i++)
			bav_temp_buf[i] = temp;
		bav_first_in = 1;
	}
	for(i=0;i<4;i++)
		bav_temp_buf[i] = bav_temp_buf[i+1];
	bav_temp_buf[4] = temp;
	
	temp1 = 0;
	for(i=0;i<5;i++)
		temp1 += bav_temp_buf[i];
	temp = temp1;
	temp /= 5;
	temp = temp - temp%5;
	
	return temp;
}

char agiStTemp;
void string_process(char * response_str)
{
    char data_buf[50],data_buf1[50];
    char i;
	
	
    data_buf[0] = 0;

    response_str[0] = 0;

    //print_log("start\r\n");
    sprintf((char*)data_buf,"%ld",machMd);
    //my_strcat(response_str, "{\"",data_buf,"\",");
    my_strcat(response_str, "{\"MachMd\":\"",data_buf,"\",");

    agiStTemp = 0;
    sprintf((char*)data_buf,"%ld",agiStTemp);
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"AgiSt\":\"",data_buf,"\",");

    sprintf((char*)data_buf,"%ld",mms);
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"MMS\":\"",data_buf,"\",");

//    sprintf((char*)data_buf,"%.1lf",baV);
    //my_strcat(response_str, "\"",data_buf,"\",");
//    my_strcat(response_str, "\"BaV\":\"",data_buf,"\",");

    
	
	sprintf((char*)data_buf,"%ld",return_power());
    my_strcat(response_str, "\"BaSt\":\"",data_buf,"\",");
	
    sprintf((char*)data_buf,"%ld",lockSt);
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"LockSt\":\"",data_buf,"\",");

	data_buf[0] = fiErr;
    data_buf[1] = 0;
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"FiErr\":\"",data_buf,"\",");
	
   /* data_buf[0] = mmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"MMO\":\"",data_buf,"\",");*/
	
	
    data_buf[0] = rmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"RMO\":\"",data_buf,"\",");
	
	
    data_buf[0] = bmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"BMO\":\"",data_buf,"\",");
	
	
    data_buf[0] = dbn;
    data_buf[1] = 0;
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"DBN\":\"",data_buf,"\",");
	
	

    //sprintf((char*)data_buf,"%ld",tMin);
    //my_strcat(response_str, "\"",data_buf,"\",");
    //my_strcat(response_str, "\"",data_buf,"\",");
    //my_strcat(response_str, "\"TMin\":\"",data_buf,"\",");

	LUTime[16] = 0;
    my_strcat((char*)response_str, "\"LUTime\":\"",(char*)LUTime,"\",");
    //my_strcat(response_str, "\"",(char*)LUTime,"\",");

    sprintf((char*)data_buf,"%d",molw);
    my_strcat(response_str, "\"MOLW\":\"",data_buf,"\",");
	
    sprintf((char*)data_buf,"%.1lf",tarea);
    my_strcat(response_str, "\"Tarea\":\"",data_buf,"\",");
    //my_strcat(response_str, "\"",data_buf,"\",");

    sprintf((char*)data_buf,"%.1lf",avgSpeed);
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"AvgSpeed\":\"",data_buf,"\",");


    for(i=1; i<9; i++)
    {
        data_buf1[0] = 0;
        strcat((char*)data_buf1,"\"AMin");
        data_buf[0] = 0;
        sprintf((char*)data_buf,"%d",i);

        strcat((char*)data_buf1,(const char*)data_buf);
        strcat((char*)data_buf1,"\":\"");

        sprintf((char*)data_buf,"%ld",aMin[i]);
        strcat(data_buf1,data_buf);
        strcat(data_buf1,"\",");
        strcat(response_str, data_buf1);
        //my_strcat(response_str, "\"" ,data_buf,"\",");
    }

    data_buf[0] = 0;
    sprintf((char*)data_buf,"%ld",TMIHM);
    //my_strcat(response_str, "\"",data_buf,"\",");
    my_strcat(response_str, "\"TMIHM\":\"",data_buf,"\",");

    //my_strcat(response_str, "\"",(char*)DOFU,"\",");
	DOFU[16] = 0;
    my_strcat(response_str, "\"DOFU\":\"",(char*)DOFU,"\",");

    sprintf((char*)data_buf,"%ld",Tdays);
    //my_strcat(response_str, "\"",data_buf,"\"}");
    my_strcat(response_str, "\"Tdays\":\"",data_buf,"\"}");

    //print_log("%s%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);

    //base16_encryption(key,response_str);
    //print_log("%sdata=%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);

    //base16_decrypt(key,response_str);
    //print_log("%sdata=%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);
    //print_log("end\r\n");
}


void string_send_to_server(char * response_str)
{
    char data_buf[50],data_buf1[50];
    char i;
    data_buf[0] = 0;

    response_str[0] = 0;

    //print_log("start\r\n");
    sprintf((char*)data_buf,"%ld",machMd);
    //my_strcat(response_str, "{",data_buf,"&");
    my_strcat(response_str, "MachMd=",data_buf,"&");

    agiStTemp = 0;
    sprintf((char*)data_buf,"%ld",agiStTemp);
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "AgiSt=",data_buf,"&");

    sprintf((char*)data_buf,"%ld",mms);
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "MMS=",data_buf,"&");

//    sprintf((char*)data_buf,"%.1lf",baV);
    //my_strcat(response_str, "",data_buf,"&");
//    my_strcat(response_str, "BaV=",data_buf,"&");


    /*data_buf[0] = baSt;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "BaSt=",data_buf,"&");*/
	
	sprintf((char*)data_buf,"%ld",return_power());
    my_strcat(response_str, "BaSt=",data_buf,"&");

    sprintf((char*)data_buf,"%ld",lockSt);
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "LockSt=",data_buf,"&");

    data_buf[0] = fiErr;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "FiErr=",data_buf,"&");
	
    /*data_buf[0] = mmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "MMO=",data_buf,"&");*/
	
	data_buf[0] = rmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "RMO=",data_buf,"&");
	
	data_buf[0] = bmo;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "BMO=",data_buf,"&");
	
	data_buf[0] = dbn;
    data_buf[1] = 0;
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "DBN=",data_buf,"&");

    //sprintf((char*)data_buf,"%ld",tMin);
    //my_strcat(response_str, "",data_buf,"&");
    //my_strcat(response_str, "",data_buf,"&");
    //ssmy_strcat(response_str, "TMin=",data_buf,"&");

	LUTime[16] = 0;
    my_strcat((char*)response_str, "LUTime=",(char*)LUTime,"&");
    //my_strcat(response_str, "",(char*)LUTime,"&");

    sprintf((char*)data_buf,"%d",molw);
    my_strcat(response_str, "MOLW=",data_buf,"&");
	
    sprintf((char*)data_buf,"%.1lf",tarea);
    my_strcat(response_str, "Tarea=",data_buf,"&");
    //my_strcat(response_str, "",data_buf,"&");

    sprintf((char*)data_buf,"%.1lf",avgSpeed);
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "AvgSpeed=",data_buf,"&");


    for(i=1; i<9; i++)
    {
        data_buf1[0] = 0;
        strcat((char*)data_buf1,"AMin");
        data_buf[0] = 0;
        sprintf((char*)data_buf,"%d",i);

        strcat((char*)data_buf1,(const char*)data_buf);
        strcat((char*)data_buf1,"=");

        sprintf((char*)data_buf,"%ld",aMin[i]);
        strcat(data_buf1,data_buf);
        strcat(data_buf1,"&");
        strcat(response_str, data_buf1);
        //my_strcat(response_str, "" ,data_buf,"&");
    }

    data_buf[0] = 0;
    sprintf((char*)data_buf,"%ld",TMIHM);
    //my_strcat(response_str, "",data_buf,"&");
    my_strcat(response_str, "TMIHM=",data_buf,"&");

    //my_strcat(response_str, "",(char*)DOFU,"&");
	DOFU[16] = 0;
    my_strcat(response_str, "DOFU=",(char*)DOFU,"&");

    sprintf((char*)data_buf,"%ld",Tdays);
    //my_strcat(response_str, "",data_buf,"}");
    my_strcat(response_str, "Tdays=",data_buf,"");

    //print_log("%s%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);

    //base16_encryption(key,response_str);
    //print_log("%sdata=%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);

    //base16_decrypt(key,response_str);
    //print_log("%sdata=%s \r\n\r\n",RESPONSE_HEAD_ENCRYPTED_STRING,response_str);
    //print_log("data=%s \r\n\r\n",response_str);
    //print_log("end\r\n");
}

void string_task(void *pvParameters)
{
    while(1)
    {
        wifi_delay_ms(5000);
    }
}

void my_strcat(char * str, char * head, char * datastr, char * tail)
{
    strcat(str,head);
    strcat(str,(const char *)datastr);
    strcat(str,tail);
}

