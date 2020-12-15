#pragma once

/************************************************************************/
/*								����һ�¾����õ��ĺ���                                        */
/************************************************************************/

#include "User-Bridge.h"

#include <tlhelp32.h>

#include <iostream>
#include <vector>

class Util
{
private:
	// ����ID
	DWORD m_ProcessID;

	// ���̾��
	HANDLE m_Handle;

public:
	Util();
	~Util();

public:
	/* ��ʼ������ */
	BOOL InitDriver(CONST WCHAR* DriverPath);

	/* ��һ������ */
	BOOL OpenWinProcess(CONST WCHAR* ProcessName);
	BOOL OpenWinProcess(DWORD Process);

	/* ģʽƥ�� */
	DWORD64 PatternCheck(
		BYTE* Region,							// �ڴ�����
		DWORD RegionSize,				// �ڴ������С
		BYTE* Pattern,							// ģʽ
		DWORD PatternSize,				// ģʽ��С
		DWORD StartOffset = 0);			// ��ʼ��λ��ƫ��

	/* ģʽ���� */
	VOID PatternSearch(
		BYTE* Pattern,									// ģʽ
		DWORD PatternSize,						// ģʽ��С
		std::vector<DWORD64>& Result,	// ���ҽ��
		DWORD64 StartAddr,						// ���ҵĿ�ʼ��ַ
		DWORD64 StopAddr);						// ���ҵĽ�����ַ

	/* 32λ��ģʽ���� */
	VOID PatternSearchX32(
		BYTE* Pattern,											// ģʽ
		DWORD PatternSize,								// ģʽ��С
		std::vector<DWORD64>& Result,			// ���ҽ��
		DWORD64 StartAddr = 0x00010000,		// ���ҵĿ�ʼ��ַ
		DWORD64 StopAddr = 0xFFFFFFFF);		// ���ҵĽ�����ַ

	/* 64λ��ģʽ���� */
	VOID PatternSearchX64(
		BYTE* Pattern,															// ģʽ
		DWORD PatternSize,												// ģʽ��С
		std::vector<DWORD64>& Result,							// ���ҽ��
		DWORD64 StartAddr = 0x0000000000010000,			// ���ҵĿ�ʼ��ַ
		DWORD64 StopAddr = 0x00007FFFFFFFFFFF);			// ���ҵĽ�����ַ

	/* ��ȡ�ַ��� */
	CHAR* ReadCharStr(DWORD64 Addr, DWORD Size);
	WCHAR* ReadWCharStr(DWORD64 Addr, DWORD Size);

	/* ��ȡ�ֽ� */
	BYTE* ReadByte(DWORD64 Addr, DWORD Size);

	/* д���ַ��� */
	VOID WriteCharStr(DWORD64 Addr, CONST CHAR* Buff, DWORD Size);
	VOID WriteWCharStr(DWORD64 Addr, CONST WCHAR* Buff, DWORD Size);

	/* ��ȡ�ڴ� */
	template<class T>
	T Read(DWORD64 Addr)
	{
		T Res{};
		Processes::MemoryManagement::KbReadProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, &Res, sizeof(Res));
		return Res;
	}

	/* д���ڴ� */
	template<class T>
	VOID Write(DWORD64 Addr, T Buff)
	{
		Processes::MemoryManagement::KbWriteProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, &Buff, sizeof(Buff), FALSE);
	}
};
