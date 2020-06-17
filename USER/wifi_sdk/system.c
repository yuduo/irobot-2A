/****************************************Copyright (c)*************************
**                               版权所有 (C), 2015-2020, 涂鸦科技
**
**                                 http://www.tuya.com
**
**--------------文件信息-------------------------------------------------------
**文   件   名: system.c
**描        述: wifi数据处理函数
**使 用 说 明 : 用户无需关心该文件实现内容
**
**
**--------------当前版本修订---------------------------------------------------
** 版  本: v2.3.8
** 日　期: 2018年1月17日
** 描　述: 1:变量添加volatile防止编译器优化
           2:添加#error提示
		   
** 版  本: v2.3.7
** 日　期: 2017年4月18日
** 描　述: 1:优化串口队列接收处理
		   
** 版  本: v2.3.6
** 日　期: 2016年7月21日
** 描　述: 1:修复获取本地时间错误
           2:添加hex_to_bcd转换函数
		   
** 版  本: v2.3.5
** 日　期: 2016年6月3日
** 描　述: 1:修改返回协议版本为0x01
           2:固件升级数据偏移量修改为4字节

** 版  本: v2.3.4
** 日　期: 2016年5月26日
** 描　述: 1:优化串口解析函数
           2:优化编译器兼容性,取消enum类型定义

** 版  本: v2.3.3
** 日　期: 2016年5月24日
** 描　述: 1:修改mcu获取本地时间函数
           2:添加wifi功能测试

** 版  本: v2.3.2
** 日　期: 2016年4月23日
** 描　述: 1:优化串口数据解析
           2:优化MCU固件升级流程
           3:优化上报流程

** 版  本: v2.3.1
** 日　期: 2016年4月15日
** 描　述: 1:优化串口数据解析

** 版  本: v2.3
** 日　期: 2016年4月14日
** 描　述: 1:支持MCU固件在线升级

** 版  本: v2.2
** 日　期: 2016年4月11日
** 描　述: 1:修改串口数据接收方式

** 版  本: v2.1
** 日　期: 2016年4月8日
** 描　述: 1:加入某些编译器不支持函数指针兼容选项

** 版  本: v2.0
** 日　期: 2016年3月29日
** 描　述: 1:优化代码结构
           2:节省RAM空间
**
**-----------------------------------------------------------------------------
******************************************************************************/
#define SYSTEM_GLOBAL
#include "sys.h"
//#include "wifi.h"	//
//#include "protocol.h"
//
//
extern const DOWNLOAD_CMD_S download_cmd[];

/*****************************************************************************
函数名称 : set_wifi_uart_byte
功能描述 : 写wifi_uart字节
输入参数 : dest:缓存区其实地址;
           byte:写入字节值
返回参数 : 写入完成后的总长度
*****************************************************************************/
unsigned short set_wifi_uart_byte(unsigned short dest, unsigned char byte)
{
  unsigned char *obj = (unsigned char *)wifi_uart_tx_buf + DATA_START + dest;
  
  *obj = byte;
  dest += 1;
  
  return dest;
}
/*****************************************************************************
函数名称 : set_wifi_uart_buffer
功能描述 : 写wifi_uart_buffer
输入参数 : dest:目标地址
           src:源地址
           len:数据长度
返回参数 : 无
*****************************************************************************/
unsigned short set_wifi_uart_buffer(unsigned short dest, unsigned char *src, unsigned short len)
{
  unsigned char *obj = (unsigned char *)wifi_uart_tx_buf + DATA_START + dest;
  
  my_memcpy(obj,src,len);
  
  dest += len;
  return dest;
}
/*****************************************************************************
函数名称 : wifi_uart_write_data
功能描述 : 向wifi uart写入连续数据
输入参数 : in:发送缓存指针
           len:数据发送长度
返回参数 : 无
*****************************************************************************/
static void wifi_uart_write_data(unsigned char *in, unsigned short len)
{
  if((NULL == in) || (0 == len))
  {
    return;
  }
  
  while(len --)
  {
    uart_transmit_output(*in);
    in ++;
  }
}
/*****************************************************************************
函数名称 : get_check_sum
功能描述 : 计算校验和
输入参数 : pack:数据源指针
           pack_len:计算校验和长度
返回参数 : 校验和
*****************************************************************************/
unsigned char get_check_sum(unsigned char *pack, unsigned short pack_len)
{
  unsigned short i;
  unsigned char check_sum = 0;
  
  for(i = 0; i < pack_len; i ++)
  {
    check_sum += *pack ++;
  }
  
  return check_sum;
}
/*****************************************************************************
函数名称 : wifi_uart_write_frame
功能描述 : 向wifi串口发送一帧数据
输入参数 : fr_type:帧类型
           len:数据长度
返回参数 : 无
*****************************************************************************/
void wifi_uart_write_frame(unsigned char fr_type, unsigned short len)
{
  unsigned char check_sum = 0;
  
  wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
  wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
  wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x00;
  wifi_uart_tx_buf[FRAME_TYPE] = fr_type;
  wifi_uart_tx_buf[LENGTH_HIGH] = len >> 8;
  wifi_uart_tx_buf[LENGTH_LOW] = len & 0xff;
  len += PROTOCOL_HEAD;
  check_sum = get_check_sum((unsigned char *)wifi_uart_tx_buf, len - 1);
  wifi_uart_tx_buf[len - 1] = check_sum;
  wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len);
}
void wifi_uart_write_cmd(unsigned char fr_type, unsigned short len)
{
  unsigned char check_sum = 0;
  wifi_uart_tx_buf[HEAD_FIRST] = 0x55;
  wifi_uart_tx_buf[HEAD_SECOND] = 0xaa;
/*
#if MAP_VERSION
  wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x03;
#else
  wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x01;
#endif
*/
  wifi_uart_tx_buf[PROTOCOL_VERSION] = 0x03;//Candy MCU->WIFI 此处都为03
  wifi_uart_tx_buf[FRAME_TYPE] = fr_type;
  wifi_uart_tx_buf[LENGTH_HIGH] = len >> 8;
  wifi_uart_tx_buf[LENGTH_LOW] = len & 0xff;
  
  len += PROTOCOL_HEAD;
  check_sum = get_check_sum((unsigned char *)wifi_uart_tx_buf, len - 1);
  wifi_uart_tx_buf[len - 1] = check_sum;
  /*if(0x28 == wifi_uart_tx_buf[FRAME_TYPE])
  {
	  log_printf("[wifi]");
	  for(unsigned char iii = 0; iii < len; iii++)
	  {
		log_printf("%02x ",wifi_uart_tx_buf[iii]);
	  }
	  log_printf("\r\n");
  }*/
  //
  wifi_uart_write_data((unsigned char *)wifi_uart_tx_buf, len);
}
/*****************************************************************************
函数名称 : get_update_dpid_index
功能描述 : 或许制定DPID在数组中的序号
输入参数 : dpid:dpid
返回参数 : index:dp序号
*****************************************************************************/
static unsigned char get_dowmload_dpid_index(unsigned char dpid)
{
  unsigned char index;
  unsigned char total = get_download_cmd_total();
  
  for(index = 0; index < total; index ++)
  {
    if(download_cmd[index].dp_id == dpid)
    {
      break;
    }
  }
  
  return index;
}
/*****************************************************************************
函数名称 : data_point_handle
功能描述 : 下发数据处理
输入参数 : value:下发数据源指针
返回参数 : ret:返回数据处理结果
*****************************************************************************/
static unsigned char data_point_handle(const unsigned char value[])
{
  unsigned char dp_id,index;
  unsigned char dp_type;
  unsigned char ret;
  unsigned short dp_len;
  
  dp_id = value[0];
  dp_type = value[1];
  dp_len = value[2] * 0x100;
  dp_len += value[3];
  
  index = get_dowmload_dpid_index(dp_id);

  if(dp_type != download_cmd[index].dp_type && dp_type != 0)
  {
    //错误提示
    log_printf("[IDX]%d-%d:%d-%d\r\n",index,dp_type,download_cmd[index].dp_id,download_cmd[index].dp_type);
    return FALSE;
  }
  else
  {
    ret = dp_download_handle(dp_id,value + 4,dp_len);
  }
  if(ret == FALSE)
  {
  	log_printf("[ERR]%d\r\n",dp_id);
  }
  return ret;
}
/*****************************************************************************
函数名称 : data_handle
功能描述 : 数据帧处理
输入参数 : offset:数据起始位
返回参数 : 无
*****************************************************************************/
void data_handle(unsigned short offset)
{
  unsigned short dp_len;
  ErrorStatus ret;
  unsigned short i,total_len;
  unsigned char cmd_type = wifi_uart_rx_buf[offset + FRAME_TYPE];

  log_printf("\r\n[data_handle]cmd:%d\r\n",cmd_type);
  switch(cmd_type)
  {
  case WIFI_STATE_CMD:                                  //wifi工作状态	
	{
	    wifi_work_state = wifi_uart_rx_buf[offset + DATA_START];
	    //wifi_uart_write_cmd(WIFI_STATE_CMD,0);
	    //uint8_t ding_cnt = 0;
	    switch(wifi_work_state)
	    {
	    	case WIFI_NOT_CONNECTED:
	    		//ding_cnt = 3;
	    		sys->wifi_time_sync = RESET;
	    		break;
		case WIFI_CONN_CLOUD:
			//ding_cnt = 4;
			break;
		case SMART_CONFIG_STATE:
			//ding_cnt = 1;
			break;
		case AP_STATE:
			//ding_cnt = 2;
			break;
		default:
			break;
	    }
	    log_printf("[WIFI]WIFI_STATE_CMD:%d\r\n",wifi_work_state);
	    /*
	    if(0 < ding_cnt)
	    {
	    	for(uint8_t i=0;i<ding_cnt;i++)
			{
				delay_ms_tm5(1000);
				ny3p_play(VOICE_DIDI);
			}
	    }
	    */
	}
    break;
    
  case WIFI_MODE_CMD:                                   //选择smartconfig/AP模式(wifi返回成功)	
    set_wifimode_flag = SET_WIFICONFIG_SUCCESS;
    break;
    
  case DATA_QUERT_CMD:                                  //命令下发
    total_len = wifi_uart_rx_buf[offset + LENGTH_HIGH] * 0x100;
    total_len += wifi_uart_rx_buf[offset + LENGTH_LOW];
    
    for(i = 0;i < total_len;)
    {
      dp_len = wifi_uart_rx_buf[offset + DATA_START + i + 2] * 0x100;
      dp_len += wifi_uart_rx_buf[offset + DATA_START + i + 3];
      //
      ret = (ErrorStatus)data_point_handle((unsigned char *)(wifi_uart_rx_buf + offset + DATA_START + i));
      
      if(SUCCESS == ret)
      {
        //成功提示
        log_printf("WIFI data_point_handle SUCCESS\r\n");
      }
      else
      {
        //错误提示
        log_printf("WIFI data_point_handle ERR\r\n");
      }
      
      i += (dp_len + 4);
    }
    
    break;
    
//#ifdef SUPPORT_MCU_RTC_CHECK
  case GET_LOCAL_TIME_CMD:                             //获取本地时间
      mcu_write_rtctime((unsigned char *)(wifi_uart_rx_buf + offset + DATA_START));
    break;
//#endif
 
//#ifdef WIFI_STREAM_ENABLE
  case STREAM_OPEN_CMD:
    stream_status = wifi_uart_rx_buf[offset + DATA_START];//流服务开启是否成功
    break;
  
  case STREAM_START_CMD:
    stream_status = wifi_uart_rx_buf[offset + DATA_START];//流数据开启是否成功
    break;

  case STREAM_TRANS_CMD:
    stream_status = wifi_uart_rx_buf[offset + DATA_START];//流服务传输返回接收
    break;

  case STREAM_STOP_CMD:
    stream_status = wifi_uart_rx_buf[offset + DATA_START];//流数据结束是否成功
    break;
//#endif    
  default:
    break;
  }
}
/*****************************************************************************
函数名称 : get_queue_total_data
功能描述 : 读取队列内数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
unsigned char get_queue_total_data(void)
{
  if(queue_in != queue_out)
    return 1;
  else
    return 0;
}
/*****************************************************************************
函数名称 : Queue_Read_Byte
功能描述 : 读取队列1字节数据
输入参数 : 无
返回参数 : 无
*****************************************************************************/
unsigned char Queue_Read_Byte(void)
{
  unsigned char value;
  
  if(queue_out != queue_in)
  {
    //有数据
    if(queue_out >= (unsigned char *)(wifi_queue_buf + sizeof(wifi_queue_buf)))
    {
      //数据已经到末尾
      queue_out = (unsigned char *)(wifi_queue_buf);
    }
    
    value = *queue_out ++;   
  }
  
  return value;
}

