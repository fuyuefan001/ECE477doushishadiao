#include "timer.h"
#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//?????????,??????,??????????
//ALIENTEK STM32F7???
//?????????	   
//????@ALIENTEK
//????:www.openedv.com
//????:2015/11/27
//??:V1.0
//????,?????
//Copyright(C) ????????????? 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

TIM_HandleTypeDef TIM3_Handler;      //????? 
TIM_HandleTypeDef TIM2_Handler;      //????? 
extern u8 ov_frame;
extern volatile u16 jpeg_data_len;
extern volatile u8 jpeg_data_qinf;
//extern volatile u8 qinf;
//?????3?????
//arr:??????
//psc:??????
//???????????:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=???????,??:Mhz
//?????????3!(???3??APB1?,???HCLK/2)

TIM_HandleTypeDef TIM7_Handler;      //��ʱ����� 
extern vu16 USART2_RX_STA;

//������ʱ��7�жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!
TIM_HandleTypeDef TIM3_Handler;         //????? 
TIM_OC_InitTypeDef TIM3_CH3Handler;     //???3??4??
TIM_HandleTypeDef TIM3_Handler;         //????? 
TIM_OC_InitTypeDef TIM3_CH4Handler;     //???3??4??
void TIM7_Int_Init(u16 arr,u16 psc)
{
		TIM7_Handler.Instance=TIM7;                          //ͨ�ö�ʱ��3
    TIM7_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM7_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM7_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM7_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM7_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM7_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE									 
}
//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
// void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
// {
//     if(htim->Instance==TIM7)
// 	{
// 		__HAL_RCC_TIM7_CLK_ENABLE();            //ʹ��TIM7ʱ��
// 		HAL_NVIC_SetPriority(TIM7_IRQn,0,1);    //�����ж����ȼ�����ռ���ȼ�0�������ȼ�1
// 		HAL_NVIC_EnableIRQ(TIM7_IRQn);          //����ITM7�ж�   
// 	}
// }
//��ʱ��7�жϷ������		    
void TIM7_IRQHandler(void)
{ 	    		    
	// HAL_TIM_IRQHandler(&TIM7_Handler);
		 USART2_RX_STA|=1<<15;	//��ǽ������
		 __HAL_TIM_CLEAR_FLAG(&TIM7_Handler,TIM_EventSource_Update );       //���TIM7�����жϱ�־  
	 TIM7->CR1&=~(1<<0);     			//�رն�ʱ��7     											 
} 
void TIM_SetTIM3Compare4(u32 compare)
{
	TIM3->CCR4=compare; 
}
void TIM_SetTIM3Compare3(u32 compare)
{
	TIM3->CCR3=compare; 
}
void TIM3_PWM_Init(u16 arr,u16 psc)
{ 
    TIM3_Handler.Instance=TIM3;            //???3
    TIM3_Handler.Init.Prescaler=psc;       //?????
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//??????
    TIM3_Handler.Init.Period=arr;          //??????
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM3_Handler);       //???PWM
    
    TIM3_CH3Handler.OCMode=TIM_OCMODE_PWM1; //????PWM1
    TIM3_CH3Handler.Pulse=arr/2;            //?????,?????????,
                                            //???????????????,?????50%
    TIM3_CH3Handler.OCPolarity=TIM_OCPOLARITY_HIGH; //???????? 
    HAL_TIM_PWM_ConfigChannel(&TIM3_Handler,&TIM3_CH3Handler,TIM_CHANNEL_3);//??TIM3??4
    HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_3);//??PWM??4
   TIM_SetTIM3Compare3(150); //?????,?????
   TIM_SetTIM3Compare4(150); 
}
void TIM3_PWM_Init1(u16 arr,u16 psc)
{ 
    TIM3_Handler.Instance=TIM3;            //???3
    TIM3_Handler.Init.Prescaler=psc;       //?????
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;//??????
    TIM3_Handler.Init.Period=arr;          //??????
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&TIM3_Handler);       //???PWM
    
    TIM3_CH4Handler.OCMode=TIM_OCMODE_PWM1; //????PWM1
    TIM3_CH4Handler.Pulse=arr/2;            //?????,?????????,
                                            //???????????????,?????50%
    TIM3_CH4Handler.OCPolarity=TIM_OCPOLARITY_HIGH; //???????? 
    HAL_TIM_PWM_ConfigChannel(&TIM3_Handler,&TIM3_CH4Handler,TIM_CHANNEL_4);//??TIM3??4
    HAL_TIM_PWM_Start(&TIM3_Handler,TIM_CHANNEL_4);//??PWM??4
   TIM_SetTIM3Compare3(150); //?????,?????
   TIM_SetTIM3Compare4(150); 
}


