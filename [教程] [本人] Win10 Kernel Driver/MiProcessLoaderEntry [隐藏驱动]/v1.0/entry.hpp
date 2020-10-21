#pragma once

#include <ntddk.h>
#include <wdm.h>

// ������ʾ
#define printfs( format, ... ) DbgPrintEx( 0, 0, "[win10 kernel driver hide] : " format "\n", ##__VA_ARGS__ )

// MiProcessLoaderEntry����ָ��
typedef NTSTATUS(__fastcall *f_MiProcessLoaderEntry)(PVOID pDriverSection, BOOLEAN bLoad);

// LDR
typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID      DllBase;
	PVOID      EntryPoint;
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

namespace win10_kernel_driver_hide
{
	// �ж��Ƿ���win10ϵͳ
	EXTERN_C bool is_win10_system()
	{
		RTL_OSVERSIONINFOW os{ 0 };
		os.dwOSVersionInfoSize = sizeof(os);
		if (NT_SUCCESS(RtlGetVersion(&os)))
		{
			printfs("Major : %ld", os.dwMajorVersion);
			printfs("Minor : %ld", os.dwMinorVersion);
			printfs("Platform : %ld", os.dwPlatformId);
			printfs("Build : %ld", os.dwBuildNumber);
		}
		return os.dwMajorVersion == 10;
	}

	// ��ȡMiProcessLoaderEntry����ָ��
	EXTERN_C  f_MiProcessLoaderEntry get_MiProcessLoaderEntry_addr()
	{
		// �ӿ��ٶ�
		static f_MiProcessLoaderEntry res = nullptr;
		if (res != nullptr) return res;

		// ��ȡMmUnloadSystemImage������ַ
		UNICODE_STRING MmUnloadSystemImage_name;
		RtlInitUnicodeString(&MmUnloadSystemImage_name, L"MmUnloadSystemImage");
		ULONG_PTR MmUnloadSystemImage_addr = (ULONG_PTR)MmGetSystemRoutineAddress(&MmUnloadSystemImage_name);
		printfs("MmUnloadSystemImage address is : %x", MmUnloadSystemImage_addr);
		if (MmUnloadSystemImage_addr == 0) return res;

		// ������ʼλ�úͽ���λ��
		ULONG_PTR begin_pos = MmUnloadSystemImage_addr;
		ULONG_PTR end_pos = MmUnloadSystemImage_addr + 0x500;

		// sig
		char MmUnloadSystemImage_sig[] = "\x83\xCA\xFF\x48\x8B\xCF\x48\x8B\xD8\xE8";
		ULONG_PTR MiUnloadSystemImage_addr = 0;

		// ������ʼ
		while (begin_pos < end_pos)
		{
			// �ڴ�ƥ��
			if (memcmp((VOID*)begin_pos, MmUnloadSystemImage_sig, strlen(MmUnloadSystemImage_sig)) == 0)
			{
				begin_pos += strlen(MmUnloadSystemImage_sig);
				MiUnloadSystemImage_addr = *(LONG*)begin_pos + begin_pos + 4;
				break;
			}

			// �൱��ָ���ƶ�
			begin_pos++;
		}

		printfs("MiUnloadSystemImage address is : %x", MiUnloadSystemImage_addr);
		if (MiUnloadSystemImage_addr == 0) return res;

		// ������ʼλ�úͽ���λ��
		begin_pos = MiUnloadSystemImage_addr;
		end_pos = MiUnloadSystemImage_addr + 0x600;

		// ������ʼ
		while (begin_pos < end_pos)
		{
			// ƥ��
			if (*(UCHAR*)begin_pos == 0xE8
				&& *(UCHAR *)(begin_pos + 5) == 0x8B
				&& *(UCHAR *)(begin_pos + 6) == 0x05)
			{
				begin_pos++;
				res = (f_MiProcessLoaderEntry)(*(LONG*)begin_pos + begin_pos + 4);
			}

			// ָ����һ��
			begin_pos++;
		}

		return res;
	}

	// ������������
	EXTERN_C void driver_hide(PDRIVER_OBJECT object)
	{
		// ϵͳ�汾
		if (is_win10_system() == false) return;

		// ����ָ��
		f_MiProcessLoaderEntry function = get_MiProcessLoaderEntry_addr();
		if (function == nullptr) return;

		// ����
		function(object->DriverSection, 0);

		// ����
		PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)object->DriverSection;
		InitializeListHead(&ldr->InLoadOrderLinks);
		InitializeListHead(&ldr->InMemoryOrderLinks);

		// �����ƻ�
		object->DriverUnload = NULL;
		object->DriverSection = NULL;
		object->DriverStart = NULL;
		object->DriverSize = NULL;
		object->DriverInit = NULL;
		object->DeviceObject = NULL;

		printfs("���سɹ�");
	}
}