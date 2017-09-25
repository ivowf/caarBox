#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
//#include "Serial_port.h"

//????????????
SYSTEMTIME sys_S, sys_R;
//extern bool isProcessComReq;

HANDLE Open_driver_T(char *name)

{
       //?????
	   //char *szPort = new char[50];
	   //szPort = name; 
	   HANDLE m_hCom = CreateFile(name,GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

       if(m_hCom == INVALID_HANDLE_VALUE)
       {
               printf("Create File failed\n");
               return NULL;
       }

       //???????????§³
       if(!SetupComm(m_hCom,1024,1024))
      {
             printf("SetupComm fail!\n");
             CloseHandle(m_hCom);
             return NULL;
      }

       //???¨®??
       COMMTIMEOUTS TimeOuts;
       memset(&TimeOuts,0,sizeof(TimeOuts));
       //TimeOuts.ReadIntervalTimeout = 50;
	   TimeOuts.ReadIntervalTimeout = 80;  //20160825.
       TimeOuts.ReadTotalTimeoutConstant = 200;
       //TimeOuts.ReadTotalTimeoutMultiplier = 100;
	   TimeOuts.ReadTotalTimeoutMultiplier = 5;  //20160825.
       TimeOuts.WriteTotalTimeoutConstant = 600;
       TimeOuts.WriteTotalTimeoutMultiplier = 10;

       SetCommTimeouts(m_hCom,&TimeOuts);
       PurgeComm(m_hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
       //??????????
       DCB dcb = {0};
       if (!GetCommState(m_hCom,&dcb))
       {
               printf("GetCommState fail\n");
               return NULL;
       }

       dcb.DCBlength = sizeof(dcb);
       if(!BuildCommDCB("38400,n,8,1",&dcb)) /*(!BuildCommDCB("115200,n,8,1",&dcb))*/
//????????????????????§µ???????????¦Ë????¦Ë
       {
               printf("BuileCOmmDCB fail\n");
               CloseHandle(m_hCom);
               return NULL;
       }

       if(SetCommState(m_hCom,&dcb))
       {
              printf("SetCommState OK!\n");
       }



	   /*Check if the band rate working*/
	   
	   
       return m_hCom;
}

 

//???????(§Õ????)--??????
unsigned char Send_driver_T(HANDLE fd, BYTE *data, DWORD dwExpectSend)
{

       DWORD dwError;
       //DWORD dwExpectSend = 20;
       DWORD dwRealSend = 0;
       BYTE *pSendBuffer;
	  

       pSendBuffer = data;
	  // if(isProcessComReq == false)
	   {
	   		//isProcessComReq = true;
		   //printf("/***********Send_driver_T, Begin***********/\r\n");
		   printf("\nsend bytes:   ");
		   for(unsigned int i =0; i< dwExpectSend; i++)
		   {
		   		printf(" %02X", pSendBuffer[i]);
		   }
		   printf("\r\n");

		   
		   

			
		   if(ClearCommError(fd,&dwError,NULL))
		   {
		          PurgeComm(fd,PURGE_TXABORT | PURGE_TXCLEAR);
		   }
		   GetLocalTime( &sys_S );
		   printf( "\nData sending @%d/%d/%d %d:%d:%d.%d\n"
			,sys_S.wYear,sys_S.wMonth,sys_S.wDay

			,sys_S.wHour,sys_S.wMinute,sys_S.wSecond,sys_S.wMilliseconds /*,sys.wDayOfWeek*/);
		   if(!WriteFile(fd,pSendBuffer,dwExpectSend ,&dwRealSend,NULL))
		   {    
		          //§Õ???????
		          printf("???????!\n");
		          return 1;
		   }
		   printf("dwRealSend = %d\r\n", dwRealSend);
		  // printf("/***********Send_driver_T, End***********/\r\n");
	   }
       return 0;
}

//3????????(??????)--??????
unsigned char Receive_driver_T(HANDLE fd, BYTE *data, unsigned int *readLen)
{

       DWORD dwError;
       DWORD dwWantRead = 267;
       DWORD dwRealRead = 0;
       BYTE* pReadBuf;

       unsigned int i;
	   unsigned short msdelay;
	  // SYSTEMTIME sys;
	   
    
      // printf("/**********Receive_driver_T, Begin**********/\r\n");
	   memset(data,0,sizeof(unsigned char)*267);
       pReadBuf = data;
       if (ClearCommError(fd,&dwError,NULL))
       {
              PurgeComm(fd,PURGE_TXABORT | PURGE_TXCLEAR);
       }
       if(!ReadFile(fd,pReadBuf,dwWantRead ,&dwRealRead,NULL))    //????????0 //??????0
       {
              return 1;
       }

	    GetLocalTime( &sys_R);
			 
		printf( "\nData received @%d/%d/%d %d:%d:%d.%d \n"
 
	 ,sys_R.wYear,sys_R.wMonth,sys_R.wDay
 
	 ,sys_R.wHour,sys_R.wMinute,sys_R.wSecond,sys_R.wMilliseconds);
		msdelay = (sys_R.wSecond - sys_S.wSecond)*1000 + (sys_R.wMilliseconds - sys_S.wMilliseconds);
		printf( "\n 2015 Time Delay is %d\n", msdelay);

       if(dwRealRead>0)
       	{
       		printf("recv_len = %d\n",dwRealRead);
			*readLen = dwRealRead;
       	}
	  

	  
	   

	   printf("Receive bytes:   ");
       for(i=0; i<dwRealRead; i++)
       {
              printf("%02X ",data[i]);
       }
       printf("\n");
	  // printf("/**********Receive_driver_T, End**********/\r\n");
	   //isProcessComReq = false;
       return 0;

}

 

//4????????

int Close_driver(void *fd)

{
       CloseHandle(fd);
       return 0;

}

