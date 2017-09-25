// socket_test_server.cpp : 定义控制台应用程序的入口点。
//


// socket.cpp : 定义控制台应用程序的入口点。
//
//服务器端


//SOCKET连接过程
//根据连接启动的方式以及本地套接字要连接的目标，套接字之间的连接过程可以分为三个步骤：服务器监听，客户端请求，连接确认。 　　
//服务器监听：是服务器端套接字并不定位具体的客户端套接字，而是处于等待连接的状态，实时监控网络状态。 　
//客户端请求：是指由客户端的套接字提出连接请求，要连接的目标是服务器端的套接字。
           //为此，客户端的套接字必须首先描述它要连接的服务器的套接字，指出服务器端套接字的地址和端口号，然后就向服务器端套接字提出连接请求。 　　
//连接确认：是指当服务器端套接字监听到或者说接收到客户端套接字的连接请求，它就响应客户端套接字的请求，建立一个新的线程，把服务器端套接字的描述发给客户端，一旦客户端确认了此描述，连接就建立好了。
              //而服务器端套接字继续处于监听状态，继续接收其他客户端套接字的连接请求。
  //如何开发一个Server-Client模型的程序


//开发原理： 　　
  //服务器，使用ServerSocket监听指定的端口，端口可以随意指定（由于1024以下的端口通常属于保留端口，在一些操作系统中不可以随意使用，所以建议使用大于1024的端口），等待客户连接请求，客户连接后，会话产生；在完成会话后，关闭连接。 　
  //　客户端，使用Socket对网络上某一个服务器的某一个端口发出连接请求，一旦连接成功，打开会话；会话完成后，关闭Socket。客户端不需要指定打开的端口，通常临时的、动态的分配一个1024以上的端口。 　
  //　Socket接口是TCP/IP网络的API，Socket接口定义了许多函数或例程，程序员可以用它们来开发TCP/IP网络上的应用程序。
    //要学Internet上的TCP/IP网络编程，必须理解Socket接口。Socket接口设计者最先是将接口放在Unix操作系统里面的。如果了解Unix系统的输入和输出的话，就很容易了解Socket了。
  //网络的Socket数据传输是一种特殊的I/O，Socket也是一种文件描述符。
  //Socket也具有一个类似于打开文件的函数调用Socket（），该函数返回一个整型的Socket描述符，随后的连接建立、数据传输等操作都是通过该Socket实现的。

//常用的Socket类型
　　//有两种：流式Socket（SOCK_STREAM）和数据报式Socket（SOCK_DGRAM）。
    //流式是一种面向连接的Socket，针对于面向连接的TCP服务应用；
    //数据报式Socket是一种无连接的Socket，对应于无连接的UDP服务应用。

//Socket 阻塞与非阻塞模式

//Windows套接字在阻塞和非阻塞两种模式下执行I/O操作。
//在阻塞模式下，在I/O操作完成前，执行的操作函数一直等候而不会立即返回，该函数所在的线程会阻塞在这里。
//相反，在非阻塞模式下，套接字函数会立即返回，而不管I/O是否完成，该函数所在的线程会继续运行。
//#pragma comment(lib, "Ws2_32.lib")
#include "stdafx.h"

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <WS2tcpip.h>
#include <process.h>
//#include <pthread.h>
#include <mstcpip.h>
#include <stdlib.h>


#include "socket_client.h"
#include "cardRead.h"
#include "Serial_port.h"

//#define CARDBOX_NET_PORT 9999  //9988 香港服务器设备运行端口
                               //9999 香港服务器代码测试端口
//using namespace std;

/*
 IMEI：0xA0
 GCI：0xB0
 IDReSult: 0xC0
 UEID： 0xD0
 FileID: 0x80
 FCP&DATA: 0xE0
 APDU: 0xF0
 
 
流量信息：0x20, 
电量信息：0x21
网络强度：0x22
用户登录信息:0x23

*/

#define FILEID   			0x80
#define IMEIID    			0xA0
#define GCI    				0xB0
#define IDRESULT    		0xC0
#define UEID    			0xD0
#define FCPDATA  			0xE0
#define APDU     			0xF0

#define FILEFCPID			0xC1
#define FILEDATAID			0xD1

#define PSDATAIND  			0x20
#define BATTARYIND 			0x21
#define NWRSSI     			0x22
#define USERLOGININGSTAUTS 	0x23

#define AUTO_GET_RESPONSE
//#define MIFI_3_VSIM /*2016.6.20, add by Xili for MIFI_3.0 vsim, begin*/


#define IDCHECKREQ    0x60
#define	LOADFILEREQ   0x61
#define	APDUREQ       0x62
#define	STATUSINDREQ  0x63
#define LOADALLFILESREQ 0x64
#define LOADALLFILESREQFOR30 0x67  //add by ml.20160728.
#define NETHEARTBEATIND  0xFE/*2016.03.15, mod by Xili for adding network heart beating, begin*/


#define SIMBANKREGREQ       0x6F
#define SIMBANKREGREQ2      0x7F
#define SIMBANKSTATUSREQ    0x7E
#define SIMBANKSTATUSRSP    0x7E
#define SIMBANKSTATUSIND    0x7E

#define COLDRESETSIMREQ     0x7D
#define COLDRESETSIMRSP     0x7D
#define HOTRESETSIMREQ      0x7C
#define HOTRESETSIMRSP      0x7C
#define UPDATESIMSTATUSREQ  0x7B
#define UPDATESIMSTATUSRSP  0x7B
#define DEACTIVESIMREQ      0x7A
#define DEACTIVESIMRSP      0x7A

#define READIMSIICCIDREQ      0x79
#define READIMSIICCIDRSP      0x79

#define UPDATESINGLESIMSTATUSREQ  0x78
#define UPDATESINGLESIMSTATUSRSP  0x78

#define UPDATEUSIMINFOIND   0x77


#define ALLDATAID	0xA1

#define USIMSTRUCTUREID 0xB2
#define IMSIID 			0xA2
#define ICCIDID         0xA3

#define SIMSTATSTRTAG      0xB3
#define SIMIDITAG  			0xD0



//#define min(x, y) ((x) <? (y))

//#define max(x, y) ((x) >? (y))




/*

typedef enum REQTYPE
{
	IDCHECKREQ   = 0x60,
	LOADFILEREQ  = 0x61,
	APDUREQ      = 0x62,
	STATUSINDREQ = 0x63
	
}ReqType;
*/



#pragma pack(1)

typedef struct TEIDREQ
{
	unsigned char head;
	unsigned char length[4];

	unsigned char reqtype;

	unsigned char imeiType;
	unsigned char imei[15];

	unsigned char gciType;
	unsigned char gci[5];

	unsigned char checkByte;
		
}TeIdReq;

typedef struct TEIDRSP
{
	unsigned char head;
	unsigned char length[4];

	unsigned char reqtype;

	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char idresulttype;
	unsigned char result;  //0x00 fail, 0x01 OK need load SIM files, 0x02 OK don't need load new SIM files
	
	char checkByte;
		
}TeIdRsp;



typedef struct TELOADFILEREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;

	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char fileIdType;
	unsigned char fileId[6];

	char checkByte;
		
}TeLoadFileReq;

typedef struct TELOADFILERSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char fcpDataType;
	unsigned char fCPDataLen[4];
	unsigned char fileType;
	unsigned char recordLen[2];
	unsigned char recordNum[2];
	/*
	unsigned char  fcpLen[2];
	unsigned char *fcp
	unsigned char  dataLen[2];
	unsigned char *data;
	unsigned char checkByte;
	*/
		
}TeLoadFileRsp;

/*2016.6.20, add by Xili for MIFI_3.0 vsim, begin*/
typedef struct TELOADFILECONTENTREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;

	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char fileIdType;
	unsigned char fileId[6];

	char checkByte;
		
}TeLoadFileContentReq;

typedef struct TELOADFILECONTENTRSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char simType;
	unsigned char allDataLen[4];
		
}TeLoadFileContentRsp;
/*2016.6.20, add by Xili for MIFI_3.0 vsim, end*/

typedef struct TEAPDUREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;

	unsigned char ueidtype;
	unsigned char ueid[4];
	unsigned char apduType;
	unsigned char apduLen[2];
	unsigned char apduRaw[262];
	unsigned char checkByte;
		
}TeApduReq;


typedef struct TEAPDURSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char ueidtype;
	unsigned char ueid[4];
	unsigned char apduType;
	unsigned char apduLen[2];
	/*
	unsigned char *apduRaw;
	unsigned char checkByte;
	*/
		
}TeApduRsp;

typedef struct TELOADALLFILESREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	
	unsigned char ueidtype;
	unsigned char ueid[4];
	unsigned char checkByte;
		
}TeLoadAllFilesReq;


typedef struct TELOADALLFILESRSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	
	unsigned char ueidtype;
	unsigned char ueid[4];

	unsigned char allDataType;
	unsigned char allDataLen[4];
		
}TeLoadAllFilesRsp;



typedef struct CARDBOXREGREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char regtype;
	unsigned char simNumAll[4];
	
	
}CardBoxRegReq;

typedef struct SIMINFOSTR
{
	unsigned char head;
	unsigned char simId[4];
	unsigned char usimStatus;
	unsigned char imsiId;
	unsigned char imsi[9];
	unsigned char iccidId;
	unsigned char iccid[10];
	
}SimInfoStr;

typedef struct CARDBOXREGRSP
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
     unsigned char simBankId[3];
     unsigned char checkingBytes;
 
}CardBoxRegRsp;




typedef struct SIMBANKPOLLUSIMSTATUSIND
{
	unsigned char head;
	unsigned char length[4];
	unsigned char regtype;
	unsigned char simNumTag;
	unsigned char simNumAll[4];
  //UsimStatusStr usimStatusStr[n]
  //unsigned char checkingBytes;
}SimBankPollUsimStatusInd;

 
typedef struct SERVERCOLDRESETSIMREQ
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
     unsigned char simIdTag;
     unsigned char simId[4];
     unsigned char checkingBytes;
}ServerColdResetSimReq;
 
typedef struct SERVERCOLDRESETSIMRSP
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
     unsigned char simIdTag;
     unsigned char simId[4];
     unsigned char atrTag;
     unsigned char atrLen;
//    unsigned char atr[n];
//    unsigned char checkingBytes;
}ServerColdResetSimRsp;
 
typedef struct SERVERHOTRESETSIMREQ
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
     unsigned char simIdTag;
     unsigned char simId[4];
     unsigned char checkingBytes;
}ServerHotResetSimReq;
 
typedef struct SERVERHOTRESETSIMRSP
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
     unsigned char simIdTag;
     unsigned char simId[4];
     unsigned char atrTag;
     unsigned char atrLen;
//    unsigned char atr[n];
//    unsigned char checkingBytes;
}ServerHotResetSimRsp;
 
 
typedef struct SERVERUPDATEUSIMSTATUSREQ
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
	 unsigned char simNumTag;
     unsigned char simNumAll[4];
   //UsimStatusStr usimStatusStr[n]
   //unsigned char checkingBytes;
}ServerUpdateUsimStatusReq;
 
typedef struct USIMSTATUSSTR
{
     unsigned char head;
     unsigned char simIdTag;
     unsigned char simId[4];
     unsigned char usimStatus;
 
}UsimStatusStr;
 
typedef struct SERVERUPDATEUSIMSTATUSRSP
{
     unsigned char head;
     unsigned char length[4];
     unsigned char regtype;
	 unsigned char simNumTag;
     unsigned char simNumAll[4];
   //UsimStatusStr usimStatusStr[n]
   //unsigned char checkingBytes;
}SeverUpdateUsimStatusRsp;


typedef struct SERVERUPDATESINGLEUSIMSTATUSREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char simIdTag;
	unsigned char simNumAll[4];
    unsigned char usimStatus;
}ServerUpdateSingleUsimStatusReq;

typedef struct SERVERUPDATESINGLEUSIMSTATUSRSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char simIdTag;
	unsigned char simNumAll[4];
    unsigned char usimStatus;
}SeverUpdateSingleUsimStatusRsp;


typedef struct SERVERREADIMSIICCIDREQ
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char simIdTag;
	unsigned char simId[4]; 
    unsigned char checkingBytes;
}ServerReadImsiIccidReq;



typedef struct SERVERREADIMSIICCIDRSP
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;
	unsigned char simIdTag;
	unsigned char simId[4]; 
	unsigned char imsiTag;
	unsigned char imsi[9];
	unsigned char iccidTag;
	unsigned char iccid[10];
    unsigned char checkingBytes;
}SeverReadImsiIccidRsp;

typedef struct SIMBANKUSIMINFOUPDATEIND
{
	unsigned char head;
	unsigned char length[4];
	unsigned char regtype;
	unsigned char simNumAll[4];
	
	
}SimBankUsimInfoUpdateInd;

/*2016.03.15, mod by Xili for adding network heart beating, begin*/
typedef struct SimBankSendHeartBeatReq
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;	
    unsigned char checkingBytes;
}SimBankSendHeartBeatReq;

typedef struct SimBankSendHeartBeatRsp
{
	unsigned char head;
	unsigned char length[4];
	unsigned char reqtype;	
    unsigned char checkingBytes;
}SimBankSendHeartBeatRsp;
/*2016.03.15, mod by Xili for adding network heart beating, end*/

//add by ml.20160928.
typedef struct SimBankGainOperationIpReq
{
    uchar head[2];  //0xAA 0x00
    uchar reserve;  //0x00
    uchar length[2];
    uchar reqtype;  //0x20
    uchar simbankidTag;  //0xDF
    uchar simbankid[3];
    uchar checkingBytes;
} SimBankGainOperationIpReq;

SimBankGainOperationIpReq *ptrSimBankGainOperationIpReq;

typedef struct SimBankGainOperationIpRsp
{
    uchar head[2];  //0xAA 0x00
    uchar reserve;  //0x00
    uchar length[2];
    uchar reqtype;  //0x20
    uchar ipTag;  //0xE0
    uchar dIp[4];
    uchar dPort[2];
    uchar bipTag;  //0xE0
    uchar bIp[4];
    uchar bPort[2];
    uchar checkingBytes;
} SimBankGainOperationIpRsp;

extern SimBankIpAddrRecord *pSimBankIpAddrRecord;

#pragma pack()

typedef struct TERMINALCLIENT
{
	unsigned char ueID;
	unsigned char IMEI[15];
	SOCKET termialSocket;
	clientStatus status;
		
}TermialClient;

