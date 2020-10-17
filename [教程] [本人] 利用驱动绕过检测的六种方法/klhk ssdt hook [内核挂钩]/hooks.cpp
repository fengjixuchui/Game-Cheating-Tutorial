#include "hooks.hpp"

using type_NtCreateFile = NTSTATUS(*)(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_opt_ PLARGE_INTEGER AllocationSize,
	_In_ ULONG FileAttributes,
	_In_ ULONG ShareAccess,
	_In_ ULONG CreateDisposition,
	_In_ ULONG CreateOptions,
	_In_reads_bytes_opt_(EaLength) PVOID EaBuffer,
	_In_ ULONG EaLength);

type_NtCreateFile f_NtCreateFile = nullptr;

NTSTATUS NTAPI MyNtCreateFile(
	_Out_ PHANDLE FileHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_opt_ PLARGE_INTEGER AllocationSize,
	_In_ ULONG FileAttributes,
	_In_ ULONG ShareAccess,
	_In_ ULONG CreateDisposition,
	_In_ ULONG CreateOptions,
	_In_reads_bytes_opt_(EaLength) PVOID EaBuffer,
	_In_ ULONG EaLength)
{
	// �����ж�
	if (ObjectAttributes
		&& ObjectAttributes->ObjectName
		&& ObjectAttributes->ObjectName->Buffer)
	{
		// ��ȡ�ļ�����
		const auto name = ObjectAttributes->ObjectName->Buffer;

		// �����ж�
		if (wcsstr(name, L"you_wont_open_this.txt"))
			return STATUS_ACCESS_DENIED;
	}

	// ����ԭʼ����
	return f_NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
		FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
}

bool hooks::initialize()
{
	if (klhk::is_klhk_loaded() == false)
	{
		printfs("klhk����δ����");
		return false;
	}

	if (klhk::initialize() == false)
	{
		printfs("��ʼ��ʧ��");
		return false;
	}

	if (klhk::hvm_init() == false)
	{
		printfs("hvm��ʼ��ʧ��");
		return false;
	}

	return true;
}

bool hooks::start_all_hook()
{
	bool state = true;

	state &= klhk::hook_ssdt_routine(SSDT_Index::Win10_20H1::NtCreateFile, MyNtCreateFile, reinterpret_cast<void**>(&f_NtCreateFile));

	return state;
}

bool hooks::start_all_un_hook()
{
	bool state = true;

	if (klhk::is_klhk_loaded())
	{
		state &= klhk::unhook_ssdt_routine(SSDT_Index::Win10_20H1::NtCreateFile, f_NtCreateFile);
	}

	return state;
}