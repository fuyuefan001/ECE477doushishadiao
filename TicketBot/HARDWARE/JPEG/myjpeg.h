#ifndef __MYJPEG_H
#define __MYJPEG_H
#include "sys.h"
#include "ff.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F7������
//SDRAM��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/5/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define OUT_BUFFER_NUM          2
#define IN_BUFFERS_NUM          2

//JPEG���ݻ���ṹ��
typedef struct
{
    u8 State;
    u8 *DataBuffer;          //JPEG���ݻ�����
    u32 DataBufferSize;      //JPEG���ݳ���
}Data_Buffer;  

//JPEG��ز����ṹ��
typedef struct
{
    u8 decodend;            //������ɱ�־��0:δ���,1:���
    u8 outbuf_readindex;    //�����������λ��
    u8 outbuf_writeindex;   //���������дλ��
    u8 output_paused;       //���������ͣ��־��1����ͣ
    u8 inbuf_readindex;     //���뻺������λ��
    u8 inbuf_writeindex;    //���뻺����дλ��
    u8 input_paused;        //���봦����ͣ��־��1����ͣ
    u32 FrameBufAddr;       //֡��ַ
    u32 blockindex;         //������
    u32 totalblock;         //�ܿ���
    FIL jpegfile;           //JPEGͼƬ��
    Data_Buffer OUT_Buffer[OUT_BUFFER_NUM]; //�������BUFFER    
    Data_Buffer IN_Buffer[IN_BUFFERS_NUM];  //��������BUFFER    
}_jpeg_dev;


#define CHUNK_SIZE_IN  ((uint32_t)(4096)) 
#define CHUNK_SIZE_OUT ((uint32_t)(768*4)) 

#define JPEG_BUFFER_EMPTY 0
#define JPEG_BUFFER_FULL  1

#define JPEG_OUTPUT_DATA_BUFFER  0xC0200000 

extern JPEG_HandleTypeDef JPEG_Handler;//JPEG���

void JPEG_Init(void);
void JPEG_Display(u16 x,u16 y,u8 *JPEGFileName,u8 mode);
u8 jpeg_decode(u32 destaddr);
u32 jpegdata_output(void);
void jpegdata_input(void);
void dma2d_datacopy(u32 *srcbuf,u32 *dstbuf,u16 x,u16 y,u16 xsize,u16 ysize);
#endif