#if 0
typedef struct RECVDATACACHE
{
	unsigned int length;
	unsigned int lastIndex;
	char data[MAXSENDNUM*2];
}RecvDataCache;

RecvDataCache recvDataCache;
#endif

//extern SimData simdata;
extern SimData *simdataHead ;
extern SimData *simdataCurr ;

TermialClient terminalClient[9];
unsigned char clientNum  = 0;
unsigned int thisSimBankId = 0;
volatile bool isServerConnected = false;
bool isProcessUeReq = false;

SOCKET sockClient;

unsigned char formerSimInfoStr[1000] = { 0 };  //36 SIM SimInfoStr length is 983

//bool isWaitingHeartBeatRsp = false;  /*2016.03.15, mod by Xili for adding network heart beat*/

//char sendbuffer[MAXSENDNUM];	//sendBuff	

//add by ml.20160805.
extern uchar pMaxSimNum;
extern ushort pServerNetPort;
extern uint pSimCardIdNum;

//mdf by ml.20160906.
//ushort sValidFlag = 0;
uchar gVlaidStatusFlag = 7;
//ushort sValidDataLen = 0;
uchar cRecvValidData[2048] = { 0 };
extern ushort *pValidFlag;
extern ushort *pValidDataLen;
extern uchar *pIsWaitingHeartBeatRsp;

unsigned int sendUsimstatusInd(void)
{
	char sendBuff[SENDRECVBUFSIZE];	//sendBuff	
	unsigned int bytesend;
	//printf("\nPlease input the bytes to send:\n");
	//scanf_s("%s",sendBuff,100);
	
    printf(" thisSimBankId = %d", thisSimBankId);
    
	/*Send Card box registration to server*/
	printf("\nSend sendUsimstatusInd to server:, the size of TeIdReq is: 6\n");
	SimBankPollUsimStatusInd simBankPollUsimStatusInd;
	unsigned int usimStrLoc;
	memset(&simBankPollUsimStatusInd, 0x00, sizeof(SimBankPollUsimStatusInd));
	memset(sendBuff, 0x00, SENDRECVBUFSIZE);
	simBankPollUsimStatusInd.head = 0xBB;
	simBankPollUsimStatusInd.regtype = SIMBANKSTATUSIND;
	simBankPollUsimStatusInd.simNumTag = 0x88;
	simBankPollUsimStatusInd.simNumAll[0] = (thisSimBankId&0xFF0000)>>16;
	simBankPollUsimStatusInd.simNumAll[1] = (thisSimBankId&0x00FF00)>>8;
	simBankPollUsimStatusInd.simNumAll[2] = (thisSimBankId&0x0000FF);
	//simBankPollUsimStatusInd.simNumAll[3] = ALLSIMNUM;
	simBankPollUsimStatusInd.simNumAll[3] = pMaxSimNum;  //mdf by ml.20160804.
	usimStrLoc = sizeof(SimBankPollUsimStatusInd); //the begin of USIMSTRCTURE
	UsimStatusStr usimStatusStr; 
	SimData *simData_tmp = simdataHead;
	/* head  SIMID ICCID_ID  ICCID[10] IMSI_ID IMSI[9] USIMSTATUS */
	while(simData_tmp != NULL)
	{
		memset(&usimStatusStr, 0x00, sizeof(UsimStatusStr));
		usimStatusStr.head = (unsigned char)SIMSTATSTRTAG;
		usimStatusStr.simIdTag = (unsigned char)SIMIDITAG;
		usimStatusStr.simId[3] = simData_tmp->simId;
		usimStatusStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
		memcpy(&sendBuff[usimStrLoc], &usimStatusStr, sizeof(UsimStatusStr));

		usimStrLoc += sizeof(UsimStatusStr);
		simData_tmp = simData_tmp->next_SIM;
	}
	
	
	sendBuff[usimStrLoc++] = (unsigned char)0xB2;  /*checking byte, usimStrLoc is the length of sendbuffer*/

	simBankPollUsimStatusInd.length[0] = ((usimStrLoc-5)&0xFF000000)>>24;
	simBankPollUsimStatusInd.length[1] = ((usimStrLoc-5)&0x00FF0000)>>16;
	simBankPollUsimStatusInd.length[2] = ((usimStrLoc-5)&0x0000FF00)>>8;
	simBankPollUsimStatusInd.length[3] = ((usimStrLoc-5)&0x000000FF);

	memcpy(sendBuff, &simBankPollUsimStatusInd, sizeof(simBankPollUsimStatusInd));
	printf("\n Card box registration is :");
	for(unsigned int i = 0; i< usimStrLoc; i++)
	{
		printf(" %02X", (unsigned char)sendBuff[i]);
	}
	logCurrentTime();
	bytesend = send(sockClient,sendBuff,usimStrLoc,0);//向服务器发送数据"This is Kary"
	printf("\n%d Bytes send\n", bytesend);
	return bytesend;
}

unsigned int sendUsimInfoUpdateInd(void)
{
	
	char sendBuff[SENDRECVBUFSIZE];	//sendBuff	
	unsigned int bytesend;
	unsigned int updatedSimNum = 0;
	bool usimStatusChanged = false;
	//printf("\nPlease input the bytes to send:\n");
	//scanf_s("%s",sendBuff,100);
	
   printf("\n thisSimBankId = %d", thisSimBankId);
		 
   /*Send Card box registration to server*/
	printf("\nSend sendUsimInfoUpdateInd to server:\n");
	SimBankUsimInfoUpdateInd simBankUsimInfoUpdateInd;
	unsigned int usimStrLoc;
	memset(&simBankUsimInfoUpdateInd, 0x00, sizeof(SimBankUsimInfoUpdateInd));
	memset(sendBuff, 0x00, SENDRECVBUFSIZE);
	simBankUsimInfoUpdateInd.head = 0xBB;
	simBankUsimInfoUpdateInd.regtype = UPDATEUSIMINFOIND;
	simBankUsimInfoUpdateInd.simNumAll[0] = (thisSimBankId&0xFF0000)>>16;
	simBankUsimInfoUpdateInd.simNumAll[1] = (thisSimBankId&0x00FF00)>>8;
	simBankUsimInfoUpdateInd.simNumAll[2] = (thisSimBankId&0x0000FF);
	//simBankUsimInfoUpdateInd.simNumAll[3] = updatedSimNum;
	usimStrLoc = sizeof(SimBankUsimInfoUpdateInd); //the begin of USIMSTRCTURE
	SimInfoStr simInfoStr; 
	bool imsiFound, iccidFound;
	SimData *simData_tmp = simdataHead;
	/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
	unsigned char iccidFileLoc = 0;
	unsigned char imsiFileLoc = 0;
	/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
	/* head  SIMID ICCID_ID  ICCID[10] IMSI_ID IMSI[9] USIMSTATUS */
	while(simData_tmp != NULL)
	{
		/*2016.06.04, mod by Xili for reducing the same usimStatus reported repeat*/
		printf("simData_tmp->pre_usimStatus:%d\n", simData_tmp->pre_usimStatus);
		printf("simData_tmp->usimStatus:%d\n", simData_tmp->usimStatus);
		if(simData_tmp->pre_usimStatus != simData_tmp->usimStatus)
		{
			usimStatusChanged = true;
			updatedSimNum++;
			memset(&simInfoStr, 0x00, sizeof(SimInfoStr));
			simInfoStr.head = USIMSTRUCTUREID;
			simInfoStr.simId[3] = simData_tmp->simId;
			simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
			simInfoStr.imsiId = IMSIID;
			simInfoStr.iccidId = ICCIDID;
			imsiFound = false;
			iccidFound = false;
			for(unsigned char simLoc = 0; simLoc < SIMFILENUM; simLoc++)
			{
				
				if((iccidFound == false)&&
					(simData_tmp->simfile[simLoc].fileId[0] == 0x2F )&&
					(simData_tmp->simfile[simLoc].fileId[1] == 0xE2 )
					)
				{
					printf("ICCID Found, copy it\n");
					if(simData_tmp->simfile[simLoc].data != NULL )
					memcpy(simInfoStr.iccid, simData_tmp->simfile[simLoc].data, min(10,simData_tmp->simfile[simLoc].dataLen));
					iccidFound = true;
					iccidFileLoc = simLoc;/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID*/
				}
				if((imsiFound == false)&&
					(simData_tmp->simfile[simLoc].fileId[0] == 0x6F )&&
					(simData_tmp->simfile[simLoc].fileId[1] == 0x07 ))
				{
					printf("IMSI Found, copy it\n");
					if(simData_tmp->simfile[simLoc].data != NULL )
					memcpy(simInfoStr.imsi, simData_tmp->simfile[simLoc].data, min(9,simData_tmp->simfile[simLoc].dataLen));
					imsiFound = true;
					imsiFileLoc = simLoc;/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID*/
				}

				if((imsiFound == true)
					&&(iccidFound == true))
				{
					break;
				}
					
			}
			if(iccidFound == false)
			{
				printf("ICCID not Found\n");
				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
				if(simData_tmp->usimStatus == USIM_ON)
				{
					simData_tmp->usimStatus = USIM_OFF;
					simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
					printf("Set usimstatus as USIM_OFF for no iccid found\n");
					setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
				}
				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
			}
			if(imsiFound == false)
			{
				printf("IMSI not Found\n");
				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
				if(simData_tmp->usimStatus == USIM_ON)
				{
					simData_tmp->usimStatus = USIM_OFF;
					simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
					printf("Set usimstatus as USIM_OFF for no IMSI found\n");
					setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
				}
				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
			}

			/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
			if(((iccidFileLoc != false) && (imsiFileLoc != false)) 
				&& (( simData_tmp->simfile[iccidFileLoc].dataLen == 0) 
				|| (simData_tmp->simfile[imsiFileLoc].dataLen == 0))
				&& (simData_tmp->usimStatus == USIM_ON))
			{
				simData_tmp->usimStatus = USIM_OFF;
				simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
				printf("Set usimstatus as USIM_OFF for iccid/imsi data length as 0\n");
				setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
			}
			/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
			
			/*copy the value to the sendbuffer*/
			memcpy(&sendBuff[usimStrLoc], &simInfoStr, sizeof(SimInfoStr));

			usimStrLoc += sizeof(SimInfoStr);

			simData_tmp->pre_usimStatus = simData_tmp->usimStatus;
			
		}
	    /*2016.06.04, mod by Xili for reducing the same usimStatus reported repeat, end*/
		simData_tmp = simData_tmp->next_SIM;
	}
	
	
	sendBuff[usimStrLoc++] = (unsigned char)0xB2;  /*checking byte, usimStrLoc is the length of sendbuffer*/
    simBankUsimInfoUpdateInd.simNumAll[3] = updatedSimNum;
	simBankUsimInfoUpdateInd.length[0] = ((usimStrLoc-5)&0xFF000000)>>24;
	simBankUsimInfoUpdateInd.length[1] = ((usimStrLoc-5)&0x00FF0000)>>16;
	simBankUsimInfoUpdateInd.length[2] = ((usimStrLoc-5)&0x0000FF00)>>8;
	simBankUsimInfoUpdateInd.length[3] = ((usimStrLoc-5)&0x000000FF);

	memcpy(sendBuff, &simBankUsimInfoUpdateInd, sizeof(SimBankUsimInfoUpdateInd));
	printf("\nUsim status info data is :");
	for(unsigned int i = 0; i< usimStrLoc; i++)
	{
		printf(" %02X", (unsigned char)sendBuff[i]);
	}

	logCurrentTime();
	//compare if the siminfostr is changed
	//if(memcmp(sendBuff, formerSimInfoStr, usimStrLoc) == 0)
	if(!usimStatusChanged)
	{
		printf("\n SIM status does not change");
	}
	else
	{
	    printf("\nSend sim status info data update indication");
		bytesend = send(sockClient,sendBuff,usimStrLoc,0);
		printf("\n%d Bytes send\n", bytesend);
		memcpy(formerSimInfoStr, sendBuff, usimStrLoc);
	}
	
	
	return bytesend;
}

unsigned char querySimFileIndex(SimData *simdataCurr_p, unsigned char *fileId)
{
    printf("querySimFileIndex, fileId[0]:%02x, fileId[1]:%02x\n", fileId[0], fileId[1]);
	for(unsigned char fileNum = 0; fileNum < SIMFILENUM; fileNum++)
	{
		if((simdataCurr_p->simfile[fileNum].fileId[0] == fileId[0]) 
			&&(simdataCurr_p->simfile[fileNum].fileId[1] == fileId[1]))
		{
			printf("querySimFileIndex, get the file index:%d\n", fileNum);
			return fileNum;
		}
	}
	printf("querySimFileIndex, no such file found!!!\n"); 
}

