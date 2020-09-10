#include "Driver.h"

// {F90B1129-715C-4F84-A069-FEE12E2AFB48}
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\{F90B1129-715C-4F84-A069-FEE12E2AFB48}");
UNICODE_STRING DeviceLink = RTL_CONSTANT_STRING(L"\\??\\{F90B1129-715C-4F84-A069-FEE12E2AFB48}");

//��ȡ�ڴ�
VOID MdlReadProcessMemory(PUserData Buffer)
{
	//��Ŀ�����
	PEPROCESS Process = NULL;
	NTSTATUS Status = PsLookupProcessByProcessId((HANDLE)Buffer->Pid, &Process);
	if (!NT_SUCCESS(Status))
	{
		printfs("[Mdl] : read PsLookupProcessByProcessId����ʧ��");
		return;
	}

	//�����ڴ�ռ�
	PBYTE Temp = ExAllocatePool(PagedPool, Buffer->Size);
	if (Temp == NULL)
	{
		printfs("[Mdl] : read ExAllocatePool����ʧ��");
		ObDereferenceObject(Process);
		return;
	}

	//���ӽ���
	KAPC_STATE Stack = { 0 };
	KeStackAttachProcess(Process, &Stack);

	//�����ڴ�
	ProbeForRead((PVOID)Buffer->Address, Buffer->Size, 1);

	//�����ڴ�
	RtlCopyMemory(Temp, (PVOID)Buffer->Address, Buffer->Size);

	//�������
	ObDereferenceObject(Process);

	//��������
	KeUnstackDetachProcess(&Stack);

	//���Ƶ����ǵĻ�����
	RtlCopyMemory(Buffer->Data, Temp, Buffer->Size);

	//�ͷ��ڴ�
	ExFreePool(Temp);
}

//д���ڴ�
VOID MdlWriteProcessMemory(PUserData Buffer)
{
	//��Ŀ�����
	PEPROCESS Process = NULL;
	NTSTATUS Status = PsLookupProcessByProcessId((HANDLE)Buffer->Pid, &Process);
	if (!NT_SUCCESS(Status))
	{
		printfs("[Mdl] : write PsLookupProcessByProcessId����ʧ��");
		return;
	}

	//�����ڴ�ռ�
	PBYTE Temp = ExAllocatePool(PagedPool, Buffer->Size);
	if (Temp == NULL)
	{
		printfs("[Mdl] : write ExAllocatePool����ʧ��");
		ObDereferenceObject(Process);
		return;
	}

	//�����ڴ�����
	for (DWORD i = 0; i < Buffer->Size; i++) Temp[i] = Buffer->Data[i];

	//���ӽ���
	KAPC_STATE Stack = { 0 };
	KeStackAttachProcess(Process, &Stack);

	//����MDL
	PMDL Mdl = IoAllocateMdl((PVOID)Buffer->Address, Buffer->Size, FALSE, FALSE, NULL);
	if (Mdl == NULL)
	{
		printfs("[Mdl] : IoAllocateMdl����ʧ��");
		KeUnstackDetachProcess(&Stack);
		ExFreePool(Temp);
		ObDereferenceObject(Process);
		return;
	}

	//��������ҳ��
	MmBuildMdlForNonPagedPool(Mdl);

	//����ҳ��
	PBYTE ChangeData = MmMapLockedPages(Mdl, KernelMode);

	//�����ڴ�
	if (ChangeData) RtlCopyMemory(ChangeData, Temp, Buffer->Size);

	//�ͷ�����
	IoFreeMdl(Mdl);
	ExFreePool(Temp);
	KeUnstackDetachProcess(&Stack);
	ObDereferenceObject(Process);
}

//������ǲ����
NTSTATUS DriverIoctl(PDEVICE_OBJECT Device, PIRP pirp)
{
	//δ����
	UNREFERENCED_PARAMETER(Device);

	//��ȡ��ջ
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(pirp);

	//��ȡ������
	ULONG Code = Stack->Parameters.DeviceIoControl.IoControlCode;

	if (Stack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		//��ȡ����ָ��
		PUserData Buffer = pirp->AssociatedIrp.SystemBuffer;
		printfs("[Mdl] : PID:%d  ��ַ:%x  ��С:%d", Buffer->Pid, Buffer->Address, Buffer->Size);

		if (Code == Mdl_Read) MdlReadProcessMemory(Buffer); //��ȡ�ڴ�
		if (Code == Mdl_Write) MdlWriteProcessMemory(Buffer);//д���ڴ�

		pirp->IoStatus.Information = sizeof(UserData);
	}
	else pirp->IoStatus.Information = 0;

	//���IO
	pirp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pirp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

//����ж�غ���
VOID DriverUnload(PDRIVER_OBJECT object)
{
	if (object->DeviceObject)
	{
		IoDeleteSymbolicLink(&DeviceLink);
		IoDeleteDevice(object->DeviceObject);
	}
	printfs("[Mdl] : ����ж�سɹ�");
}

//������ں���
NTSTATUS DriverEntry(PDRIVER_OBJECT object, PUNICODE_STRING reg)
{
	printfs("[Mdl] : ����ע��� -> %wZ", reg);

	//����ж�غ���
	object->DriverUnload = DriverUnload;

	//�����豸
	PDEVICE_OBJECT Device = NULL;
	NTSTATUS Status = IoCreateDevice(object, sizeof(object->DriverExtension), &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &Device);
	if (!NT_SUCCESS(Status))
	{
		printfs("[Mdl] : IoCreateDevice����ʧ��");
		return Status;
	}

	//��������
	Status = IoCreateSymbolicLink(&DeviceLink, &DeviceName);
	if (!NT_SUCCESS(Status))
	{
		printfs("[Mdl] : IoCreateSymbolicLink����ʧ��");
		IoDeleteDevice(Device);
		return Status;
	}

	//������ǲ����
	object->MajorFunction[IRP_MJ_CREATE] = DriverIoctl;
	object->MajorFunction[IRP_MJ_CLOSE] = DriverIoctl;
	object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIoctl;

	printfs("[Mdl] : �������سɹ�");
	return STATUS_SUCCESS;
}