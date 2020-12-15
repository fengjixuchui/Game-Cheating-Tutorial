#pragma warning(disable : 4244)
#include "Utils.h"

Util::Util() : m_Handle(INVALID_HANDLE_VALUE), m_ProcessID(0)
{
}

Util::~Util()
{
	if (m_Handle != INVALID_HANDLE_VALUE) CloseHandle(m_Handle);
}

BOOL Util::InitDriver(CONST WCHAR* DriverPath)
{
	return KbLoader::KbLoadAsDriver(DriverPath);
}

BOOL Util::OpenWinProcess(DWORD Process)
{
	if (m_Handle != INVALID_HANDLE_VALUE) CloseHandle(m_Handle);
	m_Handle = INVALID_HANDLE_VALUE;

	// �򿪽���,ע������Ҫ���Ȩ��
	m_Handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, Process);
	if (m_Handle == INVALID_HANDLE_VALUE || m_Handle == NULL)
	{
		MessageBoxA(NULL, "�޷��򿪸ý��̾��,�볢���Թ���Ա��ʽ����", "����", MB_OK | MB_ICONHAND);
		return FALSE;
	}

	// �������ID
	m_ProcessID = Process;

	return TRUE;
}

BOOL Util::OpenWinProcess(CONST WCHAR* ProcessName)
{
	HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snap == INVALID_HANDLE_VALUE) return FALSE;

	PROCESSENTRY32W ProcessInfo{ 0 };
	ProcessInfo.dwSize = sizeof(ProcessInfo);

	BOOL State = Process32First(Snap, &ProcessInfo);
	while (State)
	{
		if (wcscmp(ProcessInfo.szExeFile, ProcessName) == 0)
		{
			CloseHandle(Snap);
			return OpenWinProcess(ProcessInfo.th32ProcessID);
		}

		State = Process32Next(Snap, &ProcessInfo);
	}

	CloseHandle(Snap);
	return FALSE;
}

DWORD64 Util::PatternCheck(
	BYTE* Region, /* �ڴ����� */
	DWORD RegionSize, /* �ڴ������С */
	BYTE* Pattern, /* ģʽ */
	DWORD PatternSize, /* ģʽ��С */
	DWORD StartOffset /*= 0*/)
{
	for (DWORD i = StartOffset; i < RegionSize - PatternSize; i++)
	{
		BOOL State = TRUE;
		for (DWORD j = 0; j < PatternSize; j++)
		{
			// ����
			if (Pattern[j] == '?') continue;

			if (Pattern[j] != Region[i + j])
			{
				State = FALSE;
				break;
			}
		}
		if (State) return i;
	}
	return -1;
}

//BYTE Pattern[] = { 0x91, 0x03, 0x7C, 0x5B, '?', 0x6B, 0xBC, 0xB6, 0xC3, 0x6B };
VOID Util::PatternSearch(
	BYTE* Pattern, /* ģʽ */
	DWORD PatternSize, /* ģʽ��С */
	std::vector<DWORD64>& Result, /* ���ҽ�� */
	DWORD64 StartAddr, /* ���ҵĿ�ʼ��ַ */
	DWORD64 StopAddr)
{
	// ��ַ�������
	while (StartAddr < StopAddr)
	{
		// �ڴ��ѯ
		MEMORY_BASIC_INFORMATION Info{ 0 };
		if (VirtualQueryEx(m_Handle, (LPCVOID)StartAddr, &Info, sizeof(Info)))
		{
			// ��ַ��ǰ�ƶ�
			StartAddr = (DWORD64)Info.BaseAddress + Info.RegionSize;

			// �ڴ��������������
			BOOL State = (Info.State == MEM_COMMIT)
				&& (Info.Protect != PAGE_NOACCESS)
				&& (Info.Protect & PAGE_GUARD) == 0
				&& (Info.AllocationProtect & PAGE_NOCACHE) != PAGE_NOCACHE;
			if (State == FALSE) continue;

			// ��ȡ�ڴ�����
			DWORD64 Base = (DWORD64)Info.BaseAddress;
			BYTE* Buffer = ReadByte(Base, Info.RegionSize);
			if (Buffer)
			{
				DWORD Offset = 0;
				while (TRUE)
				{
					// ģʽƥ��
					DWORD64 Pos = PatternCheck(Buffer, Info.RegionSize, Pattern, PatternSize, Offset);
					if (Pos == -1) break;

					// �����ַ
					Result.push_back(Base + Pos);

					// ��ַ��ǰ�ƶ�
					Offset = Pos + 1;
				}

				delete[] Buffer;
			}
		}
		else
		{
			MessageBoxA(NULL, "�ڴ��������ʧ��", "����", MB_OK | MB_ICONHAND);
			break;
		}
	}
}

VOID Util::PatternSearchX32(
	BYTE* Pattern, /* ģʽ */
	DWORD PatternSize, /* ģʽ��С */
	std::vector<DWORD64>& Result, /* ���ҽ�� */
	DWORD64 StartAddr /*= 0x00010000*/, /* ���ҵĿ�ʼ��ַ */
	DWORD64 StopAddr /*= 0x7ffeffff*/)
{
	return PatternSearch(Pattern, PatternSize, Result, StartAddr, StopAddr);
}

VOID Util::PatternSearchX64(
	BYTE* Pattern, /* ģʽ */
	DWORD PatternSize, /* ģʽ��С */
	std::vector<DWORD64>& Result, /* ���ҽ�� */
	DWORD64 StartAddr /*= 0x0000000000010000*/, /* ���ҵĿ�ʼ��ַ */
	DWORD64 StopAddr /*= 0x00007ffffffeffff*/)
{
	return PatternSearch(Pattern, PatternSize, Result, StartAddr, StopAddr);
}

CHAR* Util::ReadCharStr(DWORD64 Addr, DWORD Size)
{
	CHAR* Buff = new CHAR[Size];
	if (Buff)
	{
		memset(Buff, 0, Size);
		Processes::MemoryManagement::KbReadProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, Buff, Size);
	}
	return Buff;
}

WCHAR* Util::ReadWCharStr(DWORD64 Addr, DWORD Size)
{
	WCHAR* Buff = new WCHAR[Size];
	if (Buff)
	{
		memset(Buff, 0, Size * 2);
		Processes::MemoryManagement::KbReadProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, Buff, Size);
	}
	return Buff;
}

BYTE* Util::ReadByte(DWORD64 Addr, DWORD Size)
{
	BYTE* Buff = new BYTE[Size];
	if (Buff)
	{
		memset(Buff, 0, Size);
		Processes::MemoryManagement::KbReadProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, Buff, Size);
	}
	return Buff;
}

VOID Util::WriteCharStr(DWORD64 Addr, CONST CHAR* Buff, DWORD Size)
{
	Processes::MemoryManagement::KbWriteProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, (PVOID)Buff, Size, FALSE);
}

VOID Util::WriteWCharStr(DWORD64 Addr, CONST WCHAR* Buff, DWORD Size)
{
	Processes::MemoryManagement::KbWriteProcessMemory(m_ProcessID, (WdkTypes::PVOID)Addr, (PVOID)Buff, Size, FALSE);
}