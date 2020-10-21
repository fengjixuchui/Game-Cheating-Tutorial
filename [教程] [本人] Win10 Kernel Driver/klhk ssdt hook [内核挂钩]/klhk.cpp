#include "klhk.hpp"

f_set_hvm_event set_hvm_event = nullptr;
void*** system_dispatch_array = nullptr;
unsigned int* ssdt_service_count = nullptr;
unsigned int* shadow_ssdt_service_count = nullptr;
unsigned int* provider = nullptr;

bool klhk::is_klhk_loaded()
{
	const auto entry = modules::get_ldr_data_by_name(L"klhk.sys");
	return entry != nullptr;
}

bool klhk::initialize()
{
	// ��λSetHvmEvent����
	set_hvm_event = reinterpret_cast<f_set_hvm_event>(utils::find_pattern_km(L"klhk.sys", ".text", "\x48\x83\xEC\x38\x48\x83\x3D", "xxxxxxx"));
	if (!set_hvm_event)
		return false;

	// ��λklhk�����
	auto presult = utils::find_pattern_km(L"klhk.sys", "_hvmcode", "\x4C\x8D\x0D\x00\x00\x00\x00\x4D", "xxx????x");
	if (!presult)
		return false;

	// ϵͳ�ַ�����
	system_dispatch_array = reinterpret_cast<void***>(presult + *reinterpret_cast<int*>(presult + 0x3) + 0x7);

	// ��λssdt
	presult = utils::find_pattern_km(L"klhk.sys", ".text", "\x3B\x1D\x00\x00\x00\x00\x73\x56", "xx????xx");
	if (!presult)
		return false;

	// ssdt����
	ssdt_service_count = reinterpret_cast<unsigned int*>(presult + *reinterpret_cast<int*>(presult + 0x2) + 0x6);

	// ��λshadow ssdt
	presult = utils::find_pattern_km(L"klhk.sys", ".text", "\x89\x05\x00\x00\x00\x00\x8B\xFB", "xx????xx");
	if (!presult)
		return false;

	// shadow ssdt����
	shadow_ssdt_service_count = reinterpret_cast<unsigned int*>(presult + *reinterpret_cast<int*>(presult + 0x2) + 0x6);

	// ��λ�ṩ����
	presult = utils::find_pattern_km(L"klhk.sys", ".text", "\x39\x2D\x00\x00\x00\x00\x75", "xx????x");
	if (!presult)
		return false;

	// �ṩ��������
	provider = reinterpret_cast<unsigned int*>(presult + *reinterpret_cast<int*>(presult + 2) + 0x6);

	return true;
}

bool klhk::hvm_init()
{
	// ״̬�ж�
	if (!provider || !set_hvm_event)
		return false;

	*provider = 4;

	return NT_SUCCESS(set_hvm_event());
}

unsigned int klhk::get_svc_count_ssdt()
{
	return ssdt_service_count ? *ssdt_service_count : 0;
}

unsigned int klhk::get_svc_count_shadow_ssdt()
{
	return shadow_ssdt_service_count ? *shadow_ssdt_service_count : 0;
}

bool klhk::hook_ssdt_routine(ULONG index, void* dest, void** poriginal)
{
	// �����ж�
	if (!system_dispatch_array || !dest || !poriginal)
		return false;

	// ��ȡssdt����
	const auto svc_count = get_svc_count_ssdt();

	// ssdt�����ж�
	if (!svc_count || index >= svc_count)
		return false;

	// ������ַʵ��hook
	*poriginal = *system_dispatch_array[index];
	*system_dispatch_array[index] = dest;

	return true;
}

bool klhk::unhook_ssdt_routine(ULONG index, void* original)
{
	// �����ж�
	if (!system_dispatch_array || !original)
		return false;

	// ��ȡssdt����
	const auto svc_count = get_svc_count_ssdt();

	// ssdt�����͵�ַ�ж�
	if (!svc_count || index >= svc_count || *system_dispatch_array[index] == original)
		return false;

	// ��ԭ��ַʵ�ֽ��hook
	*system_dispatch_array[index] = original;

	return true;
}

bool klhk::hook_shadow_ssdt_routine(ULONG index, void* dest, void** poriginal)
{
	// �����ж�
	if (!system_dispatch_array || !dest || !poriginal)
		return false;

	// ��ȡssdt��shadow ssdt����
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();
	if (!svc_count || !svc_count_shadow_ssdt)
		return false;

	// ���������ַ���
	const auto index_dispatch_table = (index - 0x1000) + svc_count;

	// ��ȡ�ַ�����������
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// ��Ч���ж�
	if (index_dispatch_table >= dispatch_table_limit)
		return false;

	// ������ַʵ��hook
	*poriginal = *system_dispatch_array[index_dispatch_table];
	*system_dispatch_array[index_dispatch_table] = dest;

	return true;
}

bool klhk::unhook_shadow_ssdt_routine(ULONG index, void* original)
{
	// �����ж�
	if (!system_dispatch_array || !original)
		return false;

	// ��ȡssdt��shadow ssdt����
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();
	if (!svc_count || !svc_count_shadow_ssdt)
		return nullptr;

	// ����ַ�������
	const auto index_dispatch_table = (index - 0x1000) + svc_count;

	// ��ȡ�ַ�����������
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// ������Ч�Ժ͵�ַ�ж�
	if (index_dispatch_table >= dispatch_table_limit || *system_dispatch_array[index_dispatch_table] == original)
		return false;

	// ��ԭ��ַ
	*system_dispatch_array[index_dispatch_table] = original;

	return true;
}

void* klhk::get_ssdt_routine(ULONG index)
{
	// ״̬�ж�
	if (!system_dispatch_array)
		return nullptr;

	// ��ȡssdt����
	const auto svc_count = get_svc_count_ssdt();

	// ���ص�ַ
	return (svc_count && index < svc_count) ? *system_dispatch_array[index] : nullptr;
}

void* klhk::get_shadow_ssdt_routine(ULONG index)
{
	// ״̬�ж�
	if (!system_dispatch_array)
		return false;

	// ��ȡssdt��shadow ssdt����
	const auto svc_count = get_svc_count_ssdt(), svc_count_shadow_ssdt = get_svc_count_shadow_ssdt();
	if (!svc_count || !svc_count_shadow_ssdt)
		return nullptr;

	// ����ַ�������
	const auto index_dispatch_table = (index - 0x1000) + svc_count;

	// ����ַ�����������
	const auto dispatch_table_limit = svc_count + svc_count_shadow_ssdt;

	// ���ص�ַ
	return (index_dispatch_table < dispatch_table_limit) ? *system_dispatch_array[index_dispatch_table] : nullptr;
}