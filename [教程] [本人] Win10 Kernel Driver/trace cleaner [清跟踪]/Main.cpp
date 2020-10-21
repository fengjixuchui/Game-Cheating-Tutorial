#include "clean.hpp"

/*
���� : ���������ļ�����/ж�ؼ�¼,��ֹ����������ĸ��ٷ��
ϵͳ : Win10 x64 1809
*/

extern "C" VOID DriverUnload(PDRIVER_OBJECT driver_object)
{
	// δ����
	UNREFERENCED_PARAMETER(driver_object);

	log("[clear] : ����ж��");
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT  driver_object, PUNICODE_STRING registry_path)
{
	// δ����
	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(registry_path);

	// ����ж�غ���
	driver_object->DriverUnload = DriverUnload;

	// ����PiddbCacheTable
	UNICODE_STRING driver_name = RTL_CONSTANT_STRING(L"ReadWriteDriver.sys");
	if (clear::clearCache(driver_name, 0x5F7884B1) == STATUS_SUCCESS)
		log("[clear] : ����PiddbCacheTable�ɹ�");
	else
		log("[clear] : ����PiddbCacheTableʧ��");

	// ����mmUnloadedDrivers
	if (FindMmDriverData() == STATUS_SUCCESS)
	{
		if (clear::ClearUnloadedDriver(&driver_name, true) == STATUS_SUCCESS)
			log("[clear] : ����mmUnloadedDrivers�ɹ�");
		else
			log("[clear] : ����mmUnloadedDriversʧ��");
	}
	else log("[clear] : ����mmUnloadedDrivers��ʧ��");

	return STATUS_SUCCESS;
}