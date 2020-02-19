/********************************************************************
 * test_m8266wifi.c
 * .Description
 *     Source file of M8266WIFI testing application 
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
 
#include "stdio.h"
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "M8266WIFI_ops.h"
#include "led.h"

void M8266WIFI_Test(void)
{
	 u16 i;
	 u16 status = 0;
	 u8  link_no=0;

 

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	 Macro Defitions SETUP  SOCKET CONNECTIONS  (Chinese: �����׽��ֵ�һЩ��)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 
   /////////////
   ////	 Macro for Test Type(Chinese���������ͺ궨��)
	 #define TEST_M8266WIFI_TYPE    3	     //           1 = Repeative Sending, 2 = Repeative Reception, 3 = Echo  4 = multi-clients transimission test
	                                       // (Chinese: 1=ģ�����ⲻͣ�ط������� 2=ģ�鲻ͣ�ؽ������� 3= ģ�齫���յ������ݷ��͸����ͷ� 4=��ͻ��˲���) 

	 /////////////
   ////	 Macros for Socket Connection Type (Chinese���׽������͵ĺ궨��) 
	 #define TEST_CONNECTION_TYPE   1	    //           0=WIFI module as UDP, 1=WIFI module as TCP Client, 2=WIFI module as TCP Server
	                                      // (Chinese: 0=WIFIģ����UDP, 1=WIFIģ����TCP�ͻ���, 2=WIFIģ����TCP������
   /////////////
   ////	 Macros for Soket ip:port pairs  (Chinese���׽��ֵı��ض˿ں�Ŀ���ַĿ��˿ڵĺ궨��) 
   //local port	(Chinese���׽��ֵı��ض˿�)
#if (TEST_CONNECTION_TYPE==1)         //// if module as TCP Client (Chinese:���ģ����ΪTCP�ͻ���)
	 #define TEST_LOCAL_PORT  			0			//           local port=0 will generate a random local port each time fo connection. To avoid the rejection by TCP server due to repeative connection with the same ip:port
   	                                    // (Chinese: ��local port���ݵĲ���Ϊ0ʱ�����ض˿ڻ������������һ�����ģ�����ͻ��˷������ӷ�����ʱ�����á���Ϊ��������Ķ˿�ÿ�λ᲻һ�����Ӷ������������β���ͬ���ĵ�ַ�Ͷ˿�����ʱ���������ܾ���
#elif (TEST_CONNECTION_TYPE==0) || (TEST_CONNECTION_TYPE==2) //// if module as UDP or TCP Server (Chinese:���ģ����ΪUDP��TCP������)
   #define TEST_LOCAL_PORT  			4321  //           a local port should be specified //(Chinese:���ģ����ΪUDP��TCP������������Ҫָ�����׽��ֵı��ض˿�)
#else
#error WRONG TEST_CONNECTION_TYPE defined !
#endif                                  // (Chinese: ���ģ����ΪTCP��������UDP����ô����ָ�����ض˿�

   //local port	(Chinese���׽��ֵ�Ŀ���ַ��Ŀ��˿�)
#if (TEST_CONNECTION_TYPE==0)        //// if module as UDP (Chinese:���ģ����ΪUDP�������ָ��Ŀ���ַ�Ͷ˿ڣ�Ҳ���������䣬�ڷ�������ʱ�����û����)
   #define TEST_REMOTE_ADDR    		"192.168.4.2"
   #define TEST_REMOTE_PORT  	    1234
#elif (TEST_CONNECTION_TYPE==1)      //// if module as TCP Client (Chinese:���ģ����ΪTCP�ͻ��ˣ���Ȼ����ָ��Ŀ���ַ��Ŀ��˿ڣ���ģ����Ҫȥ���ӵ�TCP�������ĵ�ַ�Ͷ˿�)
   #define TEST_REMOTE_ADDR    	 	"192.168.137.1"  // "www.baidu.com"
   #define TEST_REMOTE_PORT  	    4321						// 80
#elif (TEST_CONNECTION_TYPE==2)     //// if module as TCP Server (Chinese:���ģ����ΪTCP������)
		#define TEST_REMOTE_ADDR      "1.1.1.1" // no need remote ip and port upon setup connection. below values just for filling and any value would be ok. 
    #define TEST_REMOTE_PORT  	  1234  	 //(Chinese: ����TCP������ʱ������Ҫָ��Ŀ���ַ�Ͷ˿ڣ����������ֻ��һ����ʽ��䣬�����д��
#else
#error WRONG TEST_CONNECTION_TYPE defined !
#endif			 


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	 Setup Connection and Config connection upon neccessary (Chinese: �����׽��֣��Լ���Ҫʱ���׽��ֵ�һЩ����)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////
//step 0: config tcp windows number (Chinese: ����0�������TCP���͵��׽��֣��������õ������ڲ�����
#if ( 1 && ((TEST_CONNECTION_TYPE==1)||(TEST_CONNECTION_TYPE==2)) ) //If you hope to change TCP Windows, please change '0' to '1' in the #if clause before setup the connection
																																		//(Chinese: �����ı��׽��ֵĴ����������Խ�#if����е�0�ĳ�1�����������Ҫ�ڴ����׽���֮ǰִ��)
// u8 M8266WIFI_SPI_Config_Tcp_Window_num(u8 link_no, u8 tcp_wnd_num, u16* status)
  while(M8266WIFI_SPI_Config_Tcp_Window_num(link_no, 4, &status)==0)
  {

     	  LED0(0);
	  LED1(0);
	   
	}
#endif

///////	
//step 1: setup connection (Chinese: ����1�������׽������ӣ�

  //  u8 M8266WIFI_SPI_Setup_Connection(u8 tcp_udp, u16 local_port, char remote_addr, u16 remote_port, u8 link_no, u8 timeout_in_s, u16* status);
	if(M8266WIFI_SPI_Setup_Connection(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, TEST_REMOTE_ADDR, TEST_REMOTE_PORT, link_no, 20, &status)==0)
	{		
		 while(1)
		 {
	  LED0(0);
	  LED1(0);
		 }
	}
	else  // else: setup connection successfully, we could config it (Chinese: �����׽��ֳɹ����Ϳ��������׽��֣�
	{
#if (0 && (TEST_CONNECTION_TYPE == 0) )  		//// If UDP, then the module could join a multicust group. If you hope to use multicust, Change first '0' to '1'  in the #if clause
		                                        //   (Chinese: �������׽�����UDP����ô�������ó��鲥ģʽ�������Ҫ���ó��鲥�����Խ�#if����еĵ�һ��'0'�ĳ�'1')
			//u8 M8266WIFI_SPI_Set_Multicuast_Group(u8 join_not_leave, char multicust_group_ip[15+1], u16* status)
		 if(M8266WIFI_SPI_Op_Multicuast_Group(0, "224.6.6.6", &status)==0)
     {
		   while(1)
		   {
      	  LED0(0);
	  LED1(0);			 
		   }
     }
     else

#elif (TEST_CONNECTION_TYPE == 2)          //// If TCP server, then tcp server auto disconnection timeout, and max clients allowed could be set	
		                                        //  (Chinese: ���TCP����������ô�����������TCP������(��ʱ����ͨ�Ŷ�)�Ͽ��ͻ��˵ĳ�ʱʱ��)
#if 1
		 //u8 M8266WIFI_SPI_Set_TcpServer_Auto_Discon_Timeout(u8 link_no, u16 timeout_in_s, u16* status)	
		 if( M8266WIFI_SPI_Set_TcpServer_Auto_Discon_Timeout(link_no, 120, &status) == 0)
     {
		   while(1)
		   {
     	  LED0(0);
	  LED1(0);		 
		   }
     }
#endif		 
#if 0
  		   //u8 M8266WIFI_SPI_Config_Max_Clients_Allowed_To_A_Tcp_Server(u8 server_link_no, u8 max_allowed, u16* status);
     else if( M8266WIFI_SPI_Config_Max_Clients_Allowed_To_A_Tcp_Server(link_no, 5, &status)==0)
		 {
		   while(1)
		   {
       #ifdef USE_LED_AND_KEY_FOR_TEST	// led flash in 1Hz when error
	        LED_set(0, 0); LED_set(1, 0); M8266WIFI_Module_delay_ms(1000);
	        LED_set(0, 1); LED_set(1, 1); M8266WIFI_Module_delay_ms(1000);
       #endif		 
		   }
		 }
#endif		 
		 else
#endif
		 //Setup Connection successfully (Chinese: �����׽������ӳɹ�)
     {
     	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle; LED1_Toggle; M8266WIFI_Module_delay_ms(50);	
	     	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle;LED1_Toggle;  M8266WIFI_Module_delay_ms(50);
	LED0_Toggle; LED1_Toggle; M8266WIFI_Module_delay_ms(50);	 
		 }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	 Communication Test (Chinese: WIFI�׽��ֵ������շ�ͨ�Ų���)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if (TEST_M8266WIFI_TYPE==3)  // Echo test: to receive data from remote and then echo back to remote (Chinese: �շ����ԣ�ģ�齫���յ����������̷��ظ����ͷ�)
{
#define  RECV_DATA_MAX_SIZE  128   

   u8  RecvData[RECV_DATA_MAX_SIZE];   // make sure the stack size is more than RECV_DATA_MAX_SIZE to avoid stack leak (Chinese: �����д����顣��ȷ����Ƭ���Ķ�ջ�㹻�����ٲ��������ݺͱ������ܳ��Ⱥͣ�����Ƭ��������ܻ�����������)
   u16 received = 0;
	 u16 sent;
	
	 for(i=0; i<RECV_DATA_MAX_SIZE; i++) RecvData[i]=i; 

   link_no = 0;
   sent = M8266WIFI_SPI_Send_Data(RecvData, 128, link_no, &status);

   while(1)
	 {
			if(M8266WIFI_SPI_Has_DataReceived()) //if received data (Chinese: ������յ�����)
			{
				//Receive the data (Chinese: ��Ƭ����������)
				//u16 M8266WIFI_SPI_RecvData(u8 data[], u16 max_len, uint16_t max_wait_in_ms, u8* link_no, u16* status);
	      received = M8266WIFI_SPI_RecvData(RecvData, RECV_DATA_MAX_SIZE, 5*1000, &link_no, &status);
				
				if(received!=0) //if received data length is not 0  (Chinese: �����Ƭ����ȷ���յ������ݣ������Ȳ�����0)
				{
					u16 tcp_packet_size = 1024;
          u16 loops     = 0;
		      u16 max_loops = 5000;
			    u32 len       = received; 
		     
          for(sent=0, loops=0; (sent<len)&&(loops<=max_loops); loops++)
          {		
				    sent += M8266WIFI_SPI_Send_Data(RecvData+sent, ((len-sent)>tcp_packet_size)?tcp_packet_size:(len-sent), link_no, &status);
            if(sent>=len)  break;
			      if((status&0xFF) == 0x00)
			      {
							 loops = 0;
			      }
		        else
			      {
				      if(   ((status&0xFF) == 0x14)      // 0x14 = connection of link_no not present (Chinese: ���׽��ֲ�����)
                 || ((status&0xFF) == 0x15) )    // 0x15 = connection of link_no closed(Chinese: ���׽����Ѿ��رջ�Ͽ�)
	            {
								 M8266HostIf_delay_us(99);
          			 //need to re-establish the socket connection (Chinese: ��Ҫ�ؽ������׽���)
	            }
							else if( (status&0xFF) == 0x18 )        // 0x18 = TCP server in listening states and no tcp clients have connected. (Chinese: ���TCP��������û�пͻ�����������)
			        {
				         M8266HostIf_delay_us(100);
			        }
	            else
	            {
	               M8266HostIf_delay_us(250);
	            }
			      }
          } // end of for(...
		    } // end of if(received!=0)
			}
		} // end of while(1)
}	 
 


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#else
#error NOT Supported Test Type! should be 1~4!		
#endif

} // end of M8266WIFI_Test