void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //?????3
    TIM3_Handler.Init.Prescaler=psc;                     //??
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //?????
    TIM3_Handler.Init.Period=arr;                        //?????
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    //HAL_TIM_Base_Start_IT(&TIM3_Handler); //?????3????3??   
}

//???????,????,???????
//?????HAL_TIM_Base_Init()????
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	 if(htim->Instance==TIM5)
	{
    __HAL_RCC_TIM3_CLK_ENABLE();            //??TIM3??
    HAL_NVIC_SetPriority(TIM3_IRQn,10,3);    //???????,?????1,????3
    HAL_NVIC_EnableIRQ(TIM3_IRQn);          //??ITM3??  
}

	    if(htim->Instance==TIM5)
	{
		__HAL_RCC_TIM5_CLK_ENABLE();            //??TIM2??
		HAL_NVIC_SetPriority(TIM5_IRQn,3,3);    //???????,?????1,????3
		HAL_NVIC_EnableIRQ(TIM5_IRQn);          //??ITM3??   
	}  
        if(htim->Instance==TIM7)
	{
		__HAL_RCC_TIM7_CLK_ENABLE();            //ʹ��TIM7ʱ��
		HAL_NVIC_SetPriority(TIM7_IRQn,9,9);    //�����ж����ȼ�����ռ���ȼ�0�������ȼ�1
		HAL_NVIC_EnableIRQ(TIM7_IRQn);          //����ITM7�ж�   
	}
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_Initure;
 __HAL_RCC_TIM3_CLK_ENABLE();   //?????3
    __HAL_RCC_GPIOB_CLK_ENABLE();   //??GPIOB??
 
    GPIO_Initure.Pin=GPIO_PIN_0;            //PB1
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;   //??????
    GPIO_Initure.Pull=GPIO_PULLUP;          //??
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //??
 GPIO_Initure.Alternate=GPIO_AF2_TIM3; //PB1???TIM3_CH4
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);


//GPIO_InitTypeDef GPIO_Initure;
 __HAL_RCC_TIM3_CLK_ENABLE();   //?????3
    __HAL_RCC_GPIOB_CLK_ENABLE();   //??GPIOB??
 
    GPIO_Initure.Pin=GPIO_PIN_1;            //PB1
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;   //??????
    GPIO_Initure.Pull=GPIO_PULLUP;          //??
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //??
 GPIO_Initure.Alternate=GPIO_AF2_TIM3; //PB1???TIM3_CH4
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}
//???3??????
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

//???3????????



void TIM2_Init(u16 arr,u16 psc)
{  
    TIM2_Handler.Instance=TIM5;                          //?????3
    TIM2_Handler.Init.Prescaler=psc;                     //??
    TIM2_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //?????
    TIM2_Handler.Init.Period=arr;                        //?????
    TIM2_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//??????
    HAL_TIM_Base_Init(&TIM2_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM2_Handler); //?????3????3????:TIM_IT_UPDATE    
}

//???????,????,???????
//?????HAL_TIM_Base_Init()????


//???3??????
void TIM5_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM2_Handler);
}

