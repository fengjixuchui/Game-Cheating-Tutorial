#include "hooks.hpp"

/* �������� */
EXTERN_C VOID DriverUnload(PDRIVER_OBJECT object)
{
	// δ����
	UNREFERENCED_PARAMETER(object);

	// unhookȫ��
	hooks::start_all_un_hook();

	// �ӳ��˳�
	LARGE_INTEGER LargeInteger{ };
	LargeInteger.QuadPart = -10000000;
	KeDelayExecutionThread(KernelMode, FALSE, &LargeInteger);

	printfs("����ж�����");
}

/* ������� */
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT object, PUNICODE_STRING reg)
{
	// δ����
	UNREFERENCED_PARAMETER(reg);

	// ����ж�غ���
	object->DriverUnload = DriverUnload;

	if (hooks::initialize())					// ��ʼ��
		if (hooks::start_all_hook())		// hookȫ��
			printfs("ȫ��hook���");

	printfs("�����������");
	return STATUS_SUCCESS;
}