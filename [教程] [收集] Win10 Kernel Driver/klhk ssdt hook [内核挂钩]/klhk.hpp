#pragma once

#include "module.hpp"

#include <ntifs.h>
#include <windef.h>

using f_set_hvm_event = NTSTATUS(*)();

namespace klhk
{
	// �ж�klhk�����Ƿ����
	bool is_klhk_loaded();

	// ��ʼ��
	bool initialize();

	// hvm��ʼ��
	bool hvm_init();

	// ��ȡssdt����
	unsigned int get_svc_count_ssdt();

	// ��ȡshadow ssdt����
	unsigned int get_svc_count_shadow_ssdt();

	// �ҹ�ָ��������ssdt����
	bool hook_ssdt_routine(ULONG index, void* dest, void** poriginal);

	// ��ԭָ��������ssdt����
	bool unhook_ssdt_routine(ULONG index, void* original);

	// �ҹ�ָ��������shadow ssdt����
	bool hook_shadow_ssdt_routine(ULONG index, void* dest, void** poriginal);

	// ��ԭָ��������shadow ssdt����
	bool unhook_shadow_ssdt_routine(ULONG index, void* original);

	// ��ȡָ��������ssdt������ַ
	void* get_ssdt_routine(ULONG index);

	// ��ȡָ��������shadow ssdt������ַ
	void* get_shadow_ssdt_routine(ULONG index);
}