#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Serial_port.h"
#include "cardRead.h"
#include "socket_client.h"

#include   <time.h>   

//SimData simdata;
extern SimData * simdataCurr;
/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
//extern volatile bool isProcessComReq;
extern HANDLE hSerialMutex;
/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
extern SOCKET sockClient;

bool readCmd(HANDLE hCom, 
				unsigned char fileType, 
				unsigned char recordLen, 
				unsigned char recordNum,
				SimData *simdataCurr_p)
{
	unsigned char parckedApdu_send[271];
	unsigned char parckedApdu_recv[267] = {0};
	unsigned int SendLength=0;
	unsigned int ReceiveLength = 0;
	unsigned short sendApdulength = 0;
	unsigned short receApdulength = 0;
	SimStatus swStatus = SIM_NOK;
	#if 0
	SYSTEMTIME sys;
	struct   tm   *tmNow;     
	
    time_t   long_time;     
    time(&long_time   );                             /*   Get   time   as   long   integer.   */     
    #endif
	
	SendLength = setReadCmd(simdataCurr_p->requestApdu,fileType, recordLen, recordNum,simdataCurr_p);
	sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
	#if 0
	tmNow   =   LOCALTIME_R(   &long_time   );   /*   Convert   to   local   time.   */     
    printf("%d年%d月%d日   %d时%d分%d秒",tmNow->tm_year,   tmNow->tm_mon   +   1,       
        tmNow->tm_mday,   tmNow->tm_hour,   tmNow->tm_min,   tmNow->tm_sec);    

	
	GetLocalTime( &sys );

	printf( "\nM/%d/%d %d:%d:%d.%d 星期\n"

	,sys.wYear,sys.wMonth,sys.wDay

	,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds

	,sys.wDayOfWeek);
	#endif
	/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	/*
	printf("\n readCmd before while,isProcessComReq:%d\n", isProcessComReq);
	while(isProcessComReq == true);
	isProcessComReq = true;
	*/
	// Wait for Serial to be available, then lock it.
    WaitForSingleObject( hSerialMutex, INFINITE );
	Send_driver_T(hCom, parckedApdu_send, sendApdulength);
	//Sleep(100);
	//Sleep(2);
	
	//Receive data from serial port
	Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
	//isProcessComReq = false;
	ReleaseMutex( hSerialMutex );
	/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	#if 0
	tmNow   =   LOCALTIME_R(   &long_time   );   /*   Convert   to   local   time.   */     
    printf("%d年%d月%d日   %d时%d分%d秒",tmNow->tm_year,   tmNow->tm_mon   +   1,       
        tmNow->tm_mday,   tmNow->tm_hour,   tmNow->tm_min,   tmNow->tm_sec);  

	GetLocalTime( &sys );

	printf( "\nM/%d/%d %d:%d:%d.%d 星期\n"

	,sys.wYear,sys.wMonth,sys.wDay

	,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds

	,sys.wDayOfWeek);
	#endif
	
	receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);

	//check SW
	unsigned char *sw = new unsigned char[2];
	sw = &simdataCurr_p->responseApdu[receApdulength -2];
	swStatus = checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p);
	/*Send GET RESPONSE to GET value back*/
	if(swStatus == SIM_OK)
	{
		if(simdataCurr_p->bytesToRead > 0)
		{
			SendLength = setGetResponseCmd(simdataCurr_p->bytesToRead,simdataCurr_p->requestApdu,simdataCurr_p);
			sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
			/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
			/*
            printf("\n readCmd before while,isProcessComReq:%d\n", isProcessComReq);
			while(isProcessComReq == true);
	  		isProcessComReq = true;
	  		*/
			WaitForSingleObject( hSerialMutex, INFINITE );
			Send_driver_T(hCom, parckedApdu_send, sendApdulength);

			//Receive data from serial port
			Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
			//isProcessComReq = false;
			ReleaseMutex( hSerialMutex );
			/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
			
			receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);
			sw = &simdataCurr_p->responseApdu[receApdulength -2];
			if(checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p)== SIM_OK)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return TRUE;
		}
		
	}
	else
	{
		return FALSE;
	}
	
	
	
}


bool ProcessReadCmd(HANDLE hCom, 
						unsigned char fileType, 
						unsigned short recordLen, 
						unsigned char recordNum,
						SimFile *currentFile,
						SimData *simdataCurr_p)
{
	bool result = FALSE;
	bool saveData = FALSE;
	unsigned short LengthLoc = recordLen;
	unsigned long savedDataNum = 0;
	if(currentFile != NULL)
	{
		saveData = TRUE;
	}
	switch(fileType)
	{
		case TRANSPRANTFILE:
			if(saveData == TRUE)
			{
				currentFile->dataLen = recordLen;
				/*Get the memory for file data, need to delete when application ends*/
				printf("\n Transpart file ,  currentFile->dataLen = %d\n", currentFile->dataLen);
				currentFile->data = new unsigned char[currentFile->dataLen];
				memset(currentFile->data, 0x00, sizeof(unsigned char)*currentFile->dataLen);
			}
			if(recordLen >255)
			{
				while(LengthLoc > 255)
				{
					printf("\n ProcessReadCmd read recordLen > 255, LengthLoc=%d \n", LengthLoc);
					result = readCmd(hCom, fileType, 255, recordNum, simdataCurr_p);
					if((saveData == TRUE) && (result== TRUE))
					{
						memcpy(&currentFile->data[savedDataNum], simdataCurr_p->responseApdu, sizeof(unsigned char)*255);
						savedDataNum += 255;
					}
					LengthLoc -= 255;
					printf("\n ProcessReadCmd LengthLoc=%d \n", LengthLoc);
				}
				
				if(LengthLoc > 0)
				{
					result = readCmd(hCom, fileType, LengthLoc, recordNum, simdataCurr_p);
					if((saveData == TRUE) && (result== TRUE))
					{
						memcpy(&currentFile->data[savedDataNum], simdataCurr_p->responseApdu, sizeof(unsigned char)*LengthLoc);
						savedDataNum += LengthLoc;
					}					
					
				}
				//result = TRUE;
			}
			else
			{
				result = readCmd(hCom, fileType, (unsigned char)recordLen, recordNum, simdataCurr_p);
				if((saveData == TRUE) && (result== TRUE))
				{
					memcpy(&currentFile->data[savedDataNum], simdataCurr_p->responseApdu, sizeof(unsigned char)*recordLen);
					savedDataNum += recordLen;
				}
				//result = TRUE;
			}
			break;
		case LINEARFILE:
			if(saveData == TRUE)
			{
				currentFile->dataLen = recordLen * recordNum;
				/*Get the memory for file data, need to delete when application ends*/
				printf("\nLinear file ,  currentFile->dataLen = %d\n", currentFile->dataLen);
				currentFile->data = new unsigned char[currentFile->dataLen];
				memset(currentFile->data, 0x00, sizeof(unsigned char)*currentFile->dataLen);
			}
			for(unsigned char i = 1; i<= recordNum; i++)
			{
				result = readCmd(hCom, fileType, (unsigned char)recordLen, i,simdataCurr_p);
				if((saveData == TRUE) && (result== TRUE))
				{
					memcpy(&currentFile->data[savedDataNum], simdataCurr_p->responseApdu, sizeof(unsigned char)*recordLen);
					if(i <= recordNum)  /*make sure the savedDataNum equal the file data byte nums*/
					{
						savedDataNum += recordLen;
					}
						
				}
				//result = TRUE;
			}
			break;

		default:
			printf("ProcessReadCmd: fileType error\r\n");
			result = FALSE;
	}
	
	return result;
	
}

