#pragma once

#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <winioctl.h>
#include <stdint.h>

//���ӷ�
const wchar_t* symbols = L"\\\\.\\{ED2761FC-91F4-4E1E-A441-19117D9FAC59}";

//�豸���
HANDLE g_device = INVALID_HANDLE_VALUE;

//��ȡ�ṹ
struct KB_READ_PROCESS_MEMORY_IN
{
	UINT64 ProcessId;
	PVOID BaseAddress;
	PVOID Buffer;
	ULONG Size;
};

//д��ṹ
struct KB_WRITE_PROCESS_MEMORY_IN
{
	UINT64 ProcessId;
	PVOID BaseAddress;
	PVOID Buffer;
	ULONG Size;
	BOOLEAN PerformCopyOnWrite;
};

/* ����IOCTL */
BOOL SendIOCTL(
	IN HANDLE hDevice,
	IN DWORD Ioctl,
	IN PVOID InputBuffer,
	IN ULONG InputBufferSize,
	IN PVOID OutputBuffer,
	IN ULONG OutputBufferSize,
	OPTIONAL OUT PDWORD BytesReturned = NULL,
	OPTIONAL IN DWORD Method = 3)
{
	DWORD RawIoctl = CTL_CODE(0x8000, Ioctl, Method, FILE_ANY_ACCESS);
	DWORD Returned = 0;
	BOOL Status = DeviceIoControl(hDevice, RawIoctl, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, &Returned, NULL);
	if (BytesReturned) *BytesReturned = Returned;
	return Status;
}

/* �������� */
BOOL KbSendRequest(
	int Index,
	IN PVOID Input = NULL,
	ULONG InputSize = 0,
	OUT PVOID Output = NULL,
	ULONG OutputSize = 0)
{
	if (g_device) return SendIOCTL(g_device, 0x800 + Index, Input, InputSize, Output, OutputSize);
	else MessageBoxA(0, 0, 0, 0);
	return 0;
}

/* д���ڴ� */
BOOL KbWriteProcessMemory(
	ULONG ProcessId,
	OUT PVOID BaseAddress,
	IN PVOID Buffer,
	ULONG Size,
	BOOLEAN PerformCopyOnWrite = FALSE)
{
	if (!ProcessId || !BaseAddress || !Buffer || !Size) return FALSE;
	KB_WRITE_PROCESS_MEMORY_IN Input = {};
	Input.ProcessId = ProcessId;
	Input.BaseAddress = BaseAddress;
	Input.Buffer = reinterpret_cast<PVOID>(Buffer);
	Input.Size = Size;
	Input.PerformCopyOnWrite = PerformCopyOnWrite;
	return KbSendRequest(65, &Input, sizeof(Input));
}

/* ��ȡ�ڴ� */
BOOL KbReadProcessMemory(
	ULONG ProcessId,
	IN PVOID BaseAddress,
	OUT PVOID Buffer,
	ULONG Size)
{
	if (!ProcessId || !BaseAddress || !Buffer || !Size) return FALSE;
	KB_READ_PROCESS_MEMORY_IN Input = {};
	Input.ProcessId = ProcessId;
	Input.BaseAddress = BaseAddress;
	Input.Buffer = reinterpret_cast<PVOID>(Buffer);
	Input.Size = Size;
	return KbSendRequest(64, &Input, sizeof(Input));
}

/* ��ȡ�ڴ� */
template<class T>
T read(DWORD32 process_id, DWORD64 addr)
{
	T result{};
	int size = sizeof(T);

	KbReadProcessMemory((ULONG)process_id, (PVOID)addr, (PVOID)&result, (ULONG)size);
	return result;
}

/* д���ڴ� */
template<class T>
void write(DWORD32 process_id, DWORD64 addr, T buf)
{
	int size = sizeof(T);
	KbWriteProcessMemory((ULONG)process_id, (PVOID)addr, (PVOID)&buf, (ULONG)size, FALSE);
}

/* ���豸 */
HANDLE open_device(LPCWSTR NativeDeviceName = symbols)
{
	g_device = CreateFileW(NativeDeviceName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return g_device;
}

/* ��ȡ����ID */
int get_process_id(const wchar_t* process)
{
	HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snap == INVALID_HANDLE_VALUE) return false;

	PROCESSENTRY32W ProcessInfo{ 0 };
	ProcessInfo.dwSize = sizeof(ProcessInfo);

	if (Process32First(Snap, &ProcessInfo))
	{
		do
		{
			if (wcscmp(process, ProcessInfo.szExeFile) == 0)
			{
				CloseHandle(Snap);
				return ProcessInfo.th32ProcessID;
			}
		} while (Process32Next(Snap, &ProcessInfo));
	}

	CloseHandle(Snap);
	return 0;
}

/* ����ָ��ģ�� */
MODULEENTRY32W find_module(const wchar_t* name, int pid)
{
	MODULEENTRY32W Result{ 0 };
	Result.dwSize = sizeof(Result);

	HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
	if (Snap == INVALID_HANDLE_VALUE)
	{
		int code = GetLastError();
		return Result;
	}

	if (Module32First(Snap, &Result))
	{
		do
		{
			if (wcscmp(name, Result.szModule) == 0)
			{
				CloseHandle(Snap);
				return Result;
			}
		} while (Module32Next(Snap, &Result));
	}

	CloseHandle(Snap);
	return {};
}