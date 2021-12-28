#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <wincon.h>
#include <winioctl.h>
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib, "Netapi32")

#include <Iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")

////////// -------------- mac ----------------
typedef struct _ASTAT_ {
	ADAPTER_STATUS adapt;
	NAME_BUFFER NameBuff[30];
} ASTAT, *PASTAT;

// 获取第一块网卡的mac地址
bool get_mac(char* mac, int32_t len) {
	ASTAT Adapter;
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM lenum;
	int i = 0;

	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR *)&lenum;
	Ncb.ncb_length = sizeof(lenum);

	uRetCode = Netbios(&Ncb);
	//fprintf(stdout, "The NCBENUM return adapter number is [%d] \n ", lenum.length);
	for (i = 0; i < lenum.length; i++) {
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command  = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios(&Ncb);

		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		strcpy_s((char *)Ncb.ncb_callname, sizeof(Ncb.ncb_callname), "* ");

		Ncb.ncb_buffer = (unsigned char *)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios(&Ncb);
		//fprintf(stdout, "ncb_name[%s] ncb_callname[%s]\n", Ncb.ncb_name, Ncb.ncb_callname);
		if (uRetCode == 0) {
			snprintf(mac, len, "%02X:%02X:%02X:%02X:%02X:%02X",
				Adapter.adapt.adapter_address[0],
				Adapter.adapt.adapter_address[1],
				Adapter.adapt.adapter_address[2],
				Adapter.adapt.adapter_address[3],
				Adapter.adapt.adapter_address[4],
				Adapter.adapt.adapter_address[5]);
			return true;
		}
	} // for
	return false;
}


// 获取ip地址对应的mac地址
bool get_mac_by_ip(const char* ip, char* mac, int32_t len) {
    if (ip == NULL) {
        return get_mac(mac, len);
    }

    unsigned long stSize = 0;
    PIP_ADAPTER_INFO pIpAdapterInfo = NULL;
    GetAdaptersInfo(pIpAdapterInfo, &stSize); // 获取缓冲区大小
    
    PIP_ADAPTER_INFO pNew = (PIP_ADAPTER_INFO) new (std::nothrow) char[stSize];
    pIpAdapterInfo = pNew;

    if (ERROR_SUCCESS == GetAdaptersInfo(pIpAdapterInfo, &stSize)) {
        //可能有多网卡,因此通过循环去判断
        while (pIpAdapterInfo) {
            //可能网卡有多IP,因此通过循环去判断
            IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            while (pIpAddrString) {
                // 相同的ip
                if (strncmp(ip, pIpAddrString->IpAddress.String, 16) == 0) {
                    snprintf(mac, len, "%02X:%02X:%02X:%02X:%02X:%02X",
                        pIpAdapterInfo->Address[0], pIpAdapterInfo->Address[1],
                        pIpAdapterInfo->Address[2], pIpAdapterInfo->Address[3],
                        pIpAdapterInfo->Address[4], pIpAdapterInfo->Address[5]);
                    // fprintf(stdout, "ip[%s] mac[%s] \n", pIpAddrString->IpAddress.String, mac);
                    if (pNew != NULL) {
                        delete[] pNew;
                    }                    
                    return true;
                }
                pIpAddrString = pIpAddrString->Next;
            }

            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }

    if (pNew != NULL) {
        delete[] pNew;
    }
	return false;
}


////////// -------------- hostname ----------------
bool get_hostname(char* hostname, int32_t len) {
	struct hostent *ph = 0;
	WSADATA w;
	WSAStartup(0x0101, &w);//这一行必须在使用任何SOCKET函数前写！
	gethostname(hostname, len);
	WSACleanup();
	return true;
}


////////// ----------- hard disk serial number ----------
//把WORD数组调整字节序为little-endian，并滤除字符串结尾的空格。
static 
void ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, char* pBuf) {
	int index;
	char* pDest = pBuf;
	for (index = nFirstIndex; index <= nLastIndex; ++index) {
		pDest[0] = pWords[index] >> 8;
		pDest[1] = pWords[index] & 0xFF;
		pDest += 2;
	}
	*pDest = 0;

	//trim space at the endof string; 0x20: _T(' ')
	--pDest;
	while (*pDest == 0x20) {
		*pDest = 0;
		--pDest;
	}
}

//滤除字符串起始位置的空格
static
void TrimStart(char* pBuf) {
	if (*pBuf != 0x20)
		return;

	char* pDest = pBuf;
	char* pSrc = pBuf + 1;
	while (*pSrc == 0x20) {
		++pSrc;
	}

	while (*pSrc) {
		*pDest = *pSrc;
		++pDest;
		++pSrc;
	}
	*pDest = 0;
}

// pModelNo,  硬盘型号                              
// pSerialNo, 硬盘序列号
// need admain rights
static 
BOOL GetPhyDriveSerial_AdminRight(char* pModelNo, char* pSerialNo) {
	//-1是因为 SENDCMDOUTPARAMS 的结尾是 BYTE bBuffer[1];
	BYTE IdentifyResult[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	DWORD dwBytesReturned;
	GETVERSIONINPARAMS get_version;
	SENDCMDINPARAMS send_cmd = { 0 };

	for (int i = 0; i < 16; ++i) {
		char diskname[256];
		sprintf_s(diskname, "\\\\.\\PHYSICALDRIVE%d", i);

		HANDLE hFile = CreateFileA(diskname, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			fprintf(stdout, "CreateFile[%s] failed. need admin rights \n", diskname);
			continue;
		}

		//get version
		if (!DeviceIoControl(hFile, SMART_GET_VERSION, NULL, 0, &get_version, 
			sizeof(get_version), &dwBytesReturned, NULL)) {
			fprintf(stdout, "DeviceIoControl[SMART_GET_VERSION] failed.\n");
			CloseHandle(hFile);
			continue;
		}

		//identify device
		send_cmd.irDriveRegs.bCommandReg = (get_version.bIDEDeviceMap & 0x10) ? ATAPI_ID_CMD : ID_CMD;
		if (!DeviceIoControl(hFile, SMART_RCV_DRIVE_DATA, &send_cmd, sizeof(SENDCMDINPARAMS) - 1,
			IdentifyResult, sizeof(IdentifyResult), &dwBytesReturned, NULL)) {
			fprintf(stdout, "DeviceIoControl[SMART_RCV_DRIVE_DATA] failed.\n");
			CloseHandle(hFile);
			continue;
		}
		CloseHandle(hFile);
		
		//adjust the byte order
		PUSHORT pWords = (USHORT*)(((SENDCMDOUTPARAMS*)IdentifyResult)->bBuffer);
		ToLittleEndian(pWords, 27, 46, pModelNo);
		ToLittleEndian(pWords, 10, 19, pSerialNo);
		return true;
	}

	return false;
}

// pModelNo,  硬盘型号
// pSerialNo, 硬盘序列号
// no need admin rights
static
BOOL GetPhyDriveSerial_noAdminRights(char* pModelNo, char* pSerialNo) {
	for (int i = 0; i < 16; ++i) {
		char diskname[256];
		sprintf_s(diskname, "\\\\.\\PHYSICALDRIVE%d", i);

		HANDLE hFile = CreateFileA(diskname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			fprintf(stdout, "CreateFile[%s] failed.\n", diskname);
			continue;
		}

		STORAGE_PROPERTY_QUERY query;
		DWORD cbBytesReturned = 0;
		char buffer[512]      = { 0 };
		memset(&query, 0x00, sizeof(STORAGE_PROPERTY_QUERY));
		query.PropertyId = StorageDeviceProperty;
		query.QueryType  = PropertyStandardQuery;
		if (!DeviceIoControl(hFile, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &buffer, sizeof(buffer), &cbBytesReturned, NULL)) {
			fprintf(stdout, "DeviceIoControl[IOCTL_STORAGE_QUERY_PROPERTY] failed.\n");
			CloseHandle(hFile);
			continue;
		}

        // TODO 在有些平台上，输出需要调整
		STORAGE_DEVICE_DESCRIPTOR * descrip = (STORAGE_DEVICE_DESCRIPTOR *)& buffer;
		memcpy(pModelNo,  ((char*)descrip) + descrip->ProductIdOffset,    20);
		memcpy(pSerialNo, ((char*)descrip) + descrip->SerialNumberOffset, 20);
		CloseHandle(hFile);
		return true;
	}

	return false;
}

bool get_disk_sn(char* sn, int32_t len) {
	char szModelNo[64] = { 0 }, szSerialNo[64] = { 0 };
	if (!GetPhyDriveSerial_AdminRight(szModelNo, szSerialNo)) {
		if (!GetPhyDriveSerial_noAdminRights(szModelNo, szSerialNo)) {
			return false;
		}
	}

	TrimStart(szSerialNo);
	strncpy_s(sn, len, szSerialNo, strlen(szSerialNo));
	return true;
}
