#pragma once

#pragma once

#include <Windows.h>
#include <tlhelp32.h>
#include <winioctl.h>
#include <stdint.h>
#include <Winternl.h>

#include <iostream>

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

//��������
struct KB_OPEN_PROCESS_IN
{
	UINT64 ProcessId;
	ACCESS_MASK Access;
	ULONG Attributes;
};

//�������
struct KB_OPEN_PROCESS_OUT
{
	HANDLE hProcess;
};

//�رս���
struct KB_CLOSE_HANDLE_IN
{
	HANDLE Handle;
};

//��ȡ������Ϣ
struct KB_QUERY_INFORMATION_PROCESS_THREAD_IN
{
	HANDLE Handle;
	PVOID Buffer;
	PULONG ReturnLength;
	ULONG InfoClass;
	ULONG Size;
};

class rm_driver
{
public:
	DWORD32 m_pid;			// ����ID
	DWORD64 m_base;			// ���̻�ַ
	HANDLE m_driver;			// �������

private:
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
		if (m_driver != INVALID_HANDLE_VALUE) return SendIOCTL(m_driver, 0x800 + Index, Input, InputSize, Output, OutputSize);
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

	/* �򿪽��� */
	BOOL WINAPI KbOpenProcess(
		ULONG ProcessId,
		OUT HANDLE* hProcess,
		OPTIONAL ACCESS_MASK Access = PROCESS_ALL_ACCESS,
		OPTIONAL ULONG Attributes = 64 | 512)
	{
		if (!hProcess) return FALSE;
		KB_OPEN_PROCESS_IN Input = {};
		KB_OPEN_PROCESS_OUT Output = {};
		Input.ProcessId = ProcessId;
		Input.Access = Access;
		Input.Attributes = Attributes;
		BOOL Status = KbSendRequest(50, &Input, sizeof(Input), &Output, sizeof(Output));
		*hProcess = Output.hProcess;
		return Status;
	}

	/* �رս��� */
	BOOL WINAPI KbCloseHandle(HANDLE Handle)
	{
		if (!Handle) return FALSE;
		KB_CLOSE_HANDLE_IN Input = {};
		Input.Handle = Handle;
		return KbSendRequest(55, &Input, sizeof(Input));
	}

	/* ��ȡ������Ϣ */
	BOOL WINAPI KbQueryInformationProcess(
		HANDLE hProcess,
		INT ProcessInfoClass,
		OUT PVOID Buffer,
		ULONG Size,
		OPTIONAL OUT PULONG ReturnLength = NULL)
	{
		ULONG RetLength = 0;
		KB_QUERY_INFORMATION_PROCESS_THREAD_IN Input = {};
		Input.Handle = hProcess;
		Input.Buffer = reinterpret_cast<PVOID>(Buffer);
		Input.ReturnLength = reinterpret_cast<PULONG>(&RetLength);
		Input.InfoClass = static_cast<ULONG>(ProcessInfoClass);
		Input.Size = Size;
		BOOL Status = KbSendRequest(56, &Input, sizeof(Input));
		if (ReturnLength) *ReturnLength = RetLength;
		return Status;
	}

	/* ���ҽ���ID */
	DWORD32 get_process_id(const wchar_t* process)
	{
		HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (Snap == INVALID_HANDLE_VALUE) return false;

		PROCESSENTRY32W ProcessInfo{ 0 };
		ProcessInfo.dwSize = sizeof(ProcessInfo);

		if (Process32FirstW(Snap, &ProcessInfo))
		{
			do
			{
				if (wcscmp(process, ProcessInfo.szExeFile) == 0)
				{
					CloseHandle(Snap);
					return ProcessInfo.th32ProcessID;
				}
			} while (Process32NextW(Snap, &ProcessInfo));
		}

		CloseHandle(Snap);
		return 0;
	}

public:
	rm_driver() : m_pid(0), m_base(0), m_driver(INVALID_HANDLE_VALUE) {}
	~rm_driver()
	{
		if (m_driver != INVALID_HANDLE_VALUE)
			CloseHandle(m_driver);
		m_driver = INVALID_HANDLE_VALUE;
	}

	/* ��ʼ�� */
	bool initialize(const wchar_t* name, const wchar_t* sym = L"\\\\.\\{ED2761FC-91F4-4E1E-A441-19117D9FAC59}")
	{
		m_pid = get_process_id(name);
		if (m_pid == 0) return false;
		std::cout << "[+] Ŀ��������� : "; std::wcout << name << std::endl;
		std::cout << "[+] Ŀ�����ID : " << m_pid << std::endl;

		m_driver = CreateFileW(sym,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (m_driver == INVALID_HANDLE_VALUE) return false;
		std::cout << "[+] �������ƾ�� : " << m_driver << std::endl;

		HANDLE targer;
		if (KbOpenProcess(m_pid, &targer) == FALSE) return false;
		std::cout << "[+] �򿪽��̾�� : " << targer << std::endl;

		PROCESS_BASIC_INFORMATION info{};
		KbQueryInformationProcess(targer, 0, &info, sizeof(info), NULL);

		PEB peb = read<PEB>((DWORD64)info.PebBaseAddress);
		m_base = (DWORD64)peb.Reserved3[1];
		std::cout << "[+] ģ���ַ : 0x" << std::hex << m_base << std::endl;

		KbCloseHandle(targer);
		return m_base;
	}

	/* ��ȡ�ڴ� */
	template<class T>
	T read(DWORD64 addr)
	{
		T result{};
		KbReadProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)&result, (ULONG)sizeof(T));
		return result;
	}

	/* ��ȡ���� */
	char* read_char_array(DWORD64 addr, DWORD32 size)
	{
		char* data = new char[size];
		if (data)
		{
			memset(data, 0, size);
			KbReadProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)data, (ULONG)size);
		}
		return data;
	}

	/* ��ȡ���� */
	wchar_t* read_wchar_array(DWORD64 addr, DWORD32 size)
	{
		wchar_t* data = new wchar_t[size];
		if (data)
		{
			memset(data, 0, size);
			KbReadProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)data, (ULONG)size);
		}
		return data;
	}

	/* ��ȡ���� */
	float* read_float_array(DWORD64 addr, DWORD32 size)
	{
		float* data = new float[size];
		if (data)
		{
			memset(data, 0, size);
			KbReadProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)data, (ULONG)size);
		}
		return data;
	}

	/* ��ȡ���� */
	void read_array(DWORD64 addr, PVOID data, DWORD32 size)
	{
		memset(data, 0, size);
		KbReadProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)data, (ULONG)size);
	}

	/* д���ڴ� */
	template<class T>
	void write(DWORD64 addr, T buf)
	{
		KbWriteProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)&buf, (ULONG)sizeof(T), FALSE);
	}

	/* д������ */
	void write_array(DWORD64 addr, PVOID data, DWORD32 size)
	{
		KbWriteProcessMemory((ULONG)m_pid, (PVOID)addr, (PVOID)data, (ULONG)size, FALSE);
	}
};