/*Process the termial Request */
unsigned int processTerminalReq(char *recvbuf, SOCKET sockConn)
{
	char sendbuffer[MAXSENDNUM];
	unsigned int result;
	SimData *simData_tmp;
	unsigned char simID = 0;
	bool simFound = false;
	uchar isWaitingHeartBeatRsp = *pIsWaitingHeartBeatRsp;
	
	printf("\r\n%s->%s %d: Begin.", __FILE__, __FUNCTION__, __LINE__);

    /*2016.03.15, mod by Xili for adding network heart beating, begin*/
	if((unsigned char)recvbuf[5] == NETHEARTBEATIND)
	{
		SimBankSendHeartBeatRsp *simBankSendHeartBeatRsp = NULL;			
		printf("\n Receive Heart beat packet response from server:");
		simBankSendHeartBeatRsp = (SimBankSendHeartBeatRsp *)recvbuf;
		if(simBankSendHeartBeatRsp->length[3] == 0x02)
		{
			printf("the data length is right\n");
			isWaitingHeartBeatRsp = 0;
            *pIsWaitingHeartBeatRsp = isWaitingHeartBeatRsp;
		}
		else
		{
			printf("the data length is not right\n");
		}

		return 1;
	}
    
    if (SIMBANKREGREQ == (unsigned char)recvbuf[5])
    {
        printf("\n SIMBANKREGREQ handle.");
        printf("\r\n%s->%s %d: End.SIMBANKREGREQ.", __FILE__, __FUNCTION__, __LINE__);
        
        return 1;
    }
	/*2016.03.15, mod by Xili for adding network heart beating, end*/
	
	
	/*Got the UEID from the req*/	
	simID = (unsigned char)recvbuf[10];
	simData_tmp = simdataHead;
	while((simData_tmp != NULL)&&(simFound == false))
	{
		if(simData_tmp->simId == simID)
		{
			printf("\n SIM found, simID is %d", simID);
			//simdataCurr = simData_tmp;
			while(simData_tmp->isProcessStatuscmd == true)
			{
				printf("\n%d SIM is on Status checking", simID);
				Sleep(50); //80ms
			}
			printf("\n%d SIM is out of Status checking", simID);
			simFound = true;
			break;
		}
		simData_tmp = simData_tmp->next_SIM;
	}
	if(simFound == true)
	{
		simData_tmp->isProcessNetcmd = true;
		switch((unsigned char)recvbuf[5])
 		{
 			case IDCHECKREQ:  /*Check ID*/
	 		{
	 			printf("\n IDCHECKREQ");
	 			TeIdReq *teIdReq = NULL;
	 			teIdReq = (TeIdReq *)recvbuf;
	 			if(teIdReq->imeiType == IMEIID)
	 			{
	 				/*Check if IMEI valid*/
	 				
	 				if(teIdReq->gciType== GCI)
	 				{
	 					unsigned char mcc[3];
	 					unsigned char mnc[3];
	 					
	 					unsigned short MCC = 0;
	 					unsigned short MNC = 0;
	 					//unsigned int ueid;
	 					mcc[0] = (teIdReq->gci[0]&0x0F);
	 					mcc[1] = (teIdReq->gci[0]&0xF0)>>4;
	 					mcc[2] = (teIdReq->gci[1]&0x0F);
	 					
	 					MCC = mcc[0]*256+mcc[1]*16+mcc[2];
	 					mnc[0] = (teIdReq->gci[1]&0xF0)>>4;
	 					mnc[1] = (teIdReq->gci[2]&0x0F);
	 					mnc[2] = (teIdReq->gci[2]&0xF0)>>4;
	 					MNC = mnc[0]*256+mnc[1]*16+mnc[2];
	 
	 					printf("\n processTerminalReq, MCC =%d, MNC =%d", MCC, MNC);
	 
	 					if(MCC == simData_tmp->hplmn.MCC)
	 					{
	 						printf("\n This is the same country card");
	 						/*contruct the response structure*/
	 						TeIdRsp teIdRsp;
	 						unsigned int byteSends;
	 						memset(&teIdRsp, 0x00, sizeof(TeIdRsp));
	 						
	 						teIdRsp.head = teIdReq->head;
	 						teIdRsp.length[3] = sizeof(TeIdRsp) - 5;
	 						
	 						teIdRsp.reqtype = teIdReq->reqtype;
	 												
	 						teIdRsp.idresulttype 	= IDRESULT;
	 						teIdRsp.result 			= 0x01; //OK
	 
	 						teIdRsp.ueidtype  		= UEID;
	 						/*generate the */
							teIdRsp.ueid[0] = (thisSimBankId&0xff0000)>>16;
							teIdRsp.ueid[1] = (thisSimBankId&0x00ff00)>>8;
							teIdRsp.ueid[2] = (thisSimBankId&0x0000ff);
	 						teIdRsp.ueid[3]			= 0x01;
	 
	 						/*generate the checking bit*/
	 						
	 						memset(sendbuffer, 0x00, MAXSENDNUM);
	 						memcpy(sendbuffer, &teIdRsp, sizeof(TeIdRsp));
	 
	 						printf("\n IDCHECKRSP is :");
	 						for(unsigned int i = 0; i< sizeof(TeIdRsp); i++)
	 						{
	 							printf(" %02X", (unsigned char)sendbuffer[i]);
	 						}
	 
	 						printf("%d bytes send to the Terminal:", sizeof(TeIdRsp));
	 						logCurrentTime();
	 						byteSends = send(sockConn,sendbuffer,sizeof(TeIdRsp),0);
	 						if(byteSends == sizeof(TeIdRsp))
	 						{
	 							printf("\n IDCHECKRSP send OK");
	 							result = 0x01;
	 						}
	 						
	 					}
	 					else
	 					{
	 						printf("\n MCC dismatch, reject");
	 						result = 0x00;
	 					}
	 				}
	 				else
	 				{
	 					printf("\n GCI dismatch, reject");
	 					result = 0x00;
	 				}
	 			}
	 			else
	 			{
	 				printf("\n IMEI dismatch, reject");
	 				result = 0x00;
	 			}
	 		
	 		}
 		break;
 		case LOADFILEREQ:  /*Load File Req*/
 		{
 			printf("\n LOADFILEREQ");
 			TeLoadFileReq *teLoadFileReq = NULL;
 			teLoadFileReq = (TeLoadFileReq *)recvbuf;
 			if(teLoadFileReq->ueidtype == UEID)
 			{
 				/*check if the ueid is valid*/
 				/*UE ID is contains the MCC and SIM ID*/
 
 				if(teLoadFileReq->fileIdType == FILEID)
 				{
 					/*just check the lower two bytes of the fileID*/
 
 					for(unsigned char i=0; i<SIMFILENUM; i++)
 					{
 						if((teLoadFileReq->fileId[4] == simData_tmp->simfile[i].fileId[0])
 							&&(teLoadFileReq->fileId[5] == simData_tmp->simfile[i].fileId[1]))
 						{
 							/*we got the file*/
 
 							printf("\n LOADFILEREQ, we got the right file ID, this is %dst file", i);
 							/*contruct the response structure*/
 							TeLoadFileRsp teLoadFileRsp;
 							unsigned int byteSends;
 							/*It is the begging bytes of FCP length*/
 							unsigned char  fcp_loc;
 							unsigned short data_loc;
 							unsigned int length;
 							unsigned int fcpAndDataLen;
 							
 							
 							length = sizeof(TeLoadFileRsp)+simData_tmp->simfile[i].fcpLen + simData_tmp->simfile[i].dataLen+5; 
 							memset(&teLoadFileRsp, 0x00, sizeof(TeLoadFileRsp));
 							teLoadFileRsp.head = teLoadFileReq->head;
 							teLoadFileRsp.length[2] = ((length-5)&&0xFF00)>>8;
 							teLoadFileRsp.length[3] = ((length-5)&&0x00FF);
 							
 							teLoadFileRsp.reqtype = teLoadFileReq->reqtype;
 
 							teLoadFileRsp.ueidtype  		= UEID;
 							memcpy(teLoadFileRsp.ueid, teLoadFileReq->ueid, 0x04);
 							
 							
 							/*generate the checking bit*/
 							teLoadFileRsp.fcpDataType = FCPDATA;
 							/*Filetye, recordLen[2], recordNum[2], fcpLen[2], *fcp, dataLen[2], *data*/
 							fcpAndDataLen = 5+2+simData_tmp->simfile[i].fcpLen + 2+ simData_tmp->simfile[i].dataLen;
 							teLoadFileRsp.fCPDataLen[0] = (fcpAndDataLen & 0xFF000000)>>24;
 							teLoadFileRsp.fCPDataLen[1] = (fcpAndDataLen & 0x00FF0000)>>16;
 							teLoadFileRsp.fCPDataLen[2] = (fcpAndDataLen & 0x0000FF00)>>8;
 							teLoadFileRsp.fCPDataLen[3] = (fcpAndDataLen & 0x000000FF);
 							
 							teLoadFileRsp.fileType  = (unsigned char)simData_tmp->simfile[i].fileType;
 							teLoadFileRsp.recordLen[0] = (simData_tmp->simfile[i].recordLen & 0xFF00)>>8;
 							teLoadFileRsp.recordLen[1] = (simData_tmp->simfile[i].recordLen & 0x00FF);
 							
 							
 							teLoadFileRsp.recordNum[1] = simData_tmp->simfile[i].recordNum; //char
 							
 							memset(sendbuffer, 0x00, MAXSENDNUM);
 							memcpy(sendbuffer, &teLoadFileRsp, sizeof(TeLoadFileRsp));
 							/*FCPLen[2]  FCP, DATALen[2], DATA, checkBytess*/
 							//FCP
 							fcp_loc = sizeof(teLoadFileRsp);
 							sendbuffer[fcp_loc] = (simData_tmp->simfile[i].fcpLen&0xFF00)>>8;
 							sendbuffer[fcp_loc+1] = (simData_tmp->simfile[i].fcpLen&0x00FF);
 							memcpy(&sendbuffer[fcp_loc+2], simData_tmp->simfile[i].fcp, simData_tmp->simfile[i].fcpLen);
 							//data
 							data_loc = fcp_loc+2+simData_tmp->simfile[i].fcpLen;
 							if((simData_tmp->simfile[i].dataLen & 0x00FF0000) > 0)
 							{
 								printf("\n error There is a file datalen bigger than 65535!!!!\n");
 							}
 							sendbuffer[data_loc] = (simData_tmp->simfile[i].dataLen & 0x0000FF00)>>8;
 							sendbuffer[data_loc+1] = (simData_tmp->simfile[i].dataLen & 0x000000FF);
 							memcpy(&sendbuffer[data_loc+2], simData_tmp->simfile[i].data, simData_tmp->simfile[i].dataLen);
 							
 							/*calculate the checkbytes*/
 
 							printf("\n LOADFILERSP is :");
 							for(unsigned int i = 0; i< length; i++)
 							{
 								printf(" %02X", (unsigned char)sendbuffer[i]);
 							}
 						
 							printf("\n %d bytes send to the Terminal:", length);
 							logCurrentTime();
 							byteSends = send(sockConn,sendbuffer,length,0);
 							if(byteSends == length)
 							{
 								printf("\n LOADFILERSP send OK");
 								result = 0x01;
 							}
 							else
 							{
 								printf("\n LOADFILERSP not all the data send out");
 								result = 0x00;
 							}
 							break;
 						}
 					}
 				}
 				else
 				{
 					printf("\n LOADFILERSP , fileID key word error");
 					result = 0x00;
 				}
 			}
 			else
 			{
 				printf("\n LOADFILERSP, UEID key word error");
 				result = 0x00;
 			}
 		}
 			break;
		/*2016.6.20, add by Xili for MIFI_3.0 vsim, begin*/
		//#ifdef MIFI_3_VSIM
        //case LOADALLFILESREQ://to be defined
        case LOADALLFILESREQFOR30:  //mdy by ml.20160728.
		{
 			printf("\n LOADALLFILESREQFOR30");
			unsigned int allFileDataLen = 0; 
			unsigned char fileId[2] = {0};
			unsigned char fileIndex = 0;
 			TeLoadFileContentReq *teLoadFileContentReq = NULL;
 			teLoadFileContentReq = (TeLoadFileContentReq *)recvbuf;
 			if(teLoadFileContentReq->ueidtype == UEID)
 			{
 				/*get the right SIM from UEID*/
 				TeLoadFileContentRsp teLoadFileContentRsp;
 				unsigned int byteSends;
 				unsigned int filePackage_loc;
 				memset(&teLoadFileContentRsp, 0x00, sizeof(TeLoadFileContentRsp));
 
 				teLoadFileContentRsp.head = teLoadFileContentReq->head;
 				
 				teLoadFileContentRsp.reqtype = teLoadFileContentReq->reqtype;
 
 				teLoadFileContentRsp.ueidtype = teLoadFileContentReq->ueidtype;
 				memcpy(teLoadFileContentRsp.ueid, teLoadFileContentReq->ueid, 4);
                if(simData_tmp->isUsim == true)
                {
	                teLoadFileContentRsp.simType = 1;
                }
				else
				{
					teLoadFileContentRsp.simType = 0;
				}				
 
 				
 				memset(sendbuffer, 0x00, MAXSENDNUM);
 				/*********the following cmd structure************/
 				
 				filePackage_loc = sizeof(teLoadFileContentRsp); /*the begining of the first files*/
 				//unsigned int fileStrLen;
				
				/*********Add EF IMSI data ************/
				printf("\nLOADFILESCONTENTREQ: add EF IMSI data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x07;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF LOCI data ************/
				printf("\nLOADFILESCONTENTREQ: add EF LOCI data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x7E;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}


				/*********Add EF SST/UST data ************/
				printf("\nLOADFILESCONTENTREQ: add EF SST/UST data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x38;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF KC(SIM)/KEYS(USIM) data ************/
				printf("\nLOADFILESCONTENTREQ: add EF KC(SIM)/KEYS(USIM) data\n");
				if(simData_tmp->isUsim == true)
				{
					fileId[0] = 0x6F;
					fileId[1] = 0x08;
				}
				else
				{
					fileId[0] = 0x6F;
					fileId[1] = 0x20;
				}
				
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF ACC data ************/
				printf("\nLOADFILESCONTENTREQ: add EF ACC data\n");
				
				fileId[0] = 0x6F;
				fileId[1] = 0x78;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}
				

				/*********Add EF FPLMN data ************/
				printf("\nLOADFILESCONTENTREQ: add EF FPLMN data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x7B;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF AD data ************/
				printf("\nLOADFILESCONTENTREQ: add EF AD data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0xAD;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF ICCID data ************/
				printf("\nLOADFILESCONTENTREQ: add EF ICCID data\n");
				fileId[0] = 0x2F;
				fileId[1] = 0xE2;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				/*********Add EF PLMNWACT data ************/
				printf("\nLOADFILESCONTENTREQ: add EF PLMNWACT data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x60;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}
            #if 0
				/*********Add EF OPLMNWACT data ************/
				printf("\nLOADFILESCONTENTREQ: add EF OPLMNWACT data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x61;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}
            #endif
				/*********Add EF HPLMNWACT data ************/
				printf("\nLOADFILESCONTENTREQ: add EF HPLMNWACT data\n");
				fileId[0] = 0x6F;
				fileId[1] = 0x62;
				fileIndex = querySimFileIndex(simData_tmp, fileId);
				if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
				{
					sendbuffer[filePackage_loc++] = fileId[0];
					sendbuffer[filePackage_loc++] = fileId[1];
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
					sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
					memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
					filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
				}

				if(simData_tmp->isUsim == true)
				{
					/*********Add EF KEYSPS data ************/
					printf("\nLOADFILESCONTENTREQ: add EF KEYSPS data\n");
					fileId[0] = 0x6F;
					fileId[1] = 0x09;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}
					
					/*********Add EF EPSLOCI data ************/
					printf("\nLOADFILESCONTENTREQ: add EF EPSLOCI data\n");
					fileId[0] = 0x6F;
					fileId[1] = 0xE3;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}

					/*********Add EF EPSNSC data ************/
					printf("\nLOADFILESCONTENTREQ: add EF EPSNSC data\n");
					fileId[0] = 0x6F;
					fileId[1] = 0xE4;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}

                    //add by ml.20160808.
                    printf("\nLOADALLFILESREQFOR30: 0x6F & 0x56.\n");
					fileId[0] = 0x6F;
					fileId[1] = 0x56;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}
				}
				else/*for 2G SIM*/
				{
					/*********Add EF PLMNSEL data ************/
					printf("\nLOADFILESCONTENTREQ: add EF PLMNSEL data\n");
					fileId[0] = 0x6F;
					fileId[1] = 0x30;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}

					/*********Add EF PHASE data ************/
					printf("\nLOADFILESCONTENTREQ: add EF PHASE data\n");
					fileId[0] = 0x6F;
					fileId[1] = 0xAE;
					fileIndex = querySimFileIndex(simData_tmp, fileId);
					if((fileIndex != SIMFILENUM) && (simData_tmp->simfile[fileIndex].dataLen != 0))
					{
						sendbuffer[filePackage_loc++] = fileId[0];
						sendbuffer[filePackage_loc++] = fileId[1];
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0xFF000000)>>24;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x00FF0000)>>16;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x0000FF00)>>8;
						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileIndex].dataLen & 0x000000FF);
						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileIndex].data, simData_tmp->simfile[fileIndex].dataLen);
						filePackage_loc += simData_tmp->simfile[fileIndex].dataLen;
					}
				}			
 				
 				/*File contruct end*/
 				sendbuffer[filePackage_loc++] = (char )0xB2; /*Check bytes, filePackage_loc is strucure length*/
 				
 				teLoadFileContentRsp.length[0] = ((filePackage_loc - 5)&0xFF000000)>>24;
 				teLoadFileContentRsp.length[1] = ((filePackage_loc - 5)&0x00FF0000)>>16;
 				teLoadFileContentRsp.length[2] = ((filePackage_loc - 5)&0x0000FF00)>>8;
 				teLoadFileContentRsp.length[3] = ((filePackage_loc - 5)&0x000000FF);
 				
 				allFileDataLen = filePackage_loc - sizeof(TeLoadAllFilesRsp) -1;  //without the checking bytes
 
 				teLoadFileContentRsp.allDataLen[0] = (allFileDataLen&0xFF000000)>>24;
 				teLoadFileContentRsp.allDataLen[1] = (allFileDataLen&0x00FF0000)>>16;
 				teLoadFileContentRsp.allDataLen[2] = (allFileDataLen&0x0000FF00)>>8;
 				teLoadFileContentRsp.allDataLen[3] = (allFileDataLen&0x000000FF);
 				
 				memcpy(sendbuffer, &teLoadFileContentRsp, sizeof(teLoadFileContentRsp));
 				printf("\n LOADALLFILESREQ RSP is :");
 				for(unsigned int i = 0; i< filePackage_loc; i++)
 				{
 					printf(" %02X", (unsigned char)sendbuffer[i]);
 				}

				
 				printf("\n LOADALLFILESREQ, send LOADALLFILESREQ RSP back to Terminal");
 
 				unsigned int allByteSend = 0;
				
 				while(allByteSend < filePackage_loc)
 				{
 					logCurrentTime();
 					byteSends = send(sockConn,&sendbuffer[allByteSend],min(SENDRECVBUFSIZE, filePackage_loc-allByteSend),0);
 					//byteSends = send(sockConn,sendbuffer,filePackage_loc,0);
 					if(byteSends > 0)
 					{
 						printf("\n %d Bytes send OK", byteSends);
 						allByteSend += byteSends;
 						printf("\n All %d Bytes send OK", allByteSend);
 						result = 0x01;
 					}
 					else
 					{
 						printf("\n APDUREQ not all the data send out");
 						break;
 						result = 0x00;
 					}
 				}
 				
 			}
 			else
 			{
 				printf("\n LOADFILERSP, UEID key word error");
 				result = 0x00;
 			}
 		}
		    break;
		//#endif  //MIFI_3_VSIM
		/*2016.6.20, add by Xili for MIFI_3.0 vsim, end*/
 		case APDUREQ:  /*APDU REQ*/
 		{
 			printf("\nAPDUREQ");
			//isProcessUeReq = true; 
 			TeApduReq *teApduReq = NULL;
 			teApduReq = (TeApduReq *)recvbuf;
 			if(teApduReq->ueidtype == UEID)
 			{
 				/*Check if the UEID is valid*/
 				if(teApduReq->apduType == APDU)
 				{
 					unsigned char apduLen = 0; 
 					unsigned short receApdulength = 0;
 					apduLen = (((unsigned char)teApduReq->apduLen[0] << 8)|
 								((unsigned char)teApduReq->apduLen[1]));
 			
 					printf("\n APDUREQ, APDU REQ is :");
 					for(unsigned int i = 0; i< apduLen; i++)
 					{
 						printf(" %02X", teApduReq->apduRaw[i]);
 					}
 					printf("\n APDUREQ, send the APDU to SIM card");
 					
 					receApdulength = processAPDUCmd(teApduReq->apduRaw, apduLen, simData_tmp);
 					/*after SIM card process the cmd, simdataCurr->responseApdu contains the RSP APDU*/
 					/*contruct the response structure*/
 					TeApduRsp teApduRsp;
 					unsigned int byteSends;
 					memset(&teApduRsp, 0x00, sizeof(TeApduRsp));
 					teApduRsp.head = teApduReq->head;
 					teApduRsp.length[2] = ((sizeof(teApduRsp)-5+receApdulength+1)&0xFF00)>>8;
 					teApduRsp.length[3] = ((sizeof(teApduRsp)-5+receApdulength+1)&0x00FF);
 
 					teApduRsp.reqtype = teApduReq->reqtype;
 					
 					teApduRsp.ueidtype  		= UEID;
 					memcpy(teApduRsp.ueid, teApduReq->ueid, 0x04);
 
 					/*copy the APDU RSP*/
 					teApduRsp.apduType = APDU;
 					teApduRsp.apduLen[0] = (receApdulength&0xFF00)>>8;
 					teApduRsp.apduLen[1] = (receApdulength&0x00FF);
 
 					memset(sendbuffer, 0x00, MAXSENDNUM);
 					memcpy(sendbuffer, &teApduRsp, sizeof(TeApduRsp));
 					if(receApdulength <= 262)
 					{
 						printf("\n APDUREQ, %d bytes APDU is returned", receApdulength);
 						memcpy(&sendbuffer[sizeof(TeApduRsp)], simData_tmp->responseApdu, receApdulength);
 					}
 					else
 					{
 						printf("\n APDUREQ, more than 262 bytes APDU is returned!!!!!!!!!");
 					}
 
 					printf("\n APDUREQ, APDU RSP is :");
 					for(unsigned int i = 0; i< receApdulength; i++)
 					{
 						printf(" %02X", (unsigned char)sendbuffer[sizeof(TeApduRsp)+i]);
 					}
 					printf("\n APDUREQ, send the APDU RSP back to Terminal");
 
 					
 
 					printf("\n APDURSP is :");
 					for(unsigned int i = 0; i< (sizeof(TeApduRsp)+receApdulength+1); i++)
 					{
 						printf(" %02X", (unsigned char)sendbuffer[i]);
 					}
					logCurrentTime();
 					byteSends = send(sockConn,sendbuffer,(sizeof(TeApduRsp)+receApdulength+1),0);
 					if(byteSends == (sizeof(TeApduRsp)+receApdulength+1))
 					{
 						printf("\n APDUREQ send OK");
 						result = 0x01;
 					}
 					else
 					{
 						printf("\n APDUREQ not all the data send out");
 						result = 0x00;
 					}
 				}
 				else
 				{
 					printf("\n APDUREQ, APDU key word error");
 					result = 0x00;
 				}
 			
 			}
 			else
 			{
 				printf("\n APDUREQ, UEID key word error");
 				result = 0x00;
 			}
 			
 		}
 			break;
 		case STATUSINDREQ:  /*STATUSINDREQ*/
 			printf("\n STATUSINDREQ");
 			break;
 		case LOADALLFILESREQ:
 		{
 			printf("\n LOADALLFILESREQ");
 			printf("\nAPDUREQ");
 			unsigned int allDataLen = 0; 
 			TeLoadAllFilesReq *teLoadAllFilesReq_p = NULL;
 			teLoadAllFilesReq_p = (TeLoadAllFilesReq *)recvbuf;
 			if(teLoadAllFilesReq_p->ueidtype == UEID)
 			{
 				/*get the right SIM from UEID*/
 				TeLoadAllFilesRsp teLoadAllFilesRsp;
 				//unsigned int byteSends;
                int byteSends;  //mdf by ml.20160816.
 				unsigned int filePackage_loc;
 				memset(&teLoadAllFilesRsp, 0x00, sizeof(TeLoadAllFilesRsp));
 
 				teLoadAllFilesRsp.head = teLoadAllFilesReq_p->head;
 				//teLoadAllFilesRsp.length[0] = 222 ;  /*....*/
 				teLoadAllFilesRsp.reqtype = teLoadAllFilesReq_p->reqtype;
 
 				teLoadAllFilesRsp.ueidtype = teLoadAllFilesReq_p->ueidtype;
 				memcpy(teLoadAllFilesRsp.ueid, teLoadAllFilesReq_p->ueid, 4);
 
 				teLoadAllFilesRsp.allDataType = ALLDATAID;
 				
 
 				
 				memset(sendbuffer, 0x00, MAXSENDNUM);
 				/*********the following cmd structure************/
 				/*  ALLDATA  ALLDATALEN  filestructure[n]*/
 				/*filestructure[n]: filePackage Length[4] DF[4] FileId[2] fileType recoredLen recordNum FCP fcpLen fcp[] Data DataLen data[]*/
 				/*
 				filePackage: 0xB1开头，分隔文件
 				Length：文件长度。4个字节，大端模式。
 				DF：所在文件夹名字。4个字节
 				FILEID：文件ID，2个字节
 				如果该文件为MF，DF值为0x00,0x00,0x00,0x00,FILEID为0x3f,0x00
 				如果该文件为MF的下级文件或目录，DF值为0x00,0x00,0x3f,0x00,FILEID为实际ID，如0x2f,0x05
 				如果该文件为MF的二级级文件或目录，DF值为0x3f, 0x00,0x7f,0x00,FILEID为实际ID，如0x6F,0x2c
 				fileType：文件类型，1个字节0-- MF, 1--DF, 2--ADF, 3--EF TR,4-- EF LF, 5--EF CY
 				recordLen：记录长度。4个字节，大端模式。
 				recordNum：记录个数。2个字节，大端模式
 					FCP：FCP数据，0xC1
 				fcpLen:FCP数据的长度，4个字节，大端模式。
 					fcp[]：FCP数据，如果fcpLen为0，则该值不存在
 					DATA：数据内容。0xD1
 					dataLen：内容长度，4个字节，大端模式。
 					data[]：数据内容。如果dataLen为0，则该值不存在
 				*/
 				filePackage_loc = sizeof(teLoadAllFilesRsp); /*the begining of the first files*/
 				unsigned int fileStrLen;
 				for(unsigned char fileNum = 0; fileNum < SIMFILENUM; fileNum++)
 				{
 					if((simData_tmp->simfile[fileNum].fcpLen !=0)
 						||(simData_tmp->simfile[fileNum].dataLen!=0))
 					{
 						printf("\n File exit, file ID is 0x%02x %02x ", simData_tmp->simfile[fileNum].fileId[0], simData_tmp->simfile[fileNum].fileId[1]);
 						printf("\n @file begin, filePackage_loc is %d", filePackage_loc );
 						sendbuffer[filePackage_loc++] = (unsigned char)0xB1;
 					             /*DF[6] Ftype rLen[2]rNum[2] Fcp  FLen[2] + fcp[] DATA datalen data[]*/
 						fileStrLen = 6 +  1 +  2 +  2  + 1 + 2 +simData_tmp->simfile[fileNum].fcpLen+1+2+simData_tmp->simfile[fileNum].dataLen;
 						printf("\n fileStrLen is %d, filestruture len is: ", fileStrLen, fileStrLen+5);
 						sendbuffer[filePackage_loc++] = (fileStrLen&0xFF000000)>>24;
 						sendbuffer[filePackage_loc++] = (fileStrLen&0x00FF0000)>>16;
 						sendbuffer[filePackage_loc++] = (fileStrLen&0x0000FF00)>>8;
 						sendbuffer[filePackage_loc++] = (fileStrLen&0x000000FF);
 
 						if((simData_tmp->simfile[fileNum].currentDF[0]==0x00)&&
 							(simData_tmp->simfile[fileNum].currentDF[1]==0x00))
 						{
 							/*This is MF*/
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = 0x00;
 						}
 						else if((simData_tmp->simfile[fileNum].currentDF[0]==0x3F)&&
 							(simData_tmp->simfile[fileNum].currentDF[1]==0x00))
 						{
 							/*This is files or ADF ADF unsder MF*/
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = 0x3F;
 							sendbuffer[filePackage_loc++] = 0x00;
 						}
 						else /*Files under ADF DF*/
 						{
 							sendbuffer[filePackage_loc++] = 0x3F;
 							sendbuffer[filePackage_loc++] = 0x00;
 							sendbuffer[filePackage_loc++] = simData_tmp->simfile[fileNum].currentDF[0];
 							sendbuffer[filePackage_loc++] = simData_tmp->simfile[fileNum].currentDF[1];
 						}
 							
 						sendbuffer[filePackage_loc++] = simData_tmp->simfile[fileNum].fileId[0];
 						sendbuffer[filePackage_loc++] = simData_tmp->simfile[fileNum].fileId[1];
 						/*fileType recoredLen[2] recordNum[2] FCP fcpLen[4] fcp[] Data DataLen[4] data[]*/
 						sendbuffer[filePackage_loc++] = simData_tmp->simfile[fileNum].fileType;	
 						//recordLen[2]
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].recordLen&0xFF00)>>8;
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].recordLen&0x00FF);
 						//recordNum
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].recordNum&0xFF00)>>8;
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].recordNum&0x00FF);
   						//FCP
   						sendbuffer[filePackage_loc++] = (unsigned char)FILEFCPID;
 						//FcpLen[2]
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].fcpLen&0x0000FF00)>>8;
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].fcpLen&0x000000FF);
 						//fcp[]
 						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileNum].fcp, simData_tmp->simfile[fileNum].fcpLen);
 						filePackage_loc +=  simData_tmp->simfile[fileNum].fcpLen;
 
 						//DATA
 						sendbuffer[filePackage_loc++] = (unsigned char)FILEDATAID;
 						//dataLen[2]
 						if((simData_tmp->simfile[fileNum].dataLen & 0x00FF0000) > 0)
 						{
 							printf("\n error There is a file datalen bigger than 65535!!!!\n");
 						}
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].dataLen&0x0000FF00)>>8;
 						sendbuffer[filePackage_loc++] = (simData_tmp->simfile[fileNum].dataLen&0x000000FF);
 						//data[]
 						memcpy(&sendbuffer[filePackage_loc], simData_tmp->simfile[fileNum].data, simData_tmp->simfile[fileNum].dataLen);
 						filePackage_loc +=  simData_tmp->simfile[fileNum].dataLen;	
 						printf("\n @file Ending, filePackage_loc is %d", filePackage_loc);
 						
 					}
 					else
 					{
 						printf("\n File 0x%02x %02x  does not exist. ", simData_tmp->simfile[fileNum].fileId[0], simData_tmp->simfile[fileNum].fileId[1]);
 					}
 				
 					
 
 					
 				}
 				/*File contruct end*/
 				sendbuffer[filePackage_loc++] = (char )0xB2; /*Check bytes, filePackage_loc is strucure length*/
 				//Length[4]
 				#if 0
 				sendbuffer[1] = ((filePackage_loc - 5)&0xFF000000)>>24;
 				sendbuffer[2] = ((filePackage_loc - 5)&0x00FF0000)>>16;
 				sendbuffer[3] = ((filePackage_loc - 5)&0x0000FF00)>>8;
 				sendbuffer[4] = ((filePackage_loc - 5)&0x000000FF);
 				#endif 
 				teLoadAllFilesRsp.length[0] = ((filePackage_loc - 5)&0xFF000000)>>24;
 				teLoadAllFilesRsp.length[1] = ((filePackage_loc - 5)&0x00FF0000)>>16;
 				teLoadAllFilesRsp.length[2] = ((filePackage_loc - 5)&0x0000FF00)>>8;
 				teLoadAllFilesRsp.length[3] = ((filePackage_loc - 5)&0x000000FF);
 				
 				allDataLen = filePackage_loc - sizeof(TeLoadAllFilesRsp) -1;  //without the checking bytes
 
 				teLoadAllFilesRsp.allDataLen[0] = (allDataLen&0xFF000000)>>24;
 				teLoadAllFilesRsp.allDataLen[1] = (allDataLen&0x00FF0000)>>16;
 				teLoadAllFilesRsp.allDataLen[2] = (allDataLen&0x0000FF00)>>8;
 				teLoadAllFilesRsp.allDataLen[3] = (allDataLen&0x000000FF);
 				
 				memcpy(sendbuffer, &teLoadAllFilesRsp, sizeof(TeLoadAllFilesRsp));
 				printf("\n LOADALLFILESREQ RSP is :");
 				for(unsigned int i = 0; i< filePackage_loc; i++)
 				{
 					printf(" %02X", (unsigned char)sendbuffer[i]);
 				}

				/*Save to local files*/
				#if 0
				if((simData_tmp->allFileData[0] == 0x00)&&
					(simData_tmp->allFileData[1] == 0x00)&&
					(simData_tmp->allFileData[2] == 0x00)&&
					(simData_tmp->allFileData[3] == 0x00)&&
					(simData_tmp->allFileData[4] == 0x00))
				{
					memcpy(simData_tmp->allFileData, sendbuffer, filePackage_loc);
					char fileName[10];
					sprintf(fileName, "%d.txt", simData_tmp->simId);
					writeFile(fileName,sendbuffer,filePackage_loc);
				}
				#endif
 				printf("\n LOADALLFILESREQ, send LOADALLFILESREQ RSP back to Terminal");
 
 				unsigned int allByteSend = 0;
				
 				while(allByteSend < filePackage_loc)
 				{
 					logCurrentTime();
 					byteSends = send(sockConn,&sendbuffer[allByteSend],min(SENDRECVBUFSIZE, filePackage_loc-allByteSend),0);
 					//byteSends = send(sockConn,sendbuffer,filePackage_loc,0);
 					if(byteSends > 0)
 					{
 						printf("\n %d Bytes send OK", byteSends);
 						allByteSend += byteSends;
 						printf("\n All %d Bytes send OK", allByteSend);
 						result = 0x01;
 					}
 					else
 					{
 						printf("\n APDUREQ not all the data send out");
 						break;
 						result = 0x00;
 					}
 				}
 				
 			}
 			break;
 			
 		}
 		case UPDATESIMSTATUSREQ:
		{
			//isProcessUeReq = true; 
			ServerUpdateUsimStatusReq *serverUpdateUsimStatusReq_p = NULL;
			UsimStatusStr usimStatusStr;
			unsigned char i,usimNum;
			printf("\n UPDATESIMSTATUSREQ:");
			serverUpdateUsimStatusReq_p = (ServerUpdateUsimStatusReq *)recvbuf;
			usimNum = serverUpdateUsimStatusReq_p->simNumAll[3];
			printf("\n UPDATESIMSTATUSREQ: usimNum=%d",usimNum);
			for(i = 1; i<= usimNum; i++)
			{
				memset(&usimStatusStr, 0x00, sizeof(UsimStatusStr));
				//usimStatusStr_p = (UsimStatusStr)*(&recvbuf[sizeof(ServerUpdateUsimStatusReq)+(i-1)*sizeof(UsimStatusStr)]);
				memcpy(&usimStatusStr, (&recvbuf[sizeof(ServerUpdateUsimStatusReq)+(i-1)*sizeof(UsimStatusStr)]), sizeof(UsimStatusStr));
				simData_tmp->usimStatus = (UsimStatus)usimStatusStr.usimStatus;
				printf("\n UPDATESIMSTATUSREQ:, simData_tmp->usimStatus=%d",simData_tmp->usimStatus);
				setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
			}
			
			break;
		}
		case UPDATESINGLESIMSTATUSREQ:
		{
			//isProcessUeReq = true; 
			ServerUpdateSingleUsimStatusReq *serverUpdateSingleUsimStatusReq = NULL;
			unsigned char usimNum;
			printf("\n UPDATESINGLESIMSTATUSREQ:");
			serverUpdateSingleUsimStatusReq = (ServerUpdateSingleUsimStatusReq *)recvbuf;
			usimNum = serverUpdateSingleUsimStatusReq->simNumAll[3];
			printf("\n UPDATESINGLESIMSTATUSREQ: usimNum=%d",usimNum);
			
			simData_tmp->usimStatus = (UsimStatus)serverUpdateSingleUsimStatusReq->usimStatus;
			printf("\n UPDATESIMSTATUSREQ:, simData_tmp->usimStatus=%d",simData_tmp->usimStatus);
			setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
			break;
		}
		case READIMSIICCIDREQ:
		{
			//isProcessUeReq = true; 
			ServerReadImsiIccidReq *serverReadImsiIccidReq_p = NULL;
			unsigned char usimNum;
			printf("\n READIMSIICCIDREQ:");
			serverReadImsiIccidReq_p = (ServerReadImsiIccidReq *)recvbuf;
			usimNum = serverReadImsiIccidReq_p->simId[3];
			printf("\n READIMSIICCIDREQ: simId =%d",usimNum);

			SeverReadImsiIccidRsp severReadImsiIccidRsp;
			memset(&severReadImsiIccidRsp, 0x00, sizeof(ServerReadImsiIccidReq));
			severReadImsiIccidRsp.head = (unsigned char)0xBB;
			severReadImsiIccidRsp.length[0] = (sizeof(ServerReadImsiIccidReq)&0xFF000000)>>24;
			severReadImsiIccidRsp.length[1] = (sizeof(ServerReadImsiIccidReq)&0x00FF0000)>>16;
			severReadImsiIccidRsp.length[2] = (sizeof(ServerReadImsiIccidReq)&0x0000FF00)>>8;
			severReadImsiIccidRsp.length[3] = (sizeof(ServerReadImsiIccidReq)&0x000000FF);
			severReadImsiIccidRsp.reqtype = READIMSIICCIDRSP;

			
			severReadImsiIccidRsp.simIdTag = SIMIDITAG;
			memcpy(severReadImsiIccidRsp.simId, serverReadImsiIccidReq_p->simId,0x04);
			bool imsiFound, iccidFound;
			/* head  SIMID ICCID_ID  ICCID[10] IMSI_ID IMSI[9] USIMSTATUS */
			
			severReadImsiIccidRsp.imsiTag = IMSIID;
			severReadImsiIccidRsp.iccidTag= ICCIDID;
			imsiFound = false;
			iccidFound = false;
			unsigned int bytesSend =0;
			for(unsigned char simLoc = 0; simLoc < SIMFILENUM; simLoc++)
			{
				
				if((iccidFound == false)&&
					(simData_tmp->simfile[simLoc].fileId[0] == 0x2F )&&
					(simData_tmp->simfile[simLoc].fileId[1] == 0xE2 ))
				{
					printf("ICCID Found, copy it\n");
					memcpy(severReadImsiIccidRsp.iccid, simData_tmp->simfile[simLoc].data, min(10,simData_tmp->simfile[simLoc].dataLen));
					iccidFound = true;
				}
				if((imsiFound == false)&&
					(simData_tmp->simfile[simLoc].fileId[0] == 0x6F )&&
					(simData_tmp->simfile[simLoc].fileId[1] == 0x07 ))
				{
					printf("IMSI Found, copy it\n");
					memcpy(severReadImsiIccidRsp.imsi, simData_tmp->simfile[simLoc].data, min(9,simData_tmp->simfile[simLoc].dataLen));
					imsiFound = true;
				}

				if((imsiFound == true)
					&&(iccidFound == true))
				{
					break;
				}
					
			}
			if(iccidFound == false)
			{
				printf("ICCID not Found\n");
			}
			if(imsiFound == false)
			{
				printf("IMSI not Found\n");
			}
			/*copy the value to the sendbuffer*/
			
			memcpy(sendbuffer, &severReadImsiIccidRsp, sizeof(SeverReadImsiIccidRsp));

			printf("\n READIMSIICCIDRSP is :");
			for(unsigned int i = 0; i< sizeof(SeverReadImsiIccidRsp); i++)
			{
				printf(" %02X", (unsigned char)sendbuffer[i]);
			}

			printf("%d bytes send to the Terminal:", sizeof(TeIdRsp));
			logCurrentTime();
			bytesSend = send(sockConn,sendbuffer,sizeof(SeverReadImsiIccidRsp),0);
			if(bytesSend == sizeof(SeverReadImsiIccidRsp))
			{
				printf("\n READIMSIICCIDRSP send OK");
				result = 0x01;
			}
			break;
		}
		
 		default: 
 			printf("\n unknow REQ");
 			break;
 		}
		simData_tmp->isProcessNetcmd = false;
	}	
	
	printf("\r\n%s->%s %d: End.", __FILE__, __FUNCTION__, __LINE__);
	
	return 1;
}

