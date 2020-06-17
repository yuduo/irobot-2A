/**
  ******************************************************************************
  * @file USART/Interrupt/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version  V3.0.0
  * @date  04/06/2009
  * @brief  Main Interrupt Service Routines.
  *         This file provides template for all exceptions handler and 
  *         peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "sys.h"
/*
#include "pid.h"
#include "bsp.h"
#include "systick.h"
#include "usart.h"
#include "motor.h"
#include "pid.h"
#include "iclean.h"
#include "string.h"
*/

//左电机，主霍尔计算
void TIM4_IRQHandler(void)
{

	 		

 }
//右电机霍尔计数
void TIM2_IRQHandler(void)

{

}

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval : None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval : None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  TargetSysReset();
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval : None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval : None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval : None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval : None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval : None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval : None
  */
/*
void PendSV_Handler(void)
{
}
*/
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
/*
void SysTick_Handler(void)
{
}
*/


/******************************************************************************/
/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/
/******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
 // CanRxMsg RxMessage;
 // CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

}
/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
void CAN1_RX1_IRQHandler(void)
{	
} 
/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
  /*
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus( USART1, USART_IT_RXNE) !=RESET)
	{
	    USART_ClearITPendingBit( USART1, USART_IT_RXNE);
	}
}
 */
/**
  * @brief  This function handles USART2 global interrupt request.
  * @param  None
  * @retval None
  */



/**
  * @brief  This function handles DMA1_Channel1 global interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Channel1_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
  {	 
	 DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */


/*
PE8	是IR电源开关	输出	
PC7	中间IR（回充用来接收信号）	中断输入/捕获输入	INT7/TIM3_CH2
PB12	左边IR（回充用来接收信号的）	中断输入/捕获输入	INT12
PC8	右边IR（回充用来接收信号)	中断输入/捕获输入	INT8/TIM3_CH3
PC9	360°IR接收	中断输入/捕获输入					INT9/TIM3_CH4

*/
//char staaa=0;
extern uint8_t bdtest_fantest;
extern uint16_t tmod_fancd;
void EXTI15_10_IRQHandler(void)
{
//	uint8_t dir1,dir2;
/*
	if(EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
 
	     if(GPIOC->IDR & GPIO_Pin_12)
	     {
   	 		rx_ird(&iRight);	     	
	     }
	
	 	EXTI_ClearITPendingBit(EXTI_Line13);
	}
	*/
#if IR_L_NUM == 15
	if(EXTI_GetITStatus(IR_L_LINE)!=RESET){
			EXTI_ClearITPendingBit(IR_L_LINE);
			ir_parse(IR_L_PIN_NUM);
		}	
#endif
#if IR_M_NUM == 15
	if(EXTI_GetITStatus(IR_M_LINE)!=RESET){
		EXTI_ClearITPendingBit(IR_M_LINE);
		ir_parse(IR_M_PIN_NUM);
	}
#endif		
/*
	if(EXTI_GetITStatus(IR_L_LINE)!=RESET){
		EXTI_ClearITPendingBit(IR_L_LINE);
		ir_parse(IR_L_PIN_NUM);
	}
	*/

	//wy190615 滚刷码盘
	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		navigat->gun_cnt++;
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
/*	
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		navigat->behind_hw++;
 		EXTI_ClearITPendingBit(EXTI_Line13);
 	}
*/	
/*
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
		//navigat->distance--;
 		//if(////navigat->is_walk)
 		if(sys->sState == SYS_TEST_BD && bdtest_fantest)
 		{
 			if(tmod_fancd ++ > 30000)tmod_fancd = 30000;
 			//log_printf("><");
 		}
 		else
 		{
	 		if(sys->mState == GO_FORWARD || sys->mState == GO_BACK || sys->sState == SYS_TEST_BD || sys->sState == SYS_IDLE || sys->sState == SYS_RANDOM)
	 		{
	 			dir1 = (GPIOD->IDR & GPIO_Pin_14)?1:0;
	 			dir2 = (GPIOD->IDR & GPIO_Pin_13)?1:0;

					if(dir1 != dir2)	
			     	{
			     	//	log_printf("e");
			   	 		motor.c_front_hw--;	    
			   	 		navigat->distance--;
		 
#if CALE_BY_FRONT
			   	 		calc_xy_org(-1,&navigat->x_org_t,&navigat->y_org_t);	
#endif		   	 		
		   	 		}else
		   	 		{
						motor.c_front_hw++;	    
			   	 		navigat->distance++;
			   	 	
#if CALE_BY_FRONT
			   	 		calc_xy_org(1,&navigat->x_org_t,&navigat->y_org_t);	
#endif		   	 		
	   	 		
		   	 		}
		   	 	
	   
			 }
			 else if(sys->sState == SYS_IDLE)
			 {
				 dir1 = (GPIOD->IDR & GPIO_Pin_14)?1:0;
				 dir2 = (GPIOD->IDR & GPIO_Pin_13)?1:0;
	 
				 if(dir1 != dir2)	 
				 {

					 navigat->distance--;
		
				 }else
				 {
					 navigat->distance++;
				 }

			 }
		 }
	 	EXTI_ClearITPendingBit(EXTI_Line13);
	}	
*/				 	
}
void EXTI9_5_IRQHandler(void)
{
#if IR_L_NUM == 7
	if(EXTI_GetITStatus(IR_L_LINE)!=RESET){
			EXTI_ClearITPendingBit(IR_L_LINE);
			ir_parse(IR_L_PIN_NUM);
		}	
#endif
#if (IR_M_NUM == 7) || (IR_M_NUM == 9)
	if(EXTI_GetITStatus(IR_M_LINE)!=RESET){
		EXTI_ClearITPendingBit(IR_M_LINE);
		ir_parse(IR_ML_PIN_NUM);
	}
#endif	
#if (IR_M2_NUM == 9) || (IR_M2_NUM == 7)
	if(EXTI_GetITStatus(IR_M2_LINE)!=RESET){
		EXTI_ClearITPendingBit(IR_M2_LINE);
		//log_printf("*");
		ir_parse(IR_MR_PIN_NUM);
	}
#endif	
#if IR_R_NUM == 8
	if(EXTI_GetITStatus(IR_R_LINE)!=RESET){
		EXTI_ClearITPendingBit(IR_R_LINE);
		ir_parse(IR_R_PIN_NUM);
	}
#endif	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		if(GPIOB->IDR & GPIO_Pin_5)
	     {
	     	  	 		
	     }

		EXTI_ClearITPendingBit(EXTI_Line5);
	}
/*
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		if(GPIOC->IDR & GPIO_Pin_7)
	     {
   	 		rx_ird(&iRight);	     	
	     }

		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		if(GPIOC->IDR & GPIO_Pin_8)
	     {
   	 		rx_ird(&iRight);	     	
	     }

		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	if(EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		if(GPIOC->IDR & GPIO_Pin_9)
	     {
   	 		rx_ird(&iRight);	     	
	     }
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
	*/
}
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  
void EXTI2_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line2))
   {
   	//log_printf("ir2,t=%d\r\n",STIME->CNT+sys->tird);
   	if(GPIOC->IDR & 4)
   	 rx_ird(&iRight);
	  EXTI_ClearFlag(EXTI_Line2);
   }	
}*/
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */
  //PE4	右轮码盘输出A相
