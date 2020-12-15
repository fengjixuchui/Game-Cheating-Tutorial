#pragma once

#include<ntifs.h>
#include<windef.h>

/* ���Ը������� */
#define printfs(x, ...) DbgPrintEx(0, 0, x, __VA_ARGS__)

/* ��ȡ�ڴ� */
#define Mdl_Read CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ALL_ACCESS)

/* д���ڴ� */
#define Mdl_Write CTL_CODE(FILE_DEVICE_UNKNOWN,0x801,METHOD_BUFFERED,FILE_ALL_ACCESS)

/* ������Ϣ�Ľṹ */
typedef struct _UserData
{
	DWORD Pid;							//Ҫ��д�Ľ���ID
	DWORD64 Address;				//Ҫ��д�ĵ�ַ
	DWORD Size;							//��д����
	PBYTE Data;								//Ҫ��д������
}UserData, *PUserData;