#if 0
bool decodeMsgHeadandLen(RecvDataCache  *recvDataCache, unsigned int *msglen)
{
	bool ret = false;
	int i = 0;

	if(recvDataCache->length == 0)
	{
		printf("\n recvDataCache->length == 0!");
		return ret;
	}

	printf("\n Check Data Head: 0x%02x, 0x%02x, 0x%02x", (unsigned char)recvDataCache->data[0], (unsigned char)recvDataCache->data[1], (unsigned char)recvDataCache->data[2]);
	
	while(i <= recvDataCache->length - 3)
	{
	    
		if(((unsigned char)recvDataCache->data[i] == 0xBB)
			&& ((unsigned char)recvDataCache->data[i+1] == 0)
			&& ((unsigned char)recvDataCache->data[i+2] == 0))
		{
			*msglen = (unsigned char)recvDataCache->data[i+1]<<24|
					  (unsigned char)recvDataCache->data[i+2]<<16| 
					  (unsigned char)recvDataCache->data[i+3]<<8|
					  (unsigned char)recvDataCache->data[i+4];
			ret = true;
			break;
		}
		else
		{
			i++;
		}
	}

	
	if(i > 0)
	{	
		printf("\n %d bytes data unexpected in the head of cacheData, removed it!", i);
		recvDataCache->length -= i;
		recvDataCache->lastIndex = recvDataCache->length;
		memcpy(&recvDataCache->data[0], &recvDataCache->data[i], recvDataCache->length);
		printf("\n remained recvDataCache->length:%d", recvDataCache->length);		
	}	

	return ret;

	/*
	switch((unsigned char)recvDataCache->data[0])
	{
		case 0xAA:
		case 0xBB:
			{				
				*msglen = (unsigned char)recvDataCache->data[1]<<24|
						  (unsigned char)recvDataCache->data[2]<<16|
						  (unsigned char)recvDataCache->data[3]<<8|
						  (unsigned char)recvDataCache->data[4];
				ret = true;
						  
			}
			break;
		default:
			printf("\n Receive Middle Data:");			
			break;
	}

	return ret;
	*/
}
#endif

#if 0
void processSingleMsg(char *recvBuffer)
{
	
	unsigned int data_pro_loc = 0;
	FILE * sfp;
	char simBankId[3]={0};
		
	/*Process the Reqfrom Teriminal*/							
	printf("\n Process the Reqfrom Teriminal:\n");
	if((unsigned char)recvBuffer[data_pro_loc] == 0xBB) /*card box head*/
	{
		printf("Reqfrom Teriminal with personal protocol:\n");
		/*De-compose the head and tail, then call processAPDUCmd*/
		//processAPDUCmd();
		/*send the RSP to terminal */
		if((unsigned char)recvBuffer[data_pro_loc + 5] == 0x6F)
		{
			/*Receive the SIMREGRSP*/			 
			CardBoxRegRsp *cardBoxRegRsp_p = NULL;
			cardBoxRegRsp_p = (CardBoxRegRsp *)(recvBuffer+data_pro_loc);
			thisSimBankId = (unsigned char)cardBoxRegRsp_p->simBankId[0]<<16|
							(unsigned char)cardBoxRegRsp_p->simBankId[1]<<8|
							cardBoxRegRsp_p->simBankId[2];
			printf("\nthisSimBankId = %d", thisSimBankId);

			if((sfp=fopen("simBankId","w"))==NULL)
			{
				 printf("\nsimBankId file open failed");
			}
			else
			{
				for(unsigned char i = 0; i<3; i++)
				 {
				 	
					printf(" %d", simBankId[i]);
					fputc(cardBoxRegRsp_p->simBankId[i],sfp);
				 }
			}
			fclose(sfp);
			
		}
		else
		{
			isProcessUeReq = true;
			processTerminalReq(recvBuffer+data_pro_loc, sockClient);
			isProcessUeReq = false;
		}
		
	}
	else
	{
		printf("Reqfrom Teriminal without personal protocol:\n");
		printf("Just forward the SIM card and return the card RESP:\n");

	}		
}
#endif