/*
PE4	码盘输出A相
PE5	码盘输出B相
*/
void EXTI4_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line4))
   {
   	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
   	{
		if(sys->mState == GO_FORWARD || sys->mState == GO_BACK || sys->sState == SYS_TEST_BD/*|| sys->sState == SYS_IDLE*/)// 
		{
#if HW_AB_EN			
			int dir1 = (GPIOE->IDR & GPIO_Pin_4)?1:0;
			int dir2 = (GPIOE->IDR & GPIO_Pin_5)?1:0;
#if HW_AB_OLD//新旧码盘板的AB相是相反的
			if(dir1 == dir2)
#else
			if(dir1 != dir2)
#endif
	     		{  
	   	 		motor.c_right_hw++;
#if JUDGE_STOP_FRONT
	   	 		
	   	 		if(motor.c_front_stop >0)
#endif	   	 		
	   	 			calc_xy_org(1,&navigat->x_org_r,&navigat->y_org_r);	


   	 		}else
   	 		{
	   	 		motor.c_right_hw--;
	   	 		#if CALE_ADJ_RIGHT
	   	 			calc_xy_org(-1,&navigat->x_org_r,&navigat->y_org_r);	
	   	 		#endif
   	 		}	
#else
			if(sys->mState == GO_BACK)	
	     		{  
	   	 		motor.c_right_hw--;
	   	 		calc_xy_org(-1,&navigat->x_org_r,&navigat->y_org_r);	
   	 		}else if(sys->mState == GO_FORWARD)
   	 		{
	   	 		motor.c_right_hw++;
	   	 		calc_xy_org(1,&navigat->x_org_r,&navigat->y_org_r);	
   	 		}else
   	 			motor.c_right_hw++;
#endif
		}else
		{
			motor.c_right_hw++; //左转，右转的时候，要做速度PID，只加加
		}			
   	}
	  EXTI_ClearFlag(EXTI_Line4);
   }
}

//PD1	左后轮码盘出输A相
/*
PD1	左后轮码盘输出A相
PD2	左后轮码盘输出B相
*/
void EXTI1_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line1))
   {
   		if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1))
   		{
			if(sys->mState == GO_FORWARD || sys->mState == GO_BACK || sys->sState == SYS_TEST_BD)
			{
#if HW_AB_EN				
				int dir1 = (GPIOD->IDR & GPIO_Pin_1)?1:0;
				int dir2 = (GPIOD->IDR & GPIO_Pin_2)?1:0;
#if HW_AB_OLD//新旧码盘板的AB相是相反的
				if(dir1 == dir2)
#else
				if(dir1 != dir2)
#endif
				{
					motor.c_left_hw--;
#if JUDGE_STOP_FRONT
					
					if(motor.c_front_hw>0)
#endif					
						calc_xy_org(-1,&navigat->x_org_f,&navigat->y_org_f);	
					motor.c_front_hw = 0;
				}else
				{
					motor.c_left_hw++;
#if JUDGE_STOP_FRONT
					
					//log_printf("%d,%d,%d\r\n",motor.c_left_hw,motor.c_front_hw,motor.c_front_stop );
					if(sys->c_front_hw >= MAX_FRONT_HW)
						sys->c_front_hw = 0;
					sys->front_hw[sys->c_front_hw++] = motor.c_front_hw;
					
					if(motor.c_front_hw > 0)
					{
						motor.c_front_stop = 10;
					}
					
					if(motor.c_front_stop > 0)
						motor.c_front_stop--;
					if(motor.c_front_stop >0)
#endif					
					{
						calc_xy_org(1,&navigat->x_org_f,&navigat->y_org_f);	
					}
						
					motor.c_front_hw = 0;
				}
#else
				if(sys->mState == GO_FORWARD)
				{
					motor.c_left_hw++;
					calc_xy_org(1,&navigat->x_org_f,&navigat->y_org_f);	
				}else if(sys->mState == GO_BACK)
				{
					motor.c_left_hw--;
					calc_xy_org(-1,&navigat->x_org_f,&navigat->y_org_f);	
				}else
					motor.c_left_hw++;	//左转，右转的时候，要做速度PID，只加加
#endif
			}else
				motor.c_left_hw++;	//左转，右转的时候，要做速度PID，只加加
   		}
		EXTI_ClearFlag(EXTI_Line1);
   }
}
/*
void EXTI2_IRQHandler(void)
{
   if(EXTI_GetITStatus(EXTI_Line2))
   {
   		//log_printf("ir5\r\n");
   		if(GPIOD->IDR & 4)
   		{
		//	motor.speed_left++;
		//	if(navigat->is_walk)
			if(sys->mState == GO_FORWARD || sys->mState == GO_BACK || sys->sState == SYS_TEST_BD)
			//if(1)//if(sys->mState == GO_FORWARD || sys->mState == GO_BACK || sys->sState == SYS_TEST_BD)
			{
#if HW_AB_EN				
				int dir1 = (GPIOD->IDR & GPIO_Pin_1)?1:0;
				int dir2 = (GPIOD->IDR & GPIO_Pin_2)?1:0;
#if HW_AB_OLD//新旧码盘板的AB相是相反的
				if(dir1 == dir2)
#else
				if(dir1 != dir2)
#endif
				{
					motor.c_left_hw--;
#if JUDGE_STOP_FRONT
					
					if(motor.c_front_hw>0)
#endif					
						calc_xy_org(-1,&navigat->x_org_f,&navigat->y_org_f);	
					motor.c_front_hw = 0;
				}else
				{
					motor.c_left_hw++;
#if JUDGE_STOP_FRONT
					
					//log_printf("%d,%d,%d\r\n",motor.c_left_hw,motor.c_front_hw,motor.c_front_stop );
					if(sys->c_front_hw >= MAX_FRONT_HW)
						sys->c_front_hw = 0;
					sys->front_hw[sys->c_front_hw++] = motor.c_front_hw;
					
					if(motor.c_front_hw > 0)
					{
						motor.c_front_stop = 10;
					}else
					{
						
						
					}
					if(motor.c_front_stop > 0)
						motor.c_front_stop--;
					if(motor.c_front_stop >0)
#endif					
					{
						calc_xy_org(1,&navigat->x_org_f,&navigat->y_org_f);	
					}//else
						//log_printf("e%d,%d\r\n",motor.c_left_hw,navigat->distance);
						
					motor.c_front_hw = 0;
				}
#else
				if(sys->mState == GO_FORWARD)
				{
					motor.c_left_hw++;
					calc_xy_org(1,&navigat->x_org_f,&navigat->y_org_f);	
				}else if(sys->mState == GO_BACK)
				{
					motor.c_left_hw--;
					calc_xy_org(-1,&navigat->x_org_f,&navigat->y_org_f);	
				}else
					motor.c_left_hw++;	//左转，右转的时候，要做速度PID，只加加
#endif
			}else
				motor.c_left_hw++;	//左转，右转的时候，要做速度PID，只加加


   		}
   		//rx_ird(&iLeft);
	  EXTI_ClearFlag(EXTI_Line2);
   }
}
*/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */
void TIM1_TRG_COM_IRQHandler(void)
{ 
  TIM1->SR&=0;           //清中断标志
	     //调用换向函数

}
/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/
void TIM1_BRK_IRQHandler(void)
{
//	TIM_ClearFlag(TIM1, TIM_FLAG_Break);
//	BLDC_Stop();
 //	LED_On();
//	TIM_CtrlPWMOutputs(TIM1,DISABLE);
//	TIM_ITConfig(TIM1, TIM_IT_COM ,DISABLE);
//	TIM_ITConfig(TIM3, TIM_IT_Trigger|TIM_IT_CC4, DISABLE);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Break);
//	TIM_ITConfig(TIM1, TIM_IT_Break, DISABLE);
}

/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/

//int tread=0,twrite=0;
	
void TIM6_IRQHandler(void)
{

  //  uint8_t gIO_ird;

	 if(TIM_GetITStatus(STIME,TIM_IT_Update)!=RESET)
	 {
	 	  STIME->CNT=0;
	 	  
		  sys->tird+=10000;	

		  TIM_ClearITPendingBit(STIME,TIM_IT_Update);  

	 }
	 
}

/**********************************************************************
* Description    : None
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
**********************************************************************/

//int tread=0,twrite=0;
extern uint32_t irda_tmr;	
void TIM7_IRQHandler(void)
{

  //  uint8_t gIO_ird;

	 if(TIM_GetITStatus(TIM7,TIM_IT_Update)!=RESET)
	 {
	 	  TIM7->CNT=0;
	 	  
		  irda_tmr+=65536;	

		  TIM_ClearITPendingBit(TIM7,TIM_IT_Update);  
		  //log_printf("\r\n[%d]\r\n",irda_tmr);
	 }
	 
}


/*
void TIM6_IRQHandler(void)
{

	uint8_t idx;

	 if(TIM_GetITStatus(STIME,TIM_IT_Update)!=RESET)
	 {
	 	  STIME->CNT=0;
	 	 // log_printf("time2 updata\r\n");
		  TIM_ClearITPendingBit(STIME,TIM_IT_Update);  
	   	
		  //状态 sta 从1开始   8*2*2 = 32  2个字节
		  if(sys->sta >=17 || sys->tx_byte==0)
		  {
		//	en_pwm(sys->chnl, 0);  //关掉所有	
			en_pwm(0, 0);
			TIM_Cmd(STIME, DISABLE); 		//是能定时器
			log_printf("[t2o]send over ,sta=%d,byte=%02X\r\n",sys->sta,sys->tx_byte);
			//TIM_Cmd(TIM8, ENABLE);		//发送完了，把接收开起来
			sys->sta=0;
			sys->tx_byte=0;
		  }else
		  {
			  if(sys->sta & 1)		//1// 1 3 5 7 都是发送高电平,奇数，发送高电平
			  {
				STIME->ARR = 500;
				en_pwm(sys->chnl, 0);		//不输出PWM，高电平
				//log_printf("sat=%d,disPWM,arr=500\r\n",sys->sta);
				sys->sta++;				
			  }else
			  {
				idx = sys->sta >> 1;
				en_pwm(sys->chnl, 1);				
				idx--;
				if((sys->tx_byte>>idx) & 1)		// 1 1ms
					STIME->ARR = 1000;
				else
					STIME->ARR = 500;
				//log_printf("sat=%d,enPWM,arr=%d\r\n",sys->sta,STIME->ARR);
				sys->sta++;
					
			  }
		  }
	  

	 }
	 
}

*/



#define IR_TIME	 	TIM8

uint16_t c_tmr8_out=0;

uint16_t c_tmr8_out1=0;

void TIM8_UP_IRQHandler(void)
{
/*
	 	// 2 ms溢出一次
		 if(TIM_GetITStatus(IR_TIME,TIM_IT_Update)!=RESET)
		 {
		  	TIM_ClearITPendingBit(IR_TIME,TIM_IT_Update);
		  	sys->rx_byte1=0;
		  	sys->rx_idx1=sys->rx_rbe1=0;
		  	if(c_tmr8_out1  <=250) // 200 ms
		  	{
				c_tmr8_out1++;	
		  	}else
		  	{
				sys->c_rx_idra=0;
		  	}
		  }
		  */
}
//用来做初略计算，及遥控
void TIM8_CC_IRQHandler(void)
{
//		uint16_t ccr;
//	uint8_t idx;
	 //红外接收
	 IR_TIME->CNT=0;
	 /*
	if (TIM_GetITStatus(TIM8, TIM_IT_CC2) != RESET)
	{
	     TIM_ClearITPendingBit(TIM8, TIM_IT_CC2 );
	     
		ccr=IR_TIME->CCR2;
		

		if(sys->rx_idx1 > 0  && sys->rx_idx1 <=MAX_RX_BIT)  //去除掉引导码
		{
			  idx=sys->rx_idx1-1;

			  if(ccr >=500 && ccr < 900)		// 0 以700为中心
			  {
				 if(ccr<600 || ccr >800)
				 	sys->rx_rbe1++;
				 	//log_printf("far ccr=%d\r\n",ccr);
			  }
			  else if(ccr >=1000 && ccr < 1400)	  // 1 以1200为中心
			  {
				sys->rx_byte1 |=(1 <<idx);
				 if(ccr<900 || ccr >1300)
				 	sys->rx_rbe1++;
				 				
			  }else
			  {
				//log_printf("rx error,idx=%d,ccr=%d\r\n",sys->rx_idx,ccr);
			  	sys->rx_idx1=sys->rx_byte1=sys->rx_rbe1=0;
			  	return ;
			  }

			  if(c_tmr8_out1>=220)
			  {
				STIME->CNT = 0;  // 定时，重新接受
				//log_printf("reset stim\r\n");
			  }
			  c_tmr8_out1=0;
			  
			  if(sys->rx_idx1 ==MAX_RX_BIT)
			  {
			  	//if(sys->trace)
			  	//log_printf("rx:%X,%d\r\n",sys->rx_byte1,sys->rx_rbe1);
			  	insert_irda(sys->rx_byte1);
			  	sys->rx_idx1=sys->rx_byte1=sys->rx_rbe1=0;
			  }

			
		}
		sys->rx_idx1++;
	

	}
	*/
}

void TIM3_IRQHandler(void)
{
//		uint16_t ccr;
//	uint8_t idx;
	/*
	 //红外接收
	 TIM3->CNT=0;
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
	{
	     TIM_ClearITPendingBit(TIM3, TIM_IT_CC2 );
	     
		ccr=TIM3->CCR2;
	  // log_printf("cc3=%d\r\n",ccr);
		if(sys->rx_idx > 0  && sys->rx_idx <=MAX_RX_BIT)  //去除掉引导码
		{
			  idx=sys->rx_idx-1;

			  if(ccr >=300 && ccr < 700)		// 0 以500为中心
			  {
				 if(ccr<400 || ccr >600)
				 	if(rx_irda.freq <250)
				 		rx_irda.freq++;
				 	
			  }
			  else if(ccr >=800 && ccr < 1200)	  // 1 以1000为中心
			  {
				sys->rx_byte |=(1 <<idx);
				 if(ccr<900 || ccr >1100)
				 	if(rx_irda.freq <250)
				 		rx_irda.freq++;
				 	//log_printf("far ccr=%d\r\n",ccr);				
			  }else
			  {
			//	log_printf("rx error,idx=%d,ccr=%d\r\n",sys->rx_idx,ccr);
			  	sys->rx_idx=sys->rx_byte=0;
			  	return ;
			  }

			  if(c_tmr8_out>=210)
			  {
				TIM3->CNT = 0;  // 定时，重新接受
				//log_printf("reset stim\r\n");
			  }
			  c_tmr8_out=0;
			  
			  if(sys->rx_idx ==MAX_RX_BIT)
			  {
			  	//if(sys->trace)
			  	//	log_printf("rx:%d\r\n",sys->rx_byte);
			  	insert_irda(sys->rx_byte);
			  	sys->rx_idx=sys->rx_byte=0;
			  }

			
		}
		sys->rx_idx++;

	}
	 if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
	 {
	  	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	  	sys->rx_byte=0;
	  	sys->rx_idx=0;
	  	if(c_tmr8_out  <=200) // 200 ms
	  	{
			c_tmr8_out++;	
	  	}
	  }	
	*/
}


void TIM1_CC_IRQHandler(void)
{

}


void ADC1_2_IRQHandler(void)
{

//	static uint8_t st=0;
	if(ADC_GetITStatus(ADC1, ADC_IT_JEOC) == SET)
	{
		ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);

		
	}

}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
