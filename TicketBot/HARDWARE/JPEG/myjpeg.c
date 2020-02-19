#include "myjpeg.h"
#include "ltdc.h"
#include "lcd.h"
#include "malloc.h"
#include "jpeg_utils.h"
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

JPEG_HandleTypeDef  JPEG_Handler;           //JPEG���
DMA_HandleTypeDef   JPEGDMAIN_Handler;      //JPEG��������DMA
DMA_HandleTypeDef   JPEGDMAOUT_Handler;     //JPEG�������DMA

_jpeg_dev jpeg_dev;     //����JPEG����Ҫ����

JPEG_YCbCrToRGB_Convert_Function pConvert_Function;

u8 Out_DataBuffer[OUT_BUFFER_NUM][CHUNK_SIZE_OUT];  //���������
u8 In_DataBuffer[OUT_BUFFER_NUM][CHUNK_SIZE_IN];    //���뻺����

extern u32 decodetime;

//��ʾJPEGͼƬ
//x,y��ͼƬ���Ͻ���LCD�ϵ�����
//JPEGFileName:Ҫ��ʾ��ͼƬ�ļ�·��
//mode:��ʾģʽ��0 ����ָ��λ����ʾ,X��Y������Ч
//               1 ������ʾ��X��Y������Ч
void JPEG_Display(u16 x,u16 y,u8 *JPEGFileName,u8 mode)
{
    JPEG_ConfTypeDef JPEG_Info;
    u8 state=0;
    
    jpeg_dev.totalblock=0;          //�ܿ�������
    jpeg_dev.blockindex=0;          //��ʼ��������Ϊ��
    jpeg_dev.decodend=0;            //����״̬��ʶ���㣬����δ���
    jpeg_dev.FrameBufAddr=0;        //֡��ַ����

    jpeg_dev.outbuf_readindex=0;    //�����������λ������
    jpeg_dev.outbuf_writeindex=0;   //���������дλ������
    jpeg_dev.output_paused=0;  

    jpeg_dev.inbuf_readindex=0;     //���뻺������λ������
    jpeg_dev.inbuf_writeindex=0;    //���뻺����дλ������
    jpeg_dev.input_paused=0;      
    
    if(f_open(&jpeg_dev.jpegfile,(const TCHAR*)JPEGFileName,FA_READ)!=FR_OK)//��ͼƬ
    {
        printf("ͼƬ��ʧ��\r\n");
    }
    jpeg_decode(JPEG_OUTPUT_DATA_BUFFER);
    do
    {
        SCB_CleanInvalidateDCache();
        jpegdata_input();
        state=jpegdata_output();
          
    }while(state==0);
    HAL_JPEG_GetInfo(&JPEG_Handler,&JPEG_Info);
    
    //ͼƬ�ߴ�С��LCD�ߴ��ʱ����ܾ�����ʾ
    if(mode&&(lcddev.width>=JPEG_Info.ImageWidth)&&(lcddev.height>=JPEG_Info.ImageHeight))
    {
        x=(lcddev.width-JPEG_Info.ImageWidth)/2;
        y=(lcddev.height-JPEG_Info.ImageHeight)/2;
    }
    printf("����ͼƬ��ʱ:%dms\r\n",decodetime/10);
    dma2d_datacopy((u32 *)JPEG_OUTPUT_DATA_BUFFER,(u32 *)LCD_FRAME_BUF_ADDR,x,y,lcddev.width,lcddev.height);
    f_close(&jpeg_dev.jpegfile);        //�ر��ļ� 
}

//JPEG����ģ���ʼ��
void JPEG_Init(void)
{ 
    u8 i=0;
       
    //������ݻ����������Ϣ��ʼ��
    for(i=0;i<OUT_BUFFER_NUM;i++)
    {
        jpeg_dev.OUT_Buffer[i].State=JPEG_BUFFER_EMPTY;
        jpeg_dev.OUT_Buffer[i].DataBufferSize=0;
        jpeg_dev.OUT_Buffer[i].DataBuffer=Out_DataBuffer[i];
    }
  
    //�������ݻ����������Ϣ��ʼ��
    for(i=0;i<OUT_BUFFER_NUM;i++)
    {
        jpeg_dev.IN_Buffer[i].State=JPEG_BUFFER_EMPTY;
        jpeg_dev.IN_Buffer[i].DataBufferSize=0;
        jpeg_dev.IN_Buffer[i].DataBuffer=In_DataBuffer[i];
    }
    
    JPEG_InitColorTables();         //��ʼ��JPEG��LUT��YCbCr��RGB֮���ת��
    JPEG_Handler.Instance=JPEG;
    HAL_JPEG_Init(&JPEG_Handler);   //��ʼ��JPEG
}

//JPEG�ײ�������ʱ��ʹ��
//�˺����ᱻHAL_JPEG_Init()����
//hsdram:JPEG���
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg)
{
    __HAL_RCC_JPEG_CLK_ENABLE();            //ʹ��JPEGʱ��
    __DMA2_CLK_ENABLE();                    //ʹ��DMA2ʱ��
    
    HAL_NVIC_SetPriority(JPEG_IRQn,1,0);    //����JPEG�жϣ���ռ���ȼ�1�������ȼ�0
    HAL_NVIC_EnableIRQ(JPEG_IRQn);          //ʹ��JPEG�ж�
    
    //������������DMA   
    JPEGDMAIN_Handler.Instance=DMA2_Stream0;                        //DMA2������0
    JPEGDMAIN_Handler.Init.Channel=DMA_CHANNEL_9;                   //ͨ��9
    JPEGDMAIN_Handler.Init.Direction=DMA_MEMORY_TO_PERIPH;          //�洢��������ģʽ
    JPEGDMAIN_Handler.Init.PeriphInc=DMA_PINC_DISABLE;              //���������ģʽ
    JPEGDMAIN_Handler.Init.MemInc=DMA_MINC_ENABLE;                  //�洢������ģʽ
    JPEGDMAIN_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_WORD; //�������ݳ���:32λ
    JPEGDMAIN_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_WORD;    //�洢�����ݳ���:32λ
    JPEGDMAIN_Handler.Init.Mode=DMA_NORMAL;                         //��ͨģʽ
    JPEGDMAIN_Handler.Init.Priority=DMA_PRIORITY_HIGH;              //�����ȼ�
    JPEGDMAIN_Handler.Init.FIFOMode=DMA_FIFOMODE_ENABLE;            //ʹ��FIFO   
    JPEGDMAIN_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;   //ʹ��ȫFIFO
    JPEGDMAIN_Handler.Init.MemBurst=DMA_MBURST_INC4;                //�洢��4�ֽ�����ͻ������
    JPEGDMAIN_Handler.Init.PeriphBurst=DMA_PBURST_INC4;             //����4�ֽ�����ͻ������
  
    __HAL_LINKDMA(hjpeg,hdmain,JPEGDMAIN_Handler);      //����������DMA��JPEG����������DMA��������
    HAL_DMA_DeInit(&JPEGDMAIN_Handler);            
    HAL_DMA_Init(&JPEGDMAIN_Handler);                   //��ʼ����������DMA

    //�����������DMA  
    JPEGDMAOUT_Handler.Instance=DMA2_Stream1;                       //DMA2������1   
    JPEGDMAOUT_Handler.Init.Channel=DMA_CHANNEL_9;                  //ͨ��9    
    JPEGDMAOUT_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;         //���赽�洢��
    JPEGDMAOUT_Handler.Init.PeriphInc=DMA_PINC_DISABLE;             //���������ģʽ
    JPEGDMAOUT_Handler.Init.MemInc=DMA_MINC_ENABLE;                 //�洢������ģʽ
    JPEGDMAOUT_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_WORD;//�������ݳ���:32λ
    JPEGDMAOUT_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_WORD;   //�洢�����ݳ���:32λ
    JPEGDMAOUT_Handler.Init.Mode=DMA_NORMAL;                        //��ͨģʽ
    JPEGDMAOUT_Handler.Init.Priority=DMA_PRIORITY_VERY_HIGH;        //������ȼ�
    JPEGDMAOUT_Handler.Init.FIFOMode=DMA_FIFOMODE_ENABLE;           //ʹ��FIFO
    JPEGDMAOUT_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;  //ʹ��ȫFIFO
    JPEGDMAOUT_Handler.Init.MemBurst=DMA_MBURST_INC4;               //�洢��4�ֽ�����ͻ������
    JPEGDMAOUT_Handler.Init.PeriphBurst=DMA_PBURST_INC4;            //����4�ֽ�����ͻ������

    __HAL_LINKDMA(hjpeg, hdmaout, JPEGDMAOUT_Handler);  //���������DMA��JPEG���������DMA��������
    HAL_DMA_DeInit(&JPEGDMAOUT_Handler);  
    HAL_DMA_Init(&JPEGDMAOUT_Handler);                  //��ʼ���������DMA
   
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn,2,0);//��������DMA�ж����ȼ�����ռ���ȼ�2�������ȼ�0
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);      //ʹ��JPEG����������DMA�ж�
        
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn,2,0);//�������DMA�ж����ȼ�����ռ���ȼ�2�������ȼ�0
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);      //ʹ��JPEG���������DMA�ж�
 
}

//JPEG����
//DestAddress:Ŀ�ĵ�ַ
//����ֵ:0 �ɹ���������ʧ��
u8 jpeg_decode(u32 destaddr)
{
    u8 i,state=0;
    jpeg_dev.FrameBufAddr=destaddr;       
    //��ȡJPEG���ݵ�����buffer��
    for(i=0;i<IN_BUFFERS_NUM;i++)
    {
        if(f_read(&jpeg_dev.jpegfile,jpeg_dev.IN_Buffer[i].DataBuffer,CHUNK_SIZE_IN,(UINT*)(&jpeg_dev.IN_Buffer[i].DataBufferSize))==FR_OK)
        {
            jpeg_dev.IN_Buffer[i].State=JPEG_BUFFER_FULL;   //�������buffer����
            state=0;
        }else state=1;      
    } 
    //����JPEG(DMA��ʽ)
    HAL_JPEG_Decode_DMA(&JPEG_Handler,jpeg_dev.IN_Buffer[0].DataBuffer,jpeg_dev.IN_Buffer[0].DataBufferSize,\
                        jpeg_dev.OUT_Buffer[0].DataBuffer,CHUNK_SIZE_OUT);
    return state;
}

//DMA2D���ݿ���
//srcbuf:Դ��ַ
//dstbuf:Ŀ�ĵ�ַ
//x:X����
//y:Y����
//xsize:X���С
//ysize:Y���С
void dma2d_datacopy(u32 *srcbuf,u32 *dstbuf,u16 x,u16 y,u16 xsize,u16 ysize)
{   
    u32 timeout=0; 
    u32 destina=(u32)dstbuf+(y*lcddev.width+x)*(lcdltdc.pixsize);
    u32 source=(u32)srcbuf;

    //����DMA2D��ģʽ
    DMA2D_Handler.Instance=DMA2D; 
    DMA2D_Handler.Init.Mode=DMA2D_M2M_PFC;                  //�洢�����洢����֧�����ظ�ʽת��
    DMA2D_Handler.Init.ColorMode=LCD_PIXFORMAT;             //�����ʽ
    DMA2D_Handler.Init.OutputOffset=lcddev.width-xsize;     //���ƫ�� 
    DMA2D_Handler.Init.AlphaInverted=DMA2D_REGULAR_ALPHA;   
    DMA2D_Handler.Init.RedBlueSwap=DMA2D_RB_REGULAR;        
  
    //ǰ��������
    DMA2D_Handler.LayerCfg[0].AlphaMode=DMA2D_REPLACE_ALPHA;        //�����޸�ǰ�����ALPHAֵ
    DMA2D_Handler.LayerCfg[0].InputAlpha=0xFF;                      //����ALPHֵ
    DMA2D_Handler.LayerCfg[0].InputColorMode=DMA2D_INPUT_ARGB8888;  //������ɫģʽ
    DMA2D_Handler.LayerCfg[0].InputOffset=0;                        //���ƫ��
    DMA2D_Handler.LayerCfg[0].RedBlueSwap=DMA2D_RB_REGULAR;
    DMA2D_Handler.LayerCfg[0].AlphaInverted=DMA2D_REGULAR_ALPHA; 
    
    HAL_DMA2D_Init(&DMA2D_Handler);                                 //��ʼ��DMA2D
    HAL_DMA2D_ConfigLayer(&DMA2D_Handler,0);                        //���ò�
    HAL_DMA2D_Start(&DMA2D_Handler,source,destina,xsize,ysize);     //����DMA2D����
    HAL_DMA2D_PollForTransfer(&DMA2D_Handler,5);                    //��������
    while((__HAL_DMA2D_GET_FLAG(&DMA2D_Handler,DMA2D_FLAG_TC)==0)&&(timeout<0X5000))//�ȴ�DMA2D���
    {
        timeout++;
    }
    __HAL_DMA2D_CLEAR_FLAG(&DMA2D_Handler,DMA2D_FLAG_TC);       //���������ɱ�־  
}

//JPEG�������������JPEG���������������YUV�ź�ת��ΪRGB�źš�
//����ֵ:0���ɹ���������ʧ��
u32 jpegdata_output(void)
{
    u32 datacount;
  
    if(jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].State==JPEG_BUFFER_FULL)    //�������������
    {  
        //���YUV��RGB��ת��
        jpeg_dev.blockindex+=pConvert_Function(  jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].DataBuffer, //Ҫת��������
                                            (uint8_t *)jpeg_dev.FrameBufAddr,                               //ת����ɺ�����ݴ洢��
                                            jpeg_dev.blockindex,                                            //������        
                                            jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].DataBufferSize,  //Ҫת����������
                                            &datacount);                                                    //ת����ɵ�������       
    
        jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].State=JPEG_BUFFER_EMPTY; //ת������Ժ��Ǵ����������Ϊ��
        jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].DataBufferSize=0;        //���ݳ�������
    
        jpeg_dev.outbuf_readindex++;    //���������������һ
        if(jpeg_dev.outbuf_readindex>=OUT_BUFFER_NUM)
        {
            jpeg_dev.outbuf_readindex=0;
        }
    
        if(jpeg_dev.blockindex==jpeg_dev.totalblock)
        {
            return 1;
        }
    }
    else if((jpeg_dev.output_paused==1)&& 
          (jpeg_dev.outbuf_writeindex==jpeg_dev.outbuf_readindex)&&\
          (jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_readindex].State==JPEG_BUFFER_EMPTY))
    {
        jpeg_dev.output_paused=0;
        HAL_JPEG_Resume(&JPEG_Handler,JPEG_PAUSE_RESUME_OUTPUT);  //�ָ�����     
    }
    return 0;  
}

//JPEG���봦��
void jpegdata_input(void)
{
    if(jpeg_dev.IN_Buffer[jpeg_dev.inbuf_writeindex].State==JPEG_BUFFER_EMPTY)//��ǰ���뻺����Ϊ�գ���ȡ����
    {
        if(f_read(&jpeg_dev.jpegfile,jpeg_dev.IN_Buffer[jpeg_dev.inbuf_writeindex].DataBuffer,  //��SD���ж�ȡ����
                CHUNK_SIZE_IN,(UINT*)(&jpeg_dev.IN_Buffer[jpeg_dev.inbuf_writeindex].DataBufferSize))==FR_OK)
        {  
            jpeg_dev.IN_Buffer[jpeg_dev.inbuf_writeindex].State=JPEG_BUFFER_FULL;   //��ǵ�ǰ���뻺��������
        }
        else printf("���ݶ�ȡ����\r\n");
    
        //�����봦������ͣ���������뻺����дλ�õ��ڶ�λ��ʱӦ�ûָ�����
        if((jpeg_dev.input_paused==1)&&(jpeg_dev.inbuf_writeindex==jpeg_dev.inbuf_readindex))
        {
            jpeg_dev.input_paused=0;
            //�������뻺����
            HAL_JPEG_ConfigInputBuffer( &JPEG_Handler,                                                  //JPEG���
                                        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBuffer,        //���뻺����
                                        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBufferSize);   //���뻺��������
            HAL_JPEG_Resume(&JPEG_Handler, JPEG_PAUSE_RESUME_INPUT); //�ָ����ݴ���
        }
    
        jpeg_dev.inbuf_writeindex++;    //дλ�ü�һ
        if(jpeg_dev.inbuf_writeindex>=IN_BUFFERS_NUM)
        {
            jpeg_dev.inbuf_writeindex=0;
        }            
    }
}

//JPEGͼƬ��Ϣ��ȡ�ص�����
void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo)
{
    //ִ����ɫת��
    if(JPEG_GetDecodeColorConvertFunc(pInfo,&pConvert_Function,&jpeg_dev.totalblock)!=HAL_OK)
    {
       printf("JPEG��ɫת��ʧ��!\r\n");
    }  
}

//��ȡJPEG���ݻص�����
//hjpeg:JPEG���
//NbDecodedData:Ҫ��ȡ�����ݳ���
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
    if(NbDecodedData==jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBufferSize)
    {  
        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].State=JPEG_BUFFER_EMPTY;
        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBufferSize=0;
  
        jpeg_dev.inbuf_readindex++;
        if(jpeg_dev.inbuf_readindex>=IN_BUFFERS_NUM)
        {
            jpeg_dev.inbuf_readindex=0;        
        }
  
        if(jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].State==JPEG_BUFFER_EMPTY)
        {
            HAL_JPEG_Pause(hjpeg,JPEG_PAUSE_RESUME_INPUT); //��ͣ�������봦��
            jpeg_dev.input_paused=1;                 
        }
        else
        {    
            HAL_JPEG_ConfigInputBuffer( hjpeg,
                                        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBuffer, 
                                        jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBufferSize);    
        }
    }
    else
    {
        HAL_JPEG_ConfigInputBuffer( hjpeg,
                                    jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBuffer+NbDecodedData, 
                                    jpeg_dev.IN_Buffer[jpeg_dev.inbuf_readindex].DataBufferSize-NbDecodedData);      
    }
}

//JPEG����׼����
//hjpeg:JPEG���
//pDataOut:������ݻ�����
//OutDataLength:������ݻ���������
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_writeindex].State=JPEG_BUFFER_FULL;
    jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_writeindex].DataBufferSize=OutDataLength;
    
    jpeg_dev.outbuf_writeindex++;
    if(jpeg_dev.outbuf_writeindex>=OUT_BUFFER_NUM)
    {
        jpeg_dev.outbuf_writeindex=0;        
    }

    if(jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_writeindex].State!=JPEG_BUFFER_EMPTY)
    {
        HAL_JPEG_Pause(hjpeg,JPEG_PAUSE_RESUME_OUTPUT); //��ͣ�����������
        jpeg_dev.output_paused=1;
    }
    HAL_JPEG_ConfigOutputBuffer(hjpeg,
                                jpeg_dev.OUT_Buffer[jpeg_dev.outbuf_writeindex].DataBuffer,
                                CHUNK_SIZE_OUT); 
}

//JPEG�������
void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
     jpeg_dev.decodend=1;   //JPEG�������
}

//JPEG�жϷ�����
void JPEG_IRQHandler(void)
{
    HAL_JPEG_IRQHandler(&JPEG_Handler);
}

//DMA2������0�жϷ�����
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(JPEG_Handler.hdmain);
}

//DMA2������1�жϷ�����
void DMA2_Stream1_IRQHandler(void)
{   
    HAL_DMA_IRQHandler(JPEG_Handler.hdmaout);
}