#if 0
bool checkRecvDataIntegrity(char *recvdata, int dataLen)
{
    bool ret = false;
	unsigned int msglen = 0;
	unsigned int MsgTotalLen = 0;
	unsigned int dataloc = 0;
	unsigned int remainLen = 0;
	unsigned char TmpBuff[MAXSENDNUM*2];

	recvDataCache.length += dataLen;	
	memcpy((recvDataCache.data + recvDataCache.lastIndex), recvdata, dataLen);
	recvDataCache.lastIndex = recvDataCache.length;
	printf("\n recvDataCache.length:%d, recvDataCache.lastIndex:%d\n",recvDataCache.length, recvDataCache.lastIndex);
	while(decodeMsgHeadandLen(&recvDataCache, &msglen))
	{
		MsgTotalLen = msglen + 5;
		if(recvDataCache.length < MsgTotalLen)
		{
			printf("\n More data shall be received\n");
			ret = true;
			break;
		}		
		else
		{
			printf("\n one or More messages have been received\n");
			
			processSingleMsg(&recvDataCache.data[0]);
			memset(&recvDataCache.data[0], 0, MsgTotalLen);
			recvDataCache.length -= MsgTotalLen;
			recvDataCache.lastIndex = recvDataCache.length;	
			printf("\n recvDataCache.length:%d, recvDataCache.lastIndex:%d\n",recvDataCache.length, recvDataCache.lastIndex);
			if(recvDataCache.length != 0)
			{
				memcpy(&TmpBuff[0], &recvDataCache.data[MsgTotalLen], recvDataCache.length);
				memcpy(&recvDataCache.data[0], &TmpBuff[0], recvDataCache.length);
				ret = true;
			}
			else
			{
				ret = false;
				break;
			}
			
		}
	}
	return ret;
}
#endif

void protocolPacketHandle(SOCKET s, uchar *recv_buf, ushort sRecvLen)
{
    uchar *cpRecvBuf = recv_buf;
    ushort i = 0;
    ushort sHandleFlag = 0;
    //static ushort sValidFlag = 0;
    //static ushort sValidDataLen = 0;
    ushort sValidDataCrc = 0;
    //static uchar cRecvValidData[2048] = {0};

    ushort sValidFlag = *pValidFlag;
    ushort sValidDataLen = *pValidDataLen;
    
    printf("\r\n%s->%s %d: Begin.sRecvLen = %d.", __FILE__, __FUNCTION__, __LINE__, sRecvLen);
    
    if ((0 == sRecvLen) || (sRecvLen > MAXRECVNUM))
    {
        printf("\r\n%s->%s %d: Para Error.", __FILE__, __FUNCTION__, __LINE__);
        
        return;
    }
    
    for (; i < sRecvLen; i++)
    {
        switch (sValidFlag)
        {
            case 0:  //0xBB
                if (HEADER_FLAG1 == cpRecvBuf[i])
                {
                    cRecvValidData[sValidFlag ++] = HEADER_FLAG1;
                    
                    gVlaidStatusFlag = 0;
                }
                else
                {
                    printf("\r\n  %s->%s %d: Error.cpRecvBuf[%d] = 0x%02X.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], gVlaidStatusFlag);
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag);
                
                break;
            case 1:  //0x00
                if (0 != gVlaidStatusFlag)
                {
                    printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                    
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                gVlaidStatusFlag = 1;
                
                if (HEADER_FLAG2 == cpRecvBuf[i])
                {
                    cRecvValidData[sValidFlag ++] = HEADER_FLAG2;
                }
                else
                {
                    printf("\r\n  %s->%s %d: Error.cpRecvBuf[%d] = 0x%02X.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], gVlaidStatusFlag);
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag);
                
                break;
            case 2:  //0x00
                if (1 != gVlaidStatusFlag)
                {
                    printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                    
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                gVlaidStatusFlag = 2;
                
                if (UNUSE_DATA == cpRecvBuf[i])
                    cRecvValidData[sValidFlag ++] = UNUSE_DATA;
                else
                {
                    printf("\r\n  %s->%s %d: Error.cpRecvBuf[%d] = 0x%02X.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], gVlaidStatusFlag);
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag);
                
                break;
            case 3:  //len-h
                if (2 != gVlaidStatusFlag)
                {
                    printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                    
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                gVlaidStatusFlag = 3;
                
                cRecvValidData[sValidFlag ++] = cpRecvBuf[i];
                //sHandleFlag = i;
                
                printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag);
                
                break;
            case 4:  //len-l
                if (3 != gVlaidStatusFlag)
                {
                    printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                    
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                gVlaidStatusFlag = 4;
                
                cRecvValidData[sValidFlag ++] = cpRecvBuf[i];
                sValidDataLen = (ushort)(cRecvValidData[sValidFlag - 2]) << 8 |\
                                (ushort)(cRecvValidData[sValidFlag - 1]) << 0;
                
                if ((sValidDataLen < 2) || (sValidDataLen > MAXRECVNUM))
                {
                    printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.sValidDataLen = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag, sValidDataLen);
                    
                    gVlaidStatusFlag = 7;
                    
                    goto ErrorHandle;
                }
                
                printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.sValidDataLen = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag, sValidDataLen);
                
                break;
            default :
                if (sValidFlag < (sValidDataLen + 5))
                {
                    if (4 != gVlaidStatusFlag)
                    {
                        printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                        
                        gVlaidStatusFlag = 7;
                        
                        goto ErrorHandle;
                    }
                    
                    cRecvValidData[sValidFlag ++] = cpRecvBuf[i];
                    
                    printf("\r\n  %s->%s %d: cpRecvBuf[%d] = 0x%02X.sValidFlag = %d.sValidDataLen = %d.", __FILE__, __FUNCTION__, __LINE__, i, cpRecvBuf[i], sValidFlag, sValidDataLen);
                }
                
                if (sValidFlag >= (sValidDataLen + 5))
                {
                    gVlaidStatusFlag = 5;
                    
                    printf("\r\n%s->%s %d: Handle a packet.ValidLen = %d.-->", __FILE__, __FUNCTION__, __LINE__, sValidFlag);
					for (ushort cnt = 0; cnt < sValidFlag; cnt ++)
					{
						printf(" %02X", cRecvValidData[cnt]);	
					}
					
                    //sValidDataCrc = (ushort)(cRecvValidData[sValidFlag - 2]) << 8 |\
                    //                (ushort)(cRecvValidData[sValidFlag - 1]) << 0;
                    
                    //if (sValidDataCrc == GenCRC16(cpRecvBuf, sValidDataLen + 3))
                    if (1)
                    {
                        //i = sValidFlag - 1;  //del by ml.20160825.
                        if (5 == gVlaidStatusFlag)
                        {
                            processTerminalReq((char *)&cRecvValidData[0], s);
                            
                            gVlaidStatusFlag = 6;
                        }
                        else
                        {
                            printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag);
                            
                            gVlaidStatusFlag = 7;
                            
                            goto ErrorHandle;
                        }
                    }
                    //else
                    //{
                    //    i = sHandleFlag;
                    //    
                    //    printf("\r\n%s->%s %d: crc error.", __FILE__, __FUNCTION__, __LINE__);
                    //}
                    
                    sValidFlag = 0;
                    sValidDataLen = 0;
                }
                
                break;
        }
    }

ErrorHandle:
    if (gVlaidStatusFlag >= 7)
    {
        printf("\r\n  %s->%s %d: Error.gVlaidStatusFlag = %d.sValidFlag = %d.sValidDataLen = %d.", __FILE__, __FUNCTION__, __LINE__, gVlaidStatusFlag, sValidFlag, sValidDataLen);
        
        memset(cRecvValidData, 0, 2048);
        sValidFlag = 0;
        sValidDataLen = 0;
        gVlaidStatusFlag = 7;
    }

    *pValidFlag = sValidFlag;
    *pValidDataLen = sValidDataLen;
    
    printf("\r\n%s->%s %d: End.\n", __FILE__, __FUNCTION__, __LINE__);

    return;
}

int socketClient()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    
    int nRecvBufLen = 32 * 1024;
    int nSendBufLen = 32 * 1024;
    bool sockon = false;
    
    uchar uServerConnectNum = 0;
    uchar uConnectNum = 0;
    ushort uServerConnectTime = 0;
    uchar uConnedtTime = 0;
    uchar index = 0;
    
    uchar isWaitingHeartBeatRsp = *pIsWaitingHeartBeatRsp;
    uchar uHeartBeatNum = 0;
    
    printf("\r\n%s->%s %d: Start.\n", __FILE__, __FUNCTION__, __LINE__);
    
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);  //加载Winsocket DLL
    if (err != 0)
    {
        printf("socketClient WSAStartup error, return here!\n");
        
        return 0;
    }
    
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        WSACleanup();
        printf("socketClient LOBYTE error, return here!\n");
        
        return 0;
    }
    
    SOCKADDR_IN addrSrv;  //socketAddress socket端口
    
    while (1)
    {
        if ((5 == pSimBankIpAddrRecord->netCurrentConnectFlag) ||\
            (6 == pSimBankIpAddrRecord->netCurrentConnectFlag))
        {
            bool alive = false;
            struct tcp_keepalive tcp_alive_in;
            struct tcp_keepalive tcp_alive_out;
            unsigned long ret;
			char ipBuf[16] = { 0 };
            char *pIpBuf = ipBuf;
            
            if (5 == pSimBankIpAddrRecord->netCurrentConnectFlag)  //默认服务器
            {
                sprintf(pIpBuf, "%d.%d.%d.%d", pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[0],
                                               pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[1],
                                               pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[2],
                                               pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[3]);
                
                pServerNetPort = pSimBankIpAddrRecord->defaultIpPortRecord.portAddr;
                uServerConnectNum = pSimBankIpAddrRecord->defaultIpPortRecord.connectNum;
                uServerConnectTime = pSimBankIpAddrRecord->defaultIpPortRecord.overTimes;
                
                printf("socketClient()--->连接默认服务器.pSimBankIpAddrRecord->netCurrentConnectFlag = %d.\n", pSimBankIpAddrRecord->netCurrentConnectFlag);
            }
            else if (6 == pSimBankIpAddrRecord->netCurrentConnectFlag)  //备份服务器
            {
                sprintf(pIpBuf, "%d.%d.%d.%d", pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[0],
                                               pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[1],
                                               pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[2],
                                               pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[3]);
                
                pServerNetPort = pSimBankIpAddrRecord->backupIpPortRecord.portAddr;
                uServerConnectNum = pSimBankIpAddrRecord->backupIpPortRecord.connectNum;
                uServerConnectTime = pSimBankIpAddrRecord->backupIpPortRecord.overTimes;
                
                printf("socketClient()--->连接备份服务器.pSimBankIpAddrRecord->netCurrentConnectFlag = %d.\n", pSimBankIpAddrRecord->netCurrentConnectFlag);
            }
            else
            {
                printf("socketClient()--->当前网络连接标志被更改.pSimBankIpAddrRecord->netCurrentConnectFlag = %d.\n", pSimBankIpAddrRecord->netCurrentConnectFlag);
                
                pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                pSimBankIpAddrRecord->netNextConnectFlag = 0;
                
                continue;
            }
            
            sockClient = socket(AF_INET, SOCK_STREAM, 0);  //创建套接字
            addrSrv.sin_port = htons(pServerNetPort);  //mdf by ml.20160805.
            addrSrv.sin_addr.S_un.S_addr = inet_addr(pIpBuf);  /*Server on HK*/
            addrSrv.sin_family = AF_INET;
            
            setsockopt (sockClient, SOL_SOCKET, SO_RCVBUF, (const char *)&nRecvBufLen, sizeof(int));
            setsockopt (sockClient, SOL_SOCKET, SO_SNDBUF, (const char *)&nSendBufLen, sizeof(int));
            
            alive = true;
            tcp_alive_in.onoff = true;
            tcp_alive_in.keepalivetime = 5 * 1000;
            tcp_alive_in.keepaliveinterval = 3 * 1000;
            
            memset(&tcp_alive_out, 0x00, sizeof(tcp_keepalive));
            
            /* Set: use keepalive on fd */
            if (setsockopt(sockClient, SOL_SOCKET, SO_KEEPALIVE, (const char *) &alive, sizeof(alive)) != 0)
            {
                printf("Call setsockopt keepAlive error, errno is %d/n", errno);
                // return -1;
            }
            
            if (WSAIoctl(sockClient, SIO_KEEPALIVE_VALS, &tcp_alive_in, sizeof(tcp_alive_in), 
        		&tcp_alive_out, sizeof(tcp_alive_out), &ret, NULL, NULL) != 0)
            {
                printf("Call WSAIoctl keepAlive error, errno is %d/n", errno);
            }
            
        	if (0 == connect(sockClient, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR)))
        	{
        		isServerConnected = true;
        		printf("This is card box, connected to server\n");
        		char recvBuffer[MAXRECVNUM] = { 0 };
        		int recvNum = 0;
        		sockon = true;
        	    
        		fd_set fdread;
        		int ret;
        		//struct timeval tv = {300, 0};  /*set timer 5 min*//*2016.03.15, mod by Xili for adding network heart beating*/   
        		struct timeval tv;
        		tv.tv_sec = uServerConnectTime;
        		tv.tv_usec = 0;
        		printf("socketClient()--->timeal tv = %d s.\n", tv.tv_sec);
        		
        		char sendBuff[SENDRECVBUFSIZE];	 //sendBuff	
        		unsigned int bytesend;
        		//printf("\nPlease input the bytes to send:\n");
        		//scanf_s("%s",sendBuff,100);
        	    
        	    thisSimBankId = pSimCardIdNum;  //add by ml.20160809.
        	    
        		/*Send Card box registration to server*/
        		printf("\nSend Card box registration to server:, the size of TeIdReq is: 6\n");
        		CardBoxRegReq cardBoxRegReq;
        		unsigned int usimStrLoc;
        		memset(&cardBoxRegReq, 0x00, sizeof(CardBoxRegReq));
        		memset(sendBuff, 0x00, SENDRECVBUFSIZE);
        		cardBoxRegReq.head = 0xBB;
        		cardBoxRegReq.regtype = 0x6F;
        		cardBoxRegReq.simNumAll[0] = (thisSimBankId&0xFF0000)>>16;
        		cardBoxRegReq.simNumAll[1] = (thisSimBankId&0x00FF00)>>8;
        		cardBoxRegReq.simNumAll[2] = (thisSimBankId&0x0000FF);
        		//cardBoxRegReq.simNumAll[3] = ALLSIMNUM;
        		cardBoxRegReq.simNumAll[3] = pMaxSimNum;  //mdf by ml.20160804.
        		usimStrLoc = sizeof(CardBoxRegReq); //the begin of USIMSTRCTURE
        		SimInfoStr simInfoStr; 
        		bool imsiFound, iccidFound;
        		SimData *simData_tmp = simdataHead;
        		/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
        		unsigned char iccidFileLoc = 0;
        		unsigned char imsiFileLoc = 0;
        		/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
        		/* head  SIMID ICCID_ID  ICCID[10] IMSI_ID IMSI[9] USIMSTATUS */
        		while(simData_tmp != NULL)
        		{
        			memset(&simInfoStr, 0x00, sizeof(SimInfoStr));
        			simInfoStr.head = USIMSTRUCTUREID;
        			simInfoStr.simId[3] = simData_tmp->simId;
        			simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
        			simInfoStr.imsiId = IMSIID;
        			simInfoStr.iccidId = ICCIDID;
        			imsiFound = false;
        			iccidFound = false;
        			
        			for(unsigned char simLoc = 0; simLoc < SIMFILENUM; simLoc++)
        			{
        				if((iccidFound == false)&&
        					(simData_tmp->simfile[simLoc].fileId[0] == 0x2F )&&
        					(simData_tmp->simfile[simLoc].fileId[1] == 0xE2 ))
        				{
        					printf("ICCID Found, copy it\n");
        					printf("ICCID Found, simData_tmp->simfile[simLoc].dataLen=%d\n",simData_tmp->simfile[simLoc].dataLen);
        					memcpy(simInfoStr.iccid, simData_tmp->simfile[simLoc].data, min(10,simData_tmp->simfile[simLoc].dataLen));
        					iccidFound = true;
        					iccidFileLoc = simLoc;/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID*/
        				}
        				if((imsiFound == false)&&
        					(simData_tmp->simfile[simLoc].fileId[0] == 0x6F )&&
        					(simData_tmp->simfile[simLoc].fileId[1] == 0x07 ))
        				{
        					printf("IMSI Found, copy it\n");
        					printf("IMSI Found, simData_tmp->simfile[simLoc].dataLen=%d\n",simData_tmp->simfile[simLoc].dataLen);
        					memcpy(simInfoStr.imsi, simData_tmp->simfile[simLoc].data, min(9,simData_tmp->simfile[simLoc].dataLen));
        					imsiFound = true;
        					imsiFileLoc = simLoc;/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID*/
        				}
        
        				if((imsiFound == true)
        					&&(iccidFound == true))
        				{
        					break;
        				}
        					
        			}
        			if(iccidFound == false)
        			{
        				printf("ICCID not Found\n");
        				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
        				if(simData_tmp->usimStatus == USIM_ON)
        				{
        					simData_tmp->usimStatus = USIM_OFF;
        					simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
        					printf("Set usimstatus as USIM_OFF for no iccid found\n");
        					setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
        				}
        				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
        			}
        			if(imsiFound == false)
        			{
        				printf("IMSI not Found\n");
        				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
        				if(simData_tmp->usimStatus == USIM_ON)
        				{
        					simData_tmp->usimStatus = USIM_OFF;
        					simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
        					printf("Set usimstatus as USIM_OFF for no IMSI found\n");
        					setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
        				}
        				/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
        			}
        			/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,begin*/
        			if(((iccidFileLoc != false) && (imsiFileLoc != false)) 
        				&& (( simData_tmp->simfile[iccidFileLoc].dataLen == 0) 
        				|| (simData_tmp->simfile[imsiFileLoc].dataLen == 0))
        				&& (simData_tmp->usimStatus == USIM_ON))
        			{
        				simData_tmp->usimStatus = USIM_OFF;
        				simInfoStr.usimStatus = (unsigned char)simData_tmp->usimStatus;
        				printf("Set usimstatus as USIM_OFF for iccid/imsi data length as 0\n");
        				setCurrentUsimStatus(simData_tmp->hCom, simData_tmp->simId, simData_tmp->usimStatus);
        			}
        			/*2016.04.08, mod by Xili for report error SIM state(2) without IMSI/ICCID,end*/
        			/*copy the value to the sendbuffer*/
        			memcpy(&sendBuff[usimStrLoc], &simInfoStr, sizeof(SimInfoStr));
        
        			usimStrLoc += sizeof(SimInfoStr);
        			simData_tmp = simData_tmp->next_SIM;
        		}
        		
        		sendBuff[usimStrLoc++] = (unsigned char)0xB2;  /*checking byte, usimStrLoc is the length of sendbuffer*/
                                
        		cardBoxRegReq.length[0] = ((usimStrLoc-5)&0xFF000000)>>24;
        		cardBoxRegReq.length[1] = ((usimStrLoc-5)&0x00FF0000)>>16;
        		cardBoxRegReq.length[2] = ((usimStrLoc-5)&0x0000FF00)>>8;
        		cardBoxRegReq.length[3] = ((usimStrLoc-5)&0x000000FF);
                
        		memcpy(sendBuff, &cardBoxRegReq, sizeof(CardBoxRegReq));
        		printf("\n Card box registration is :");
        		for(unsigned int i = 0; i< usimStrLoc; i++)
        		{
        			printf(" %02X", (unsigned char)sendBuff[i]);
        		}
        		logCurrentTime();
        		bytesend = send(sockClient, sendBuff, usimStrLoc, 0);  //向服务器发送数据
        		printf("\n%d Bytes send\n", bytesend);
        		//Sleep(1000);
        		
        		/*Receive the SIMREGRSP*/
        		//recvNum = recv(sockClient,recvBuffer,MAXSENDNUM,0);
        		
        		uHeartBeatNum = 0;
        		isWaitingHeartBeatRsp = 0;
        	    *pIsWaitingHeartBeatRsp = isWaitingHeartBeatRsp;
        		
        		while (sockon)
        		{
        			printf("\n Card box Wait data from server:\n");
        			FD_ZERO(&fdread); 
        			FD_SET(sockClient, &fdread);
        			isProcessUeReq = false;
        			
        			ret = select(0, &fdread, NULL, NULL, &tv);
        			if (0 == ret)
        			{
            			isWaitingHeartBeatRsp = *pIsWaitingHeartBeatRsp;
                        
                        printf("Socket Select time out,isWaitingHeartBeatRsp = %d\n", isWaitingHeartBeatRsp);
                        if (0 == isWaitingHeartBeatRsp)
                        {
                            printf("\nSend Card box Heart beat packet to server: the size of TeIdReq is: 6\n");
                            
                            SimBankSendHeartBeatReq simBankSendHeartBeatReq;
                            unsigned int byteSends;
                            memset(&simBankSendHeartBeatReq, 0x00, sizeof(SimBankSendHeartBeatReq));
                            
                            simBankSendHeartBeatReq.head = 0xBB;
                            simBankSendHeartBeatReq.length[3] = 0x02;
                            simBankSendHeartBeatReq.reqtype = NETHEARTBEATIND;
                            
                            memset(sendBuff, 0x00, SENDRECVBUFSIZE);
                            memcpy(sendBuff, &simBankSendHeartBeatReq, sizeof(SimBankSendHeartBeatReq));
                            
                            printf("\n SimBankSendHeartBeatInd is :");
                            for (unsigned int i = 0; i< sizeof(SimBankSendHeartBeatReq); i++)
                            {
                                printf(" %02X", (unsigned char)sendBuff[i]);
                            }
                            
                            printf("\n%d bytes send to the Terminal\n", sizeof(SimBankSendHeartBeatReq));
                            logCurrentTime();
                            byteSends = send(sockClient,sendBuff,sizeof(SimBankSendHeartBeatReq),0);
                            if (byteSends == sizeof(SimBankSendHeartBeatReq))
                            {
                                printf("\n SimBankSendHeartBeatInd send OK");
                                isWaitingHeartBeatRsp = 1;
                            }
                        }
                        else if (1 == isWaitingHeartBeatRsp)
                        {
                            printf("\nsocketClient()--->Didn't receive the heart beat response from server:\n");
                            isWaitingHeartBeatRsp = 0;
                            
                            uHeartBeatNum ++;
                            if (uHeartBeatNum >= HEART_BEAT_TIME)
                            {
                                uHeartBeatNum = 0;
                                
                                sockon = false;
                                closesocket(sockClient);
                                
                                printf("   Need Ro Re-connect The Server!!\n");
                                printf("   socketClient()--->uHeartBeatNum = %d.HEART_BEAT_TIME = %d.\n", uHeartBeatNum, HEART_BEAT_TIME);
                                
                                EexceptionalCasesRecord(NETWORK_HEART_BEAT_NO_RECV);
                            }
                            else
                            {
                                printf("   Repeat Send The HeartBeat Data.\n");
                                printf("   socketClient()--->uHeartBeatNum = %d.HEART_BEAT_TIME = %d.\n", uHeartBeatNum, HEART_BEAT_TIME);
                            }
                            
                            uConnectNum ++;
                            if (uConnectNum > uServerConnectNum)
                            {
                                uConnectNum = 0;
                                
                                printf("socketClient()--->连续多次未收到服务器回发的心跳包报文.\n");
                                
                                if (5 == pSimBankIpAddrRecord->netCurrentConnectFlag)
                                {
                                    if (6 == pSimBankIpAddrRecord->netNextConnectFlag)
                                    {
                                        pSimBankIpAddrRecord->netCurrentConnectFlag = 6;
                                        printf("socketClient()--->切换到备份服务器进行连接.\n");
                                    }
                                    else  //0
                                    {
                                        pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                        printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                    }
                                }
                                else
                                {
                                    pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                    printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                }
                                
                                pSimBankIpAddrRecord->netNextConnectFlag = 0;
                            }
                        }
                        else
                        {
                            printf("socket_client()--->isWaitingHeartBeatRsp = %d.\n", isWaitingHeartBeatRsp);
                            isWaitingHeartBeatRsp = 0;
                        }
                        
                        *pIsWaitingHeartBeatRsp = isWaitingHeartBeatRsp;
        			}
        			else
        			{
        				if (FD_ISSET(sockClient, &fdread)) 
        				{
        					bool recvMore = true;
        					unsigned int recv_loc = 0;
        					unsigned int actuallLen = 0;
        					bool dataRecv = false;
        					logCurrentTime();
        					//while(recvMore == true)
        					{
        						recvNum = recv(sockClient,recvBuffer,MAXSENDNUM,0);
        						if(recvNum < 0)
        						{
        							printf("\n recvNum < 0, error:%d happened \n", WSAGetLastError());
        							sockon = false;
        							closesocket(sockClient);
                                    
        							simData_tmp = simdataHead;
        							while(simData_tmp!=NULL)
        							{									
        								simData_tmp->isProcessNetcmd = false;
        								
        								simData_tmp = simData_tmp->next_SIM;
        							}
        							
        							isServerConnected = false;
        							
        							uConnectNum ++;
                                    if (uConnectNum > uServerConnectNum)
                                    {
                                        uConnectNum = 0;
                                        
                                        printf("socketClient()--->连续多次接收报文长度小于0.\n");
                                        
                                        if (5 == pSimBankIpAddrRecord->netCurrentConnectFlag)
                                        {
                                            if (6 == pSimBankIpAddrRecord->netNextConnectFlag)
                                            {
                                                pSimBankIpAddrRecord->netCurrentConnectFlag = 6;
                                                printf("socketClient()--->切换到备份服务器进行连接.\n");
                                            }
                                            else  //0
                                            {
                                                pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                                printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                            }
                                        }
                                        else
                                        {
                                            pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                            printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                        }
                                        
                                        pSimBankIpAddrRecord->netNextConnectFlag = 0;
                                    }
        							
        							EexceptionalCasesRecord(NETWORK_RECV_DATA_LESS_0);
        							
        							break;
        						}
        						else if(recvNum == 0)
        						{
        							printf("\n recvNum = 0 , peer socket closed ");
        							sockon = false;
                                    closesocket(sockClient);
                                    
        							isServerConnected = false;
        							
        							uConnectNum ++;
                                    if (uConnectNum > uServerConnectNum)  //连续多次接收报文长度等于0
                                    {
                                        uConnectNum = 0;
                                        
                                        printf("socketClient()--->连续多次接收报文长度等于0.\n");
                                        
                                        if (5 == pSimBankIpAddrRecord->netCurrentConnectFlag)
                                        {
                                            if (6 == pSimBankIpAddrRecord->netNextConnectFlag)
                                            {
                                                pSimBankIpAddrRecord->netCurrentConnectFlag = 6;
                                                printf("socketClient()--->切换到备份服务器进行连接.\n");
                                            }
                                            else  //0
                                            {
                                                pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                                printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                            }
                                        }
                                        else
                                        {
                                            pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                                            printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                                        }
                                        
                                        pSimBankIpAddrRecord->netNextConnectFlag = 0;
                                    }
        							
        							EexceptionalCasesRecord(NETWORK_RECV_DATA_EQUAL_0);
        							
        							break;
        						}
        						else
        						{
        							printf("\n Receive %d Bytes: \n", recvNum);
        							dataRecv = true;
        							for(int i = 0; i< recvNum; i++)
        							{
        								printf(" %02X",(unsigned char)recvBuffer[i+recv_loc]);	
        							}
        							
        							protocolPacketHandle(sockClient, (uchar *)&recvBuffer[0], recvNum);
        							
        							uConnectNum = 0;
        						}
        					}
        				}
        			}
        		}
        		
        	    EexceptionalCasesRecord(NETWORK_LINK_RECONNECTION);
                
                /*time value range from 1s to 20s*/
        		unsigned int timer_value = (double)rand() / (RAND_MAX + 1) * (20000 - 1000) + 1000;
        		
        		printf("Connect to the Server wait timer_value =  %6d ms.\n", timer_value);
        		Sleep(timer_value);  //Sleep with ms, sleep with s
        	}
        	else
        	{
        		/* 2016.03.29, mod by Xili for adding sleep timer with rand value, begin*/
        	    /*Refered from MSDN RAND function
        		// Generate random numbers in the half-closed interval
                // [range_min, range_max). In other words,
                // range_min <= random number < range_max   
                int u = (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
                */
                /*time value range from 5s to 60s*/
        		unsigned int timer_value = (double)rand() / (RAND_MAX + 1) * (60000 - 5000) + 5000;
                printf( "timer_value =  %6d\n", timer_value);
        		
        		printf("Fail to connect to the Server:\n");
        		Sleep(timer_value);//Sleep with ms, sleep with s
        		/* 2016.03.29, mod by Xili for adding sleep timer with rand value, end*/
        		
        		uConnectNum ++;
                if (uConnectNum > uServerConnectNum)
                {
                    uConnectNum = 0;
                    
                    printf("socketClient()--->连续多次connect服务器不成功.\n");
                    
                    if (5 == pSimBankIpAddrRecord->netCurrentConnectFlag)
                    {
                        if (6 == pSimBankIpAddrRecord->netNextConnectFlag)
                        {
                            pSimBankIpAddrRecord->netCurrentConnectFlag = 6;
                            printf("socketClient()--->切换到备份服务器进行连接.\n");
                        }
                        else  //0
                        {
                            pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                            printf("socketClient()--->切换到负载均衡服务器0进行连接.\n");
                        }
                    }
                    else
                    {
                        pSimBankIpAddrRecord->netCurrentConnectFlag = 0;
                        printf("socketClient()--->切换到负载均衡服务器进行连接.\n");
                    }
                    
                    pSimBankIpAddrRecord->netNextConnectFlag = 0;
                }
        	}
        }
        else
        {
            Sleep(300);
        }
    }
    
    closesocket(sockClient);
    WSACleanup();
    
    printf("\r\n%s->%s %d: End.Error!!!\n", __FILE__, __FUNCTION__, __LINE__);
    
    return 0;
    
}
void logCurrentTime(void)
{
	SYSTEMTIME sys_cur;
	GetLocalTime( &sys_cur);
	printf( "\nCurrentTime is: %d/%d/%d %d:%d:%d.%d \n"
 	 ,sys_cur.wYear,sys_cur.wMonth,sys_cur.wDay
 	 ,sys_cur.wHour,sys_cur.wMinute,sys_cur.wSecond,sys_cur.wMilliseconds);
}


void SimBankSendOperIpReqPacket(void)
{
    uchar i = 0;
    
    
    printf("SimBankSendOperIpReqPacket() 获取业务IP地址申请：\n");
    
    ptrSimBankGainOperationIpReq->head[0] = 0xAA;
    ptrSimBankGainOperationIpReq->head[1] = 0x00;
    ptrSimBankGainOperationIpReq->reserve = 0x00;
    ptrSimBankGainOperationIpReq->length[0] = 0x00;
    ptrSimBankGainOperationIpReq->length[1] = 0x06;
    ptrSimBankGainOperationIpReq->reqtype = 0x20;
    ptrSimBankGainOperationIpReq->simbankidTag = 0xDF;
    ptrSimBankGainOperationIpReq->simbankid[0] = (pSimCardIdNum & 0xFF0000) >> 16;
    ptrSimBankGainOperationIpReq->simbankid[1] = (pSimCardIdNum & 0x00FF00) >> 8;
    ptrSimBankGainOperationIpReq->simbankid[2] = (pSimCardIdNum & 0x0000FF);
    ptrSimBankGainOperationIpReq->checkingBytes = 0xB2;  //TODO
    
    
}

char SimBankOperIpRspPacketHandle(const uchar *pRecvBuf, const uchar uLen)
{
    uchar i = 0;
    SimBankGainOperationIpRsp *pSimBankGainOperationIpRsp = NULL;
    
    if (NULL == pRecvBuf)
    {
        printf("SimBankOperIpRspPacketHandle() pRecvBuf is NULL.\n");
        
        return -1;
    }
    
    printf("SimBankOperIpRspPacketHandle() 收到的报文为: \n");
    for (i = 0; i < uLen; i++)
    {
        printf("%02x ", pRecvBuf[i]);
    }
    printf("\n");
    
    pSimBankGainOperationIpRsp = (SimBankGainOperationIpRsp *)pRecvBuf;
    
    if ((pSimBankGainOperationIpRsp->length[1] != 0x10) ||\
        (pSimBankGainOperationIpRsp->reqtype   != 0x20) ||\
        (pSimBankGainOperationIpRsp->ipTag     != 0xE0) ||\
        (pSimBankGainOperationIpRsp->bipTag    != 0xE0))
    {
        printf("SimBankOperIpRspPacketHandle() pRecvBuf Error.\n");
        
        return -1;
    }
    
    pSimBankIpAddrRecord->defaultIpPortRecord.fieldValidFlag = 1;
    memcpy(pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr, pSimBankGainOperationIpRsp->dIp, 4);
    pSimBankIpAddrRecord->defaultIpPortRecord.portAddr = pSimBankGainOperationIpRsp->dPort[0] << 8 |\
                                                         pSimBankGainOperationIpRsp->dPort[1];
    //pSimBankIpAddrRecord->defaultIpPortRecord.connectNum = 4;
    //pSimBankIpAddrRecord->defaultIpPortRecord.overTimes = 240;  //这两个参数从配置文件中获取，不做更改
    pSimBankIpAddrRecord->serverIpValidFlag |= 1 << 5;
    
    pSimBankIpAddrRecord->backupIpPortRecord.fieldValidFlag = 1;
    memcpy(pSimBankIpAddrRecord->backupIpPortRecord.ipAddr, pSimBankGainOperationIpRsp->bIp, 4);
    pSimBankIpAddrRecord->backupIpPortRecord.portAddr = pSimBankGainOperationIpRsp->bPort[0] << 8 |\
                                                        pSimBankGainOperationIpRsp->bPort[1];
    pSimBankIpAddrRecord->backupIpPortRecord.connectNum = pSimBankIpAddrRecord->defaultIpPortRecord.connectNum;
    pSimBankIpAddrRecord->backupIpPortRecord.overTimes = pSimBankIpAddrRecord->defaultIpPortRecord.overTimes;
    pSimBankIpAddrRecord->serverIpValidFlag |= 1 << 6;
    
    printf("负载均衡服务器下发的配置：\n");
    printf("  默认服务器：\n");
    printf("    IP地址       = %d.%d.%d.%d.\n", pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[0],
                                                pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[1],
                                                pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[2],
                                                pSimBankIpAddrRecord->defaultIpPortRecord.ipAddr[3]);
    printf("    端口号       = %d.\n", pSimBankIpAddrRecord->defaultIpPortRecord.portAddr);
    printf("    重连次数     = %d.\n", pSimBankIpAddrRecord->defaultIpPortRecord.connectNum);
    printf("    重连等待时间 = %d.\n", pSimBankIpAddrRecord->defaultIpPortRecord.overTimes);
    printf("  备份服务器：\n");
    printf("    IP地址       = %d.%d.%d.%d.\n", pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[0],
                                                pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[1],
                                                pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[2],
                                                pSimBankIpAddrRecord->backupIpPortRecord.ipAddr[3]);
    printf("    端口号       = %d.\n", pSimBankIpAddrRecord->backupIpPortRecord.portAddr);
    printf("    重连次数     = %d.\n", pSimBankIpAddrRecord->backupIpPortRecord.connectNum);
    printf("    重连等待时间 = %d.\n", pSimBankIpAddrRecord->backupIpPortRecord.overTimes);
    printf("\n");
    
    return 0;
}


int UdpSocketThread()
{
    WORD socketVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    
    char recvData[64] = { 0 };
    char ret = 0;
    char strBuf[16] = { 0 };
    char *ipBuf = strBuf;
    uchar uConnectNum = 0;
    uchar uConnectTime = 0;
    char i = 0;
    uchar ptrBuf[16] = { 0 };
    uchar recvBuf[21] = { 0 };
    uchar index = 0;
    
    printf("\r\n%s->%s %d: Start.\n", __FILE__, __FUNCTION__, __LINE__);
    
    ptrSimBankGainOperationIpReq = (SimBankGainOperationIpReq *)malloc(sizeof(SimBankGainOperationIpReq));
    if (NULL == ptrSimBankGainOperationIpReq)
    {
        printf("malloc ptrSimBankGainOperationIpReq Error.\n");
        
        return 0;
    }
    
    if (WSAStartup(socketVersion, &wsaData) != 0)
    {
        printf("UdpSocketThread() WSAStartup error, return here!\n");
        
        return 0;
    }
    
    SOCKET udpclient;
    sockaddr_in sin;
    
    while (1)
    {
        if (pSimBankIpAddrRecord->netCurrentConnectFlag < 5)  //尝试连接负载均衡服务器
        {
            index = pSimBankIpAddrRecord->netCurrentConnectFlag;
            
            if (uConnectNum <= pSimBankIpAddrRecord->loadBalancingRecord[index].connectNum)
            {
                uConnectNum ++;
                logCurrentTime();
                printf("UdpSocketThread() 尝试连接负载均衡服务器%d，第%d次.\n", index, uConnectNum);
            }
            else
            {
                uConnectNum = 0;
                uConnectTime = 0;
                
                logCurrentTime();
                
                // 0-4 负载
                // 5 默认
                // 6 备份
                for (i = index + 1; i < 5; i++)
                {
                    if ((pSimBankIpAddrRecord->serverIpValidFlag & (1 << i)) != 0)
                    {
                        pSimBankIpAddrRecord->netCurrentConnectFlag = i;
                        printf("UdpSocketThread() 尝试连接负载均衡服务器%d失败，尝试连接负载均衡服务器%d.\n", index, i);
                        
                        break;
                    }
                }
                
                if (i >= 5)
                {
                    pSimBankIpAddrRecord->netCurrentConnectFlag = 5;
                    printf("UdpSocketThread() 负载均衡服务器全部无法连接，切换到默认服务器进行连接.\n");
                    
                    if ((pSimBankIpAddrRecord->serverIpValidFlag & (1 << 6)) == 0x40)
                    {
                        pSimBankIpAddrRecord->netNextConnectFlag = 6;
                        printf("UdpSocketThread()--->下一次连接切换到备份服务器.\n");
                    }
                    else
                    {
                        pSimBankIpAddrRecord->netNextConnectFlag = 0;
                        printf("UdpSocketThread()--->下一次连接切换到负载均衡服务器0（无备份服务器可切换）.\n");
                    }
                }
                
                continue;
            }
            
            udpclient = socket(AF_INET, SOCK_DGRAM, 0);
            
        	int len = sizeof(sin);
            
            index = pSimBankIpAddrRecord->netCurrentConnectFlag;
            sprintf(ipBuf, "%d.%d.%d.%d", pSimBankIpAddrRecord->loadBalancingRecord[index].ipAddr[0],
                                          pSimBankIpAddrRecord->loadBalancingRecord[index].ipAddr[1],
                                          pSimBankIpAddrRecord->loadBalancingRecord[index].ipAddr[2],
                                          pSimBankIpAddrRecord->loadBalancingRecord[index].ipAddr[3]);
            
            sin.sin_family = AF_INET;
            sin.sin_port = htons((ushort)(pSimBankIpAddrRecord->loadBalancingRecord[index].portAddr));
            sin.sin_addr.S_un.S_addr = inet_addr(ipBuf);
            
            int iMode = 1;  //0：阻塞
            ioctlsocket(udpclient, FIONBIO, (u_long FAR*) &iMode);  //非阻塞设置
            
            logCurrentTime();
            SimBankSendOperIpReqPacket();
            memcpy(ptrBuf, ptrSimBankGainOperationIpReq, sizeof(SimBankGainOperationIpReq));
            
            for (i = 0; i < sizeof(SimBankGainOperationIpReq); i++)
            {
                printf("%02x ", ptrBuf[i]);
            }
            printf("\n");
            
            sendto(udpclient, (char *)&ptrBuf, sizeof(SimBankGainOperationIpReq), 0, (sockaddr *)&sin, len);
            
            memset(recvData, 0, sizeof(recvData));
            
            for (uConnectTime = 0; uConnectTime < pSimBankIpAddrRecord->loadBalancingRecord[index].overTimes; uConnectTime ++)
            {
                ret = recvfrom(udpclient, recvData, 64, 0, (sockaddr *)&sin, &len);
                if (ret > 0)
                {
                    memset(recvBuf, 0 , 21);
                    memcpy(recvBuf, (uchar *)&recvData, 21);
                    
                    logCurrentTime();
                    printf("UdpSocketThread() 收到UDP报文.长度 = %d.报文内容：\n", ret);
                    for (i = 0; i < ret; i++)
                    {
                        printf("%02x ", recvBuf[i]);
                    }
                    printf("\n");
                    
                    ret = SimBankOperIpRspPacketHandle((uchar *)&recvBuf, (uchar)ret);
                    if (ret >= 0)
                    {
                        uConnectNum = 0;
                        pSimBankIpAddrRecord->netCurrentConnectFlag = 5;  //切换到默认服务器进行连接
                        pSimBankIpAddrRecord->netNextConnectFlag = 0;
                        
                        printf("UdpSocketThread()--->负载均衡服务器%d返回的报文解析正确.\n", index);
                        printf("UdpSocketThread()--->切换到下发的默认服务器进行连接.\n");
                        printf("UdpSocketThread()--->下次连接切换到负载均衡服务器0.\n");
                    }
                    else
                    {
                        printf("UdpSocketThread() 数据解析错误，重新发送业务IP申请报文.\n");
                    }
                    
                    uConnectTime = pSimBankIpAddrRecord->loadBalancingRecord[index].overTimes;
                }
                else
                {
                    printf("UdpSocketThread() 未收到负载均衡服务器%d发的数据.uConnectTime = %d.\n", index, uConnectTime);
                }
                
                if (uConnectTime != pSimBankIpAddrRecord->loadBalancingRecord[index].overTimes)
                    Sleep(1000);  // 1s
            }
        }
        else
        {
            uConnectNum = 0;
            uConnectTime = 0;
            
            Sleep(300);
        }
    }
    
    closesocket(udpclient);
    WSACleanup();
    
    printf("\r\n%s->%s %d: End.Error!!!\n", __FILE__, __FUNCTION__, __LINE__);
    
    return 0;
}
