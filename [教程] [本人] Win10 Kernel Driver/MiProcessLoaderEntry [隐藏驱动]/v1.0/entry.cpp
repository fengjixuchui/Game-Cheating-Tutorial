#include "entry.hpp"

VOID re_initialize(PDRIVER_OBJECT DriverObject, PVOID Context, ULONG Count)
{
	// δ����
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Count);

	// ��������
	win10_kernel_driver_hide::driver_hide(DriverObject);
}

/* ������� */
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT object, PUNICODE_STRING reg)
{
	// δ����
	UNREFERENCED_PARAMETER(reg);

	// ���³�ʼ��
	IoRegisterDriverReinitialization(object, re_initialize, NULL);

	printfs("�������سɹ�");
	return STATUS_SUCCESS;
}