bool processSelectCmd(HANDLE hCom, unsigned char *fileId, bool fcp, SimData *simdataCurr_p)
{

	 unsigned char parckedApdu_send[271];		/*262+9(CR6403 req head and tail)*/
	 unsigned char parckedApdu_recv[267] = {0};  /*255 + 2(sw)+10(CR6403 rsp head and tail)*/
	 unsigned int SendLength=0;
	 unsigned int ReceiveLength = 0;
	 unsigned short sendApdulength = 0;
	 unsigned short receApdulength = 0;
	 SimStatus swStatus = SIM_NOK;

	SendLength = setSelectCmd(fileId, simdataCurr_p->requestApdu,0x0,fcp,simdataCurr_p);
	sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
	/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	/*
	printf("\n processSelectCmd before while,isProcessComReq:%d\n", isProcessComReq);
	while(isProcessComReq == true);
	  isProcessComReq = true;
	*/
	WaitForSingleObject( hSerialMutex, INFINITE );
	Send_driver_T(hCom, parckedApdu_send, sendApdulength);
	//Sleep(100);
	//Sleep(2);
	
	//Receive data from serial port
	Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
	//isProcessComReq = false;
	ReleaseMutex( hSerialMutex );
	/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	
	memset(simdataCurr_p->responseApdu,0x00,262);
	receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);

	simdataCurr_p->currentFileId[0] = fileId[0];
	simdataCurr_p->currentFileId[1] = fileId[1];
    
	//check SW
	unsigned char *sw = new unsigned char[2];	 
	sw = &simdataCurr_p->responseApdu[receApdulength -2];
	 
	swStatus = checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p);
	/*Send GET RESPONSE to GET value back*/
	if(swStatus == SIM_OK)
	{
		
		if(simdataCurr_p->bytesToRead > 0)
		{
			SendLength = setGetResponseCmd(simdataCurr_p->bytesToRead,simdataCurr_p->requestApdu,simdataCurr_p);
			sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
			/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
			/*
			printf("\n processSelectCmd before while,isProcessComReq:%d\n", isProcessComReq);
			while(isProcessComReq == true);
	  		isProcessComReq = true;
	  	    */
	  	    WaitForSingleObject( hSerialMutex, INFINITE );
			Send_driver_T(hCom, parckedApdu_send, sendApdulength);

			//Receive data from serial port
			Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
			//isProcessComReq = false;
			ReleaseMutex( hSerialMutex );
			/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
			receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);
			
			sw = &simdataCurr_p->responseApdu[receApdulength -2];
			if(checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p)== SIM_OK)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return TRUE;
		}
		
	}
	else
	{
		return FALSE;
	}
	
	
}

bool processSelectAIDCmd(HANDLE hCom, unsigned char *AID, unsigned char aidLength, SimData *simdataCurr_p )
{
	
	 unsigned char parckedApdu_send[271];
	 unsigned char parckedApdu_recv[267] = {0};
	 unsigned int SendLength=0;
	 unsigned int ReceiveLength = 0;
	 unsigned short sendApdulength = 0;
	 unsigned short receApdulength = 0;
	 SimStatus swStatus = SIM_NOK;

	memset(simdataCurr_p->requestApdu,0,sizeof(unsigned char)*262);
	simdataCurr_p->requestApdu[0]=0x00;
	simdataCurr_p->requestApdu[1]=0xA4;

	/*P1*/
	simdataCurr_p->requestApdu[2]=0x04; //by DF name
	
	/*P2*/
	simdataCurr_p->requestApdu[3]=0x04;
	
	

	/*Lc*/
	simdataCurr_p->requestApdu[4]=aidLength;
	/*FileID*/
	memcpy(&simdataCurr_p->requestApdu[5], AID, sizeof(unsigned char)*aidLength);
	printf("select AID cmd: ");
	for(int i =0; i< 5+aidLength; i++)
	  printf(" %02X", simdataCurr_p->requestApdu[i]);

	printf("\r\n");
	SendLength = 5+aidLength;  /*Add the CLA INS P1 P2 and LC*/

	sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
	/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	/*
	printf("\n processSelectAIDCmd before while,isProcessComReq:%d\n", isProcessComReq);
	while(isProcessComReq == true);
    isProcessComReq = true;
    */
    WaitForSingleObject( hSerialMutex, INFINITE );
	Send_driver_T(hCom, parckedApdu_send, sendApdulength);
//	Sleep(100);
	//Sleep(50);
	
	//Receive data from serial port
	Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
    //isProcessComReq = false;
	ReleaseMutex( hSerialMutex );
	/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);

	//check SW
	unsigned char *sw = new unsigned char[2];
	sw = &simdataCurr_p->responseApdu[receApdulength -2];
	swStatus = checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p);
	/*Send GET RESPONSE to GET value back*/
	if(swStatus == SIM_OK)
	{
		simdataCurr_p->currentFileId[0] = 0x7F;
		simdataCurr_p->currentFileId[1] = 0xFF;
		if(simdataCurr_p->bytesToRead > 0)
		{
			SendLength = setGetResponseCmd(simdataCurr_p->bytesToRead,simdataCurr_p->requestApdu,simdataCurr_p);
			sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
			/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
			/*
			printf("\n processSelectAIDCmd before while,isProcessComReq:%d\n", isProcessComReq);
			while(isProcessComReq == true);
	  		isProcessComReq = true;
	  		*/
	  		WaitForSingleObject( hSerialMutex, INFINITE );
			Send_driver_T(hCom, parckedApdu_send, sendApdulength);

			//Receive data from serial port
			Receive_driver_T(hCom, parckedApdu_recv, &ReceiveLength);
			//isProcessComReq = false;
			ReleaseMutex( hSerialMutex );
			/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
			receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);
			sw = &simdataCurr_p->responseApdu[receApdulength -2];
			if(checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p)== SIM_OK)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
	
		}
		else
		{
			return TRUE;
		}
		
	}
	else
	{
		return FALSE;
	}
	

	
}


unsigned int processPINStatus(unsigned char *fcp,SimData *simdataCurr_p)
{
	unsigned char length;
	unsigned char proLen = 0;
	unsigned char pinstatus = 0;
	unsigned char pinNum = 0;
	length = fcp[1];
	fcp+=2;  //the begining of PIN STATUS DO
	while(proLen < length)
	{
		switch(fcp[0])
		{
			case PINSTATUSDO:
			{
				printf("\nPINSTATUSDO");
				proLen +=  (fcp[1]+2);
				pinstatus = fcp[2];
				fcp += 3;  //jump to the next TLV
				break;
			}
				
			case USAGEQUALIFIERTAG:
			{
				unsigned char usageQualifier = fcp[2];
				bool pinUsed = false;
				bool pinenable = false;
				proLen +=  (fcp[1]+2);
				if((usageQualifier & 0x08)!=0)
					pinUsed = true;
				fcp += 3; //jump to the next TLV
				if(fcp[0] == KEYREFERENCETAG)
				{
					printf("\nkey reference in USAGEQUALIFIERTAG, pinUsed =%d\n", pinUsed);
					proLen +=  (fcp[1]+2);
					if((pinstatus&(0x80>>pinNum))!= 0)
					{
						pinenable = true;
						if(pinUsed == true)
						{
							if(fcp[2] == 0x01)
							{
								simdataCurr_p->pinStatus = USIM_APP_PIN1;
								return 1;
							}
						}
						
					}
					
					fcp += 3; //jump to the next TLV
					pinNum ++;
					
				}
				else
				{
					printf("\n no key reference in USAGEQUALIFIERTAG, error \n");
				}
				break;
			}
			case KEYREFERENCETAG:
			{
				bool pinenable = false;
				proLen +=  (fcp[1]+2);
				printf("\n key reference without USAGEQUALIFIERTAG, pinUsed = true as default, proLen = %d\n", proLen);
				if((pinstatus&(0x80>>pinNum))!= 0)
				{
					pinenable = true;
					if(fcp[2] == 0x01)
					{
						simdataCurr_p->pinStatus = USIM_APP_PIN1;
						return 1;
					}
				}
				
				fcp += 3; //jump to the next TLV
				pinNum ++;
				break;
			}
			default:
			{
				printf("\nprocessPINStatus wrong head");
				proLen = length;	
				//fcp += (fcp[1]+2);
				break;
			}
				
				
		}
	}
	
	return 0;
	
}


unsigned int processFcp(unsigned char *fcp, 
							FileType *fileType, 
							unsigned short *recordLen, 
							unsigned char *recordNum,
							SimData *simdataCurr_p)
{
	unsigned short fcpLen = 0;
	unsigned int processLen = 0;

	
	if(simdataCurr_p->isUsim == true)
	{
		printf("\nprocessFcp, USIM");
		if((fcp != NULL)&&(fcp[0]==FCPTAG))
		{
			/*This three parameters shall be initialized here*/
			*fileType = FILETYPENUM;
			*recordLen = 0;
			*recordNum = 1;
			if(fcp[2] == 0xFF) //Length value >255
			{
				
				fcpLen = fcp[1]*255 + fcp[2];
				fcp+=3;
			}
			else
			{	
				fcpLen =  fcp[1];
				fcp+=2;
			}
			printf("processFcp: fcpLen = %d, processLen = %d, fcp[0] = %02X\r\n", fcpLen, processLen, fcp[0]);

			while(processLen < fcpLen)
			{
				switch(fcp[0])
				{
					case FILEDESCRIPTION:
						if((fcp[2]&0x01) == 0x01)//Transpant files
						{
							*fileType = TRANSPRANTFILE;
						}
						else if((fcp[2]&0x02) == 0x02)//Liner files
						{
							*fileType = LINEARFILE;
							*recordLen = fcp[4]*256+fcp[5];
							*recordNum = fcp[6];
						}
						else if((fcp[2]&0x38) == 0x38) //DF or ADF
						{
							*fileType = DF;
							printf("Process FCP: MF or DF or ADF\r\n");
						}
						else
						{
							printf("Process FCP: Unknow File type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
						}
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						break;
					case FILEIDENTIFIDER:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case DFNAME:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case PROPEIETARYINFO:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;	
					case LIFECYCLESTATUSINTEGER:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case SECURITYATT1:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case SECURITYATT2:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case SECURITYATT3:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case PINSTATUS:	
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						processLen +=  (fcp[1]+2);
						processPINStatus(fcp, simdataCurr_p);
						fcp += fcp[1]+2;
						printf("processFcp: simdataCurr_p->pinStatus = %d, fcpLen = %d, processLen = %d\r\n", simdataCurr_p->pinStatus,fcpLen, processLen);
						break;
					case TOTALFILESIZE:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;	
					case FILESIZE:	
						if(*fileType == TRANSPRANTFILE)
						{
							if(fcp[1] == 0x02)
							 *recordLen = fcp[2]*256 + fcp[3];
							else
							{
								/*may be bigger than 0x02*/
							}
						}
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					case SHORTFILEID:	
						processLen +=  (fcp[1]+2);
						fcp += fcp[1]+2;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
					default:
						processLen = fcpLen;
						//printf("processFcp: fcpLen = %d, processLen = %d\r\n", fcpLen, processLen);
						break;
				}
			}
			printf("processFcp: fileType = %d, recordLen=%d, recordNum=%d\r\n", *fileType, *recordLen, *recordNum );
			
			return (fcpLen+2);
			
		}
		else
		{
			return 0;
		}
	}
	else
	{
		printf("\nprocessFcp, SIM");
		switch(fcp[6])  //type of file
		{
			case 0x01:
			{
				*fileType = MF;
				fcpLen = fcp[12]+13;
				break;
			}
				
			case 0x02:
				*fileType = DF;
				fcpLen = fcp[12]+13;
				break;
			case 0x04:
			{
				fcpLen = fcp[12]+13;
				switch(fcp[13])
				{
					case 0x00:
						*fileType = TRANSPRANTFILE;
						*recordNum = 1;
						*recordLen = fcp[2]*256+fcp[3];
						break;
					case 0x01:
						*fileType = LINEARFILE;
						*recordLen = fcp[14];
						*recordNum = (fcp[2]*256+fcp[3])/(*recordLen);
						break;
					case 0x03:
						*fileType = CIRCLEFILE;
						*recordNum = 1;
						*recordLen = fcp[2]*256+fcp[3];
						break;	
					default:
						*fileType = FILETYPENUM;
						break;	
				}
				break;	
			}
			default:
				*fileType = FILETYPENUM;
				break;	
				
		}
		return fcpLen;
	}
	

	
}

unsigned int saveFcp(unsigned char *fcpwithsw, 
						SimFile *simfile, 
						unsigned short fcpLen,
						FileType  filetype,
						unsigned short recordLen,
						unsigned char recordNum,
						SimData *simdataCurr_p)
{
	simfile->fileId[0] = simdataCurr_p->currentFileId[0];
	simfile->fileId[1] = simdataCurr_p->currentFileId[1];
	simfile->fileType = filetype;
	simfile->fcpLen = fcpLen;
	simfile->recordLen = recordLen;
	simfile->recordNum = recordNum;
	simfile->fcp = new unsigned char[fcpLen];
	memcpy(simfile->fcp, fcpwithsw, sizeof(unsigned char)*fcpLen);
	switch(filetype)
	{
		case TRANSPRANTFILE:
			simfile->dataLen = recordLen;
			break;
		case LINEARFILE:
			simfile->dataLen = recordLen * recordNum;
			break;
		default:
			simfile->dataLen = 0;
			simfile->data = NULL;
			printf("MF DF or ADF, set the datalLen as 0x00\r\n");
			break;
	}
	printf("FILE %02X%02X FCP:",simfile->fileId[0], simfile->fileId[1]);
	for(unsigned int i =0; i<fcpLen; i++)
	  printf(" %02X", simfile->fcp[i]);
	printf("\r\n");

	return fcpLen;
}

int setSelectCmd(unsigned char *fileId,
					unsigned char *requestApdu, 
					char selecttype, 
					bool fcp,
					SimData *simdataCurr_p)
{

	char cmdLength;
	memset(requestApdu,0,sizeof(unsigned char)*262);
	if(simdataCurr_p->isUsim)
	{
		requestApdu[0]=0x00;
		requestApdu[1]=0xA4;

		/*P1*/
		switch (selecttype)
		{
			case 0:
				requestApdu[2]=0x00; //by fileID
				break;
			default:
				break;
		}
		/*P2*/
		if(TRUE == fcp)
			requestApdu[3]=0x04;
		else
			requestApdu[3]=0x00;
	}
	else
	{
		requestApdu[0]=0xA0;
		requestApdu[1]=0xA4;
		requestApdu[2]=0x00; //by fileID
		requestApdu[3]=0x00;
	}
	
	/*Lc*/
	requestApdu[4]=0x02;
	/*FileID*/
	requestApdu[5]=fileId[0];
	requestApdu[6]=fileId[1];
	printf("select cmd: ");
	for(int i =0; i<=6; i++)
	  printf(" %02X", requestApdu[i]);

	printf("\r\n");
	cmdLength = 7;
	return cmdLength;
}

unsigned char setReadCmd(unsigned char *requestApdu,
								unsigned char fileType, 
								unsigned char recordLen, 
								unsigned char recordNum,
								SimData *simdataCurr_p)
{

	char cmdLength;
	memset(requestApdu,0,sizeof(unsigned char)*262);
	if(simdataCurr_p->isUsim)
		requestApdu[0]=0x00;
	else
		requestApdu[0]=0xA0;
	if(fileType == TRANSPRANTFILE)
	{
		requestApdu[1]=0xB0; //read binary
		requestApdu[2]=0x00; //P1
		requestApdu[3]=0x00; //p2  from the offset 0
		requestApdu[4]=recordLen; //Le 
	}
		
	else if(fileType == LINEARFILE)
	{
		requestApdu[1]=0xB2; //read binary
		requestApdu[2]=recordNum; //the record will be read this time
		requestApdu[3]=0x04; //p2  from the offset 0
		requestApdu[4]=recordLen; //Le 
	}
		
	printf("read cmd: ");
	for(int i =0; i<=4; i++)
	  printf(" %02X", requestApdu[i]);

	printf("\r\n");
	cmdLength = 5;
	return cmdLength;
}

int setGetResponseCmd(unsigned char reqDatalen,
								unsigned char *requestApdu,
								SimData *simdataCurr_p)
{
	memset(requestApdu,0,sizeof(unsigned char)*262);
	if(simdataCurr_p->isUsim)
	{
		requestApdu[0]=0x00;
		requestApdu[1]=0xC0;
		/*P1*/
		requestApdu[2]=0x00; //by
		/*P2*/
		requestApdu[3]=0x00;
	}
	else
	{
		requestApdu[0]=0xA0;
		requestApdu[1]=0xC0;
		/*P1*/
		requestApdu[2]=0x00; //by
		/*P2*/
		requestApdu[3]=0x00;
	}
		
	

	/*Lc*/
	requestApdu[4]=reqDatalen;

	printf("Get Response cmd: ");
	for(int i =0; i<=4; i++)
	  printf(" %02X", requestApdu[i]);
	printf("\r\n");
	
	return 5;
	
}

SimStatus checkStatusWord(unsigned char *sw, 
								unsigned char *bytesToRead, 
								unsigned char *bytesToFetch,
								SimData *simdataCurr_p)
{
	SimStatus processResult;
	printf("checkStatusWord: sw1 = %02X, sw2 = %02X\r\n", sw[0],sw[1]);
	*bytesToRead = 0;
	*bytesToFetch = 0;
	simdataCurr_p->sw[0] = sw[0];
	simdataCurr_p->sw[1] = sw[1];
	switch(sw[0])
	{
		case 0x90:
			processResult = SIM_OK;
			break;
		case 0x61:
			*bytesToRead = sw[1];
			processResult = SIM_OK;
			break;
		case 0x91:
			*bytesToFetch = sw[1];
			processResult = SIM_OK;
			break;
		case 0x6A:
			if(sw[1] == 0x82)
			{
				processResult = SIM_FILE_NOT_FOUND;
				printf("checkStatusWord: File not found, ingore\r\n");
			}
			else
			{
				printf("checkStatusWord: File not found, not 0x6a82\r\n");
				processResult = SIM_NOK;
			}
				
			break;
		case 0x9F:
			if(simdataCurr_p->isUsim == false)
			{
				*bytesToRead = sw[1];
				processResult = SIM_OK;
			}
			else
			{
				printf("checkStatusWord: unknow SW, fail\r\n ");
				processResult = SIM_NOK;
			}
			
			break;
		default:
			printf("checkStatusWord: unknow SW, fail\r\n ");
			processResult = SIM_NOK;
			break;
	}
	simdataCurr_p->simstatus = processResult;
	return processResult;
}


unsigned int DecodeAllFileData(unsigned char fileNum,
							SimData *simdataCurr_p,
							unsigned int allDataLen)

{
	//int i ;
	return 0;
}


bool testIsUsimCard(unsigned char *sw)
{
	
	printf("testIsUsimCard: sw1 = %02X, sw2 = %02X\r\n", sw[0],sw[1]);
	switch(sw[0])
	{
		case 0x6E:
			printf("checkStatusWord: 6E, 2G SIM card\r\n ");
			return 0;
			break;
		case 0x6a:
			printf("MF not exit!!!\r\n ");
			return 0;
			break;
		default:
			printf("default as USIM card\r\n ");
			return 1;
			break;
	}
}

bool resetAndAtrCheck3V(HANDLE fd, SimData *simdataCurr_p)
{
	BYTE ResetCmd3V[9] = {0xaa,0xbb,0x05,0x00,0x00,0x00,0x01,0x06,0x51};
    BYTE RecvData[267] = {0};
	unsigned char ATR[100] = {0};
	unsigned char atrLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	unsigned char simNum;
	simNum = simdataCurr_p->simId;
	ResetCmd3V[5] |=(simNum-1)/3 + 1;
	ResetCmd3V[6] |= ((simNum-1)%3 << 4);
	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	  /*
	  printf("\n resetAndAtrCheck3V before while,isProcessComReq:%d\n", isProcessComReq);
	  while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, ResetCmd3V, 0x09))
      {
             printf("\nATR 发送失败!\n");
      }
      else
      {
             printf("\nATR 发送成功!\n");
      }
     // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
      {
             printf("\nATR 接收失败!\n");
			 simdataCurr_p->usimStatus = NO_USIM;
      }
      else
      {
             printf("\nATR 接收成功!\n");
      }
      //Sleep(100);
	  //isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	  //unpack the ATR
	  atrLen = unpack_APDU(RecvData,ATR,realReceiveLeg,CARDRESET3V);
	  if((ATR[0] == 0x3B) || (ATR[0] == 0x3F))
	  {
	  	checkresult = TRUE;
		simdataCurr_p->usimStatus = USIM_INITING;
	  }
	  else
	  {
	  	simdataCurr_p->usimStatus = USIM_OFF;
	  }
	  setCurrentUsimStatus(simdataCurr_p->hCom, simdataCurr_p->simId, simdataCurr_p->usimStatus);
	}
    return checkresult;
	
	
}

bool resetAndAtrCheck5V(HANDLE fd, SimData *simdataCurr_p)
{
	BYTE ResetCmd3V[9] = {0xaa,0xbb,0x05,0x00,0x00,0x00,0x02,0x06,0x51};
    BYTE RecvData[267] = {0};
	unsigned char ATR[100] = {0};
	unsigned char atrLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	unsigned char simNum;
	simNum = simdataCurr_p->simId;
	ResetCmd3V[5] |=(simNum-1)/3 + 1;
	ResetCmd3V[6] |= ((simNum-1)%3 << 4);
	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	  /*
	  printf("\n resetAndAtrCheck5V before while,isProcessComReq:%d\n", isProcessComReq);
		while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, ResetCmd3V, 0x09))
      {
             printf("复位数据 发送失败!\n");
      }
      else
      {
             printf("复位数据 发送成功!\n");
      }
     // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
      {
             printf("ATR数据 接收失败!\n");
			 simdataCurr_p->usimStatus = NO_USIM;
      }
      else
      {
             printf("ATR数据 接收成功!\n");
      }
      //Sleep(100);
	  //isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	  //unpack the ATR
	  atrLen = unpack_APDU(RecvData,ATR,realReceiveLeg,CARDRESET3V);
	  if((ATR[0] == 0x3B) || (ATR[0] == 0x3F))
	  {
	  	checkresult = TRUE;
		simdataCurr_p->usimStatus = USIM_ON;
	  }
	  else
	  {
	  	simdataCurr_p->usimStatus = NO_USIM;
	  }
	}

    return checkresult;
	
	
}

bool HotResetAndAtrCheck(HANDLE fd, SimData *simdataCurr_p)
{
	BYTE ResetCmd3V[9] = {0xaa,0xbb,0x05,0x00,0x00,0x00,0x05,0x06,0x51};
    BYTE RecvData[267] = {0};
	unsigned char ATR[100] = {0};
	unsigned char atrLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	unsigned char simNum;
	simNum = simdataCurr_p->simId;
	ResetCmd3V[5] |=(simNum-1)/3 + 1;
	ResetCmd3V[6] |= ((simNum-1)%3 << 4);
	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
		/*
	  printf("\n HotResetAndAtrCheck before while,isProcessComReq:%d\n", isProcessComReq);	  
	  while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, ResetCmd3V, 0x09))
      {
             printf("复位数据 发送失败!\n");
      }
      else
      {
             printf("复位数据 发送成功!\n");
      }
     // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
      {
             printf("ATR数据 接收失败!\n");
			 simdataCurr_p->usimStatus = NO_USIM;
      }
      else
      {
             printf("ATR数据 接收成功!\n");
      }
      //Sleep(100);
	  //isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	  //unpack the ATR
	  atrLen = unpack_APDU(RecvData,ATR,realReceiveLeg,CARDRESET3V);
	  if((ATR[0] == 0x3B) || (ATR[0] == 0x3F))
	  {
	  	checkresult = TRUE;
		simdataCurr_p->usimStatus = USIM_ON;
	  }
	  else
	  {
	  	simdataCurr_p->usimStatus = NO_USIM;
	  }
	}

    return checkresult;
	
	
}

int processAPDUCmd(unsigned char *apdu, unsigned char apduLen,SimData *simdataCurr_p)
{

	unsigned char parckedApdu_send[271];
	unsigned char parckedApdu_recv[271];
	unsigned int sendApdulength, ReceiveLength, receApdulength;

	memset(simdataCurr_p->requestApdu, 0x00, sizeof(unsigned char)*262 );
	memcpy(simdataCurr_p->requestApdu, apdu, sizeof(unsigned char)*apduLen);
	sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,apduLen,CARDREADWRITE,simdataCurr_p->simId);
	/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	/*
	printf("\n processAPDUCmd before while,isProcessComReq:%d\n", isProcessComReq);
	while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	WaitForSingleObject( hSerialMutex, INFINITE );
	if(simdataCurr_p->hCom != INVALID_HANDLE_VALUE)
		Send_driver_T(simdataCurr_p->hCom, parckedApdu_send, sendApdulength);
	else
	{
		printf("Serial port error:\n");
	}
	//Sleep(20);

	//Receive data from serial port
	Receive_driver_T(simdataCurr_p->hCom, parckedApdu_recv, &ReceiveLength);
	//isProcessComReq = false;
	ReleaseMutex( hSerialMutex );
	/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	memset(simdataCurr_p->responseApdu, 0x00, sizeof(unsigned char)*262 );
	receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);
	
//#ifdef AUTO_GET_RESPONSE
#if 1
 	//check SW
	SimStatus swStatus; 
	int SendLength;
	unsigned char *sw = new unsigned char[2];
	sw = &simdataCurr_p->responseApdu[receApdulength -2];
	swStatus = checkStatusWord(sw,&simdataCurr_p->bytesToRead,&simdataCurr_p->bytesToFetch,simdataCurr_p);
	/*Send GET RESPONSE to GET value back*/
	if(swStatus == SIM_OK)
	{
		if(simdataCurr_p->bytesToRead > 0)
		{
			SendLength = setGetResponseCmd(simdataCurr_p->bytesToRead,simdataCurr_p->requestApdu,simdataCurr_p);
			sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,SendLength,CARDREADWRITE,simdataCurr_p->simId);
			/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
			/*
			while(isProcessComReq == true);
	             isProcessComReq = true;
	        */
	        WaitForSingleObject( hSerialMutex, INFINITE );
			Send_driver_T(simdataCurr_p->hCom, parckedApdu_send, sendApdulength);

			//Receive data from serial port
			Receive_driver_T(simdataCurr_p->hCom, parckedApdu_recv, &ReceiveLength);
			//isProcessComReq = false;
			ReleaseMutex( hSerialMutex );
			/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
			receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);
			
			/*simdataCurr->responseApdu with APDU and  receApdulength is the APDU length*/
			
		}
		
	}	
 #endif

	printf("\n %d bytes send back to the Terminal:", receApdulength);
    if(receApdulength == 0)
    {
    	if(ResetAndReActiveCard(simdataCurr_p)== true)
    	{
    		memset(simdataCurr_p->requestApdu, 0x00, sizeof(unsigned char)*262 );
			memcpy(simdataCurr_p->requestApdu, apdu, sizeof(unsigned char)*apduLen);
			sendApdulength = pack_APDU(simdataCurr_p->requestApdu ,parckedApdu_send,apduLen,CARDREADWRITE,simdataCurr_p->simId);
			/*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
			/*
			printf("\n processAPDUCmd before while,isProcessComReq:%d\n", isProcessComReq);
			while(isProcessComReq == true);
	 		isProcessComReq = true;
	 		*/
	 		WaitForSingleObject( hSerialMutex, INFINITE );
			if(simdataCurr_p->hCom != INVALID_HANDLE_VALUE)
				Send_driver_T(simdataCurr_p->hCom, parckedApdu_send, sendApdulength);
			else
			{
				printf("Serial port error:\n");
			}
			//Sleep(20);

			//Receive data from serial port
			Receive_driver_T(simdataCurr_p->hCom, parckedApdu_recv, &ReceiveLength);
			//isProcessComReq = false;
			ReleaseMutex( hSerialMutex );
			/*2016.04.21 mod by Xili for adding Serial Mutex, end*/
			memset(simdataCurr_p->responseApdu, 0x00, sizeof(unsigned char)*262 );
			receApdulength = unpack_APDU(parckedApdu_recv,simdataCurr_p->responseApdu,ReceiveLength,0);

    	}
    }
	return receApdulength;
	
	
}

bool ResetAndReActiveCard(SimData *simdataCurr_p)
{

	HANDLE hCom = simdataCurr_p->hCom;

	if(resetAndAtrCheck3V(hCom,simdataCurr_p) == TRUE)
	{
		//select MF
		unsigned char fileId[2];
		FileType currentFileType = FILETYPENUM;
		unsigned short recordLen = 0;
		unsigned char recordNum = 0;
		printf("\n/********************Select MF********************/\n");
		fileId[0]= 0x3F;
		fileId[1]= 0x00;
	
		processSelectCmd(hCom,fileId,TRUE,simdataCurr_p);
		if(simdataCurr_p->isUsim==true)
		{
			printf("\n/********************Select EFdir********************/\n");
			//select EFdir
		
			fileId[0]= 0x2F;
			fileId[1]= 0x00;
			processSelectCmd(hCom,fileId,TRUE,simdataCurr_p);
			processFcp(simdataCurr_p->responseApdu, &currentFileType,&recordLen,&recordNum,simdataCurr_p);
			if(simdataCurr_p->simstatus==SIM_OK)
			{
				printf("\n/********************Read AID and Active USIM Application********************/\n");
				{

					printf("\n/********************Read AID and Active USIM Application********************/\n");
					unsigned char AID[16]={0};
					unsigned char AIDLoc = 0;
					unsigned char AIDLen = 0;
					for(unsigned char i = 1; i <= recordNum; i++)
					{
						if(ProcessReadCmd(hCom,currentFileType, recordLen, i, NULL, simdataCurr_p)== TRUE)
						{
							printf("\n/********************Select AID********************/\n");
							/*61 1E 4F 10 A0 00 00 03 43 10 02 FF 86 FF 03 89 FF FF FF FF 
									50 0A 74 69 61 6E 79 75 63 73 69 6D FF FF FF FF FF FF FF
									FF FF FF FF FF FF FF FF FF 90 00
								*/
							AIDLoc = 0;
							AIDLen = 0;
							printf("\nsimdataCurr_p->responseApdu[0]=%d\n",simdataCurr_p->responseApdu[0]);
							if(simdataCurr_p->responseApdu[0] == (unsigned char)APPLICATIONTEMPLATE)
							{
								for(AIDLoc = 1; AIDLoc< recordLen; AIDLoc++)
								{
									if(simdataCurr_p->responseApdu[AIDLoc] == (unsigned char)APPLICATIONID)
									{
										printf("\n/Found a valid AID/\n");
										AIDLen = simdataCurr_p->responseApdu[AIDLoc+1];
										memcpy(AID,&simdataCurr_p->responseApdu[AIDLoc+2], sizeof(unsigned char)*AIDLen);
										break;
									}
										
								}
							}
							/*
							3GPP USIM		'A000000087' '1002' See annex F for further coding details TS 131 102 [11]
							3GPP USIM toolkit	'A000000087' '1003' See annex G for further coding details TS 131 111 [12]
							3GPP ISIM		'A000000087' '1004' See
							3GPP2 CSIM		'A000000343' '1002' see annex F for further coding details
							*/
							
							if((AIDLen != 0)&&
							(AID[4] == 0x87)
							&&(AID[5] == 0x10)
							&&(AID[6] == 0x02))
							{
								printf("\nThis is the USIM application/\n");
								processSelectAIDCmd(hCom, AID, AIDLen,simdataCurr_p);
								/*Save the FCP of select ADF*/
							}
						}
					}
				}
				
				
			}
			
		}
		else
		{
			printf("\n/********************Select DFgsm********************/\n");
			//select DFgsm
			fileId[0]= 0x7F;
			fileId[1]= 0x20;
			processSelectCmd(hCom,fileId,TRUE,simdataCurr_p);
			
		}
		simdataCurr_p->usimStatus = USIM_ON;
		setCurrentUsimStatus(simdataCurr_p->hCom, simdataCurr_p->simId, simdataCurr_p->usimStatus);
		return true;
	}
	else
	{
		simdataCurr_p->usimStatus = USIM_OFF;
		setCurrentUsimStatus(simdataCurr_p->hCom, simdataCurr_p->simId, simdataCurr_p->usimStatus);
		printf("\n!!!!RESET FAIL, CC3310s Fail!!!!\n");
		return false;
	}

	
}

/*
1．	Set Led color : 0x0107
   
Host To Reader;
REQ:  aa bb 06 00 00 00 07 01 03 05    // set Red&green LED on 。 
RSP:  aa bb  06 00 bf  bf 07 01 00 06 
Tenth data is LED parameter ,function as below :
0 =  LED_RED  Off    ,  LED_GREEN  Off
1 =   LED_RED  On   ,  LED_GREEN =  Off
2 =   LED_GREEN On ,  LED_RED   Off
3 =  LED_GREEN  On  ,  LED_RED   ON
*/

bool setLedStatus(HANDLE fd,unsigned char ledStatus)
{
	
	BYTE setLedStatusCmd[10] = {0xaa,0xbb,0x06,0x00,0x00,0x00,0x07,0x01,0x03,0x05};  //
    BYTE RecvData[267] = {0};
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	
	setLedStatusCmd[8] = ledStatus;
	
	if(fd != NULL)
	{
	  if(Send_driver_T(fd, setLedStatusCmd, 0x0A))
      {
             printf("设置LED on 发送失败!\n");
      }
      else
      {
             printf("设置LED on 发送成功!\n");
      }
     // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
      {
             printf("设置LED on 接收失败!\n");
	  }
      else
      {
             printf("设置LED on 接收成功!\n");
      }
      //Sleep(100);

	  //unpack the ATR
	  if(RecvData[8]== 0x00)
	  {
	  	checkresult = true;
		printf("LED set OK\n");
	  }
	  
	}

    return checkresult;
	
	
}



bool getCurrentUsimStatus(HANDLE fd, unsigned char simNum, UsimStatus *usimStatus)
{	
	BYTE getUsimStatus[9] = {0xaa,0xbb,0x05,0x00,0x00,0x00,0x07,0x06,0x51};
	BYTE RecvData[267] = {0};
	unsigned char usimStausRecv[100] = {0};
	unsigned char usimStatusLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	getUsimStatus[5] |=(simNum-1)/3 + 1;
	UsimStatus usimStatusTmp = *usimStatus;/*2016.04.40, mod by Xili for keeping former state if got error state*/
	//getUsimStatus[6] |= ((simNum-1)%3 << 4);
	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	  /*
	  printf("\n getCurrentUsimStatus before while,isProcessComReq:%d\n", isProcessComReq);
	  while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, getUsimStatus, 0x09))
	  {
			 printf("\ngetCurrentUsimStatus send fail");
	  }
	  else
	  {
			 printf("\ngetCurrentUsimStatus send succ\n");
	  }
	  Sleep(50);  //20160825.

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
	  {
			 printf("\ngetCurrentUsimStatus recv fail\n");
			
	  }
	  else
	  {
			 printf("\ngetCurrentUsimStatus recv succ\n");
			 checkresult = true;
	  }
	  //Sleep(100);
	  //isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	  //unpack the SIMstatus	  
	  usimStatusLen = unpack_APDU(RecvData,usimStausRecv,realReceiveLeg,CARDRESET3V);
	  //memcpy(usimStatus, &usimStausRecv[(simNum-1)%3], 1);
	  printf("\n%d %d %d SIM with status: %d %d %d", 
	  		((simNum-1)/3)*3+1, ((simNum-1)/3)*3+2, ((simNum-1)/3)*3+3, usimStausRecv[0],usimStausRecv[1],usimStausRecv[2]);
	  /*2016.04.40, mod by Xili for keeping former state if got error state, begin*/
	  if((usimStausRecv[(simNum-1)%3] >= 0) && (usimStausRecv[(simNum-1)%3] <= 0x06))
	  {
		  *usimStatus = (UsimStatus)usimStausRecv[(simNum-1)%3];
	  }
	  else
	  {
		  printf("\n%d got the status from MCU: %d ", simNum,usimStausRecv[(simNum-1)%3]);
		  *usimStatus = usimStatusTmp;
	  }
	  /*2016.04.40, mod by Xili for keeping former state if got error state, end*/
	  
	   printf("\n%d with status: %d ", simNum,*usimStatus);
	}

	return checkresult;
	
}


bool getCurrentSingleUsimStatus(HANDLE fd, unsigned char simNum, UsimStatus *usimStatus)
{	


	BYTE getUsimStatus[9] = {0xaa,0xbb,0x05,0x00,0x00,0x00,0x07,0x06,0x51};
	BYTE RecvData[267] = {0};
	unsigned char usimStaus[100] = {0};
	unsigned char usimStatusLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	getUsimStatus[5] |=(simNum-1)/3 + 1;
	getUsimStatus[6] |= ((simNum-1)%3 << 4);
	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	  /*
	  printf("\n getCurrentSingleUsimStatus before while,isProcessComReq:%d\n", isProcessComReq);
		while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, getUsimStatus, 0x09))
	  {
			 printf("getCurrentSingleUsimStatus send fail\n");
	  }
	  else
	  {
			 printf("getCurrentSingleUsimStatus send succ\n");
	  }
	 // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
	  {
			 printf("getCurrentSingleUsimStatus recv fail\n");
			
	  }
	  else
	  {
			 printf("getCurrentSingleUsimStatus recv succ\n");
			 checkresult = true;
	  }
	  //Sleep(100);
		//isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	  //unpack the SIMstatus
	  
	  usimStatusLen = unpack_APDU(RecvData,usimStaus,realReceiveLeg,CARDRESET3V);
	  *usimStatus = (UsimStatus)usimStaus[0];
	  printf("getCurrentSingleUsimStatus %d SIM  SIM status is %d\n", simNum, *usimStatus);
	 	 
	}

	return checkresult;
	
}

bool setCurrentUsimStatus(HANDLE fd, unsigned char simNum, UsimStatus usimStatus)
{	

	BYTE setUsimStatus[10] = {0xaa,0xbb,0x06,0x00,0x00,0x00,0x06,0x06,0x00,0x51};
	BYTE RecvData[267] = {0};
	unsigned char usimStaus[100] = {0};
	unsigned char usimStatusLen = 0;
	bool checkresult = false;
	unsigned int realReceiveLeg = 0;
	setUsimStatus[5] |= (simNum-1)/3 + 1;
	setUsimStatus[6] |= ((simNum-1)%3 << 4);
	setUsimStatus[8] |= usimStatus;

	
	if(fd != NULL)
	{
	  /*2016.04.21 mod by Xili for adding Serial Mutex, begin*/
	  /*
	  printf("\n setCurrentUsimStatus before while,isProcessComReq:%d\n", isProcessComReq);
		while(isProcessComReq == true);
	  isProcessComReq = true;
	  */
	  WaitForSingleObject( hSerialMutex, INFINITE );
	  if(Send_driver_T(fd, setUsimStatus, 0x0A))
	  {
			 printf("setCurrentUsimStatus send fail\n");
	  }
	  else
	  {
			 printf("setCurrentUsimStatus send succ\n");
	  }
	 // Sleep(100);

	  if(Receive_driver_T(fd, RecvData, &realReceiveLeg))
	  {
			 printf("setCurrentUsimStatus recv fail\n");
	  }
	  else
	  {
			 printf("setCurrentUsimStatus recv succ\n");
			 checkresult = true;
	  }
	  //isProcessComReq = false;
	  ReleaseMutex( hSerialMutex );
	  /*2016.04.21 mod by Xili for adding Serial Mutex, end*/
	}
	 
	return checkresult;
	
}



unsigned int pack_APDU(unsigned char *apdu, 
						unsigned char *packedApdu, 
						int length, 
						int cmdType, 
						unsigned char simNum)
{
	  // unsigned char simNum = simdataCurr->simId;
	  // printf("\nsimNum is %d\n", simNum);
	   memset(packedApdu,0,sizeof(unsigned char)*271);
	   //check if apdu and packApdu is NULL
	   if(apdu != NULL && packedApdu != NULL)
	   {
		/*aabb 0A00 0000 1306 00 A4 04 00 00 51*/
		packedApdu[0]=0xAA;
		packedApdu[1]=0xBB;

		/*length*/
		if((length + 5)> (unsigned char)0xFF)
		{
		packedApdu[2]=(length+5)&0xFF;
		packedApdu[3]=((length+5)>>8)&0xFF;

		}
		else
		{
		packedApdu[2]=(length+5)&0xFF;
		packedApdu[3]=0x00;
		}
		/*设备标识：2BYTE  在没有指定设备标识时，此数为随机数。*/
		packedApdu[4]=0x00;
		packedApdu[5]=(simNum-1)/3 + 1;
		//ResetCmd3V[5] |=(simNum/3 + 1);
		//ResetCmd3V[6] |= ((simNum%3-1) << 4);
		/*命令码  ：2BYTE  (word)（低字节在前）*/
		switch(cmdType)
		{
    		case CARDRESET3V:
    			packedApdu[6]=(0x01|((simNum-1)%3 <<4));
    			packedApdu[7]=0x06;
    			break;
    		case CARDRESET5V:
    			packedApdu[6]=(0x02|((simNum-1)%3 <<4));
    			packedApdu[7]=0x06;
    			break;
    		case CARDREADWRITE:
    			packedApdu[6]=(0x03|((simNum-1)%3 <<4));
    			packedApdu[7]=0x06;
    			break;
    		case CAERDEACTIVE:
    			packedApdu[6]=(0x04|((simNum-1)%3 <<4));
    			packedApdu[7]=0x06;
    			break;
    		case CARDWARMRESET:
    			packedApdu[6]=(0x05|((simNum-1)%3 <<4));
    			packedApdu[7]=0x06;
    			break;
    		default:
    			printf("pack_APDU, cmdType error\r\n");
    			packedApdu[6]=0x00;
    			packedApdu[7]=0x00;
    			break;
		}
		/*APDU*/
		int i = 0;
		int j = i+8;
		unsigned char checkResult =packedApdu[3]^packedApdu[4]^packedApdu[5]^packedApdu[6]^packedApdu[7];

		for( ; i<length; )
		{
			packedApdu[j] = apdu[i];
			checkResult = checkResult^packedApdu[j];
			if(0xAA==packedApdu[j])
			{	
				j++;
				packedApdu[j] = 0x00;
				checkResult = checkResult^packedApdu[j];
			}
			i++;
			j++;
		}
		packedApdu[j] = checkResult;
		#if 0
		printf("Pack cmd:   ");
		for(int i =0; i<=j; i++)
	  		printf(" %02X", packedApdu[i]);
		printf("\r\n");
		#endif
		return (j+1);
	}

	else
		return 0;
}

unsigned int unpack_APDU(unsigned char *packedApdu, unsigned char *apdu, int length, int cmdType)
{

	   //check if apdu and packApdu is NULL
	   printf("\nunpack_APDU length=%d", length);

       /*2016.03.29, mod by Xili for issue of receiving NULL data from MCU, begin */
	   //if((packedApdu == NULL) || (length == 0))
	   if((packedApdu == NULL) || (length < 10))  //mdf by 20160826.
        {
	   	   EexceptionalCasesRecord(RS485_RECV_PACKET_ERROE);
		   closesocket(sockClient);
           
		   Sleep(5000);
		   printf("\nReceive NULL from MCU, so end process here!!\n");
		   ExitProcess(1);
	   	}
	   /*2016.03.29, mod by Xili for issue of receiving NULL data from MCU, end */
	   
	   if(apdu != NULL && packedApdu != NULL)
	   {
		/*AA BB 08 00 11 12 13 06 00 61 31 46*/
		if((packedApdu[0]==0xAA)&&
			(packedApdu[1]==0xBB)&&
			(packedApdu[8]==0x00))  /*the former command process sucessfully*/
		{
			int i =0;
			int j = i+9;
			printf("\nUnPack cmd:   ");
			for(; j < length;)  //check byte is also maybe 0xAA, we need to delete the following 00
			{
				apdu[i] = packedApdu[j];
				/*if we met AA outside of the cmd, ingore the following 00*/
				if(0xAA == packedApdu[j])
				{	
				  j++;
				}
				//printf(" %02X", apdu[i]);
				i++;
				j++;
				
				
			}
			printf("\r\n");
			return (i-1);  //if we considered the checkbyte, -1 for check bytes is not for APDU
		}
		else
			return 0;
		
	   }
       else
		   return 0;
}


int writeFile(const char* fileName, char* buf, unsigned int bufLen)  
{  
    FILE * fp = NULL;  
	int nWrite; 
	unsigned char writeLoc = 0;
    if( NULL == buf || bufLen <= 0 ) return (-1);  
  
    fp = fopen(fileName, "wb"); // 必须确保是以 二进制写入的形式打开  
  
    if( NULL == fp )  
    {  
    	printf("File open failed in writeFile\n");
        return (-1);  
    }  
		
	for(writeLoc = 0; writeLoc < (bufLen/4098);writeLoc++);
	{
			if( nWrite = fwrite( &buf[writeLoc*4098], sizeof(unsigned char), 4098, (FILE *)fileName ) !=  4098)
			{
				printf("File write error in writeFile\n");
        		return (-1); 
			}
	}
	printf("write the last few bytes in writeFile, writeLoc=%d\n",writeLoc);
	if((bufLen-4098*writeLoc) > 0)
		nWrite = fwrite( &buf[writeLoc*4098], sizeof(unsigned char), (bufLen-4098*writeLoc), (FILE *)fileName );
  	
   
    fclose(fp);  
    fp = NULL;  
  
    return 0;      
}  
int readFile(const char* fileName, char* buf,unsigned int bufLen)  
{  
    FILE* fp = NULL;  
	int nRead; 
	unsigned char readLoc = 0;
    if( NULL == buf || bufLen <= 0 ) return (-1);  
  
    fp = fopen(fileName, "rb"); // 必须确保是以 二进制读取的形式打开   
  
    if( NULL == fp )  
    {  
    	printf("File open failed in ReadFile\n");
        return (-1);  
    }  

	for(readLoc = 0; readLoc < (bufLen/4098);readLoc++ );
	{
			if( nRead = fread( &buf[readLoc*4098], sizeof(unsigned char), 4098,(FILE *)fileName ) !=  4098)
			{
				printf("File read error in ReadFile\n");
        		return (-1); 
			}
	}
	printf("Read the last few bytes in ReadFile, readLoc=%d\n",readLoc);
	if((bufLen-4098*readLoc) > 0)
		nRead = fread( &buf[readLoc*4098], sizeof(unsigned char), (bufLen-4098*readLoc), (FILE *)fileName );
  
    fclose(fp);  
    return 0;          
} 




