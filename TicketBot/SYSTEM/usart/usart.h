#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	
#include "stdarg.h"	 	 	 	 
#include "string.h"	
#include "timer.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2015/6/23
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.0�޸�˵�� 
////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	#define USART2_MAX_RECV_LEN		800					//�����ջ����ֽ���
#define USART2_MAX_SEND_LEN		800					//����ͻ����ֽ���?
#define USART2_RX_EN 			1					//0,������;1,����.

extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
extern UART_HandleTypeDef UART1_Handler; //UART���
extern UART_HandleTypeDef UART2_Handler; //UART���
#define RXBUFFERSIZE   1 //�����С
extern u8 aRxBuffer[RXBUFFERSIZE];//HAL��USART����Buffer
extern u8  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//���ջ���,���USART2_MAX_RECV_LEN�ֽ�
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//���ͻ���,���USART2_MAX_SEND_LEN�ֽ�
extern u16 USART2_RX_STA;   						//��������״̬
//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
#endif
