#include "memory.hpp"

#include <iostream>
#include <string>
#include <regex>

/*
APEX Dump
*/

void dump()
{
	HANDLE driver = open_device();
	if (driver == INVALID_HANDLE_VALUE)
	{
		std::cout << "[-] ��������δ����" << std::endl;
		return;
	}

	DWORD32 pid = 0;
	std::cout << "[+] ����APEX����ID : ";
	std::cin >> pid;

	DWORD64 base = 0;
	std::cout << "[+] ����APEX���̻�ַ : ";
	std::cin >> base;

	std::cout << "[+] Ŀ����� : " << pid << "\t Ŀ���ַ : 0x" << std::hex << base << std::endl;

	// ��ȡdosͷ
	IMAGE_DOS_HEADER dos = read<IMAGE_DOS_HEADER>(pid, base);
	if (dos.e_magic != IMAGE_DOS_SIGNATURE)
	{
		std::cout << "[-] DOSͷ����" << std::endl;
		return;
	}
	else std::cout << "[+] DOSͷƫ��Ϊ : 0x" << dos.e_lfanew << std::endl;

	// ��ȡNTͷ
	IMAGE_NT_HEADERS64 nt64 = read<IMAGE_NT_HEADERS64>(pid, base + dos.e_lfanew);
	if (nt64.Signature != IMAGE_NT_SIGNATURE || nt64.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		std::cout << "[-] NT64ͷ����" << std::endl;
		return;
	}

	std::cout << "[+] ʱ��� : 0x" << nt64.FileHeader.TimeDateStamp << std::endl;
	std::cout << "[+] У��� : 0x" << nt64.OptionalHeader.CheckSum << std::endl;
	std::cout << "[+] ӳ���С : 0x" << nt64.OptionalHeader.SizeOfImage << std::endl;

	const size_t target_len = nt64.OptionalHeader.SizeOfImage;
	uint8_t* data = read_array(pid, base, target_len);
	if (data == nullptr)
	{
		std::cout << "[-] ��ȡAPEX�ڴ�ʧ��" << std::endl;
		return;
	}

	// ��ȡNTָ��
	PIMAGE_NT_HEADERS64 p_nt64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(data + dos.e_lfanew);

	// ��ȡ����ͷ
	PIMAGE_SECTION_HEADER p_section = reinterpret_cast<PIMAGE_SECTION_HEADER>(
		data +
		static_cast<size_t>(dos.e_lfanew) +
		static_cast<size_t>(FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader)) +
		static_cast<size_t>(nt64.FileHeader.SizeOfOptionalHeader));

	// ��������
	for (size_t i = 0; i < nt64.FileHeader.NumberOfSections; i += 1)
	{
		//��ȡ����
		auto& section = p_section[i];

		// �����ַ
		section.PointerToRawData = section.VirtualAddress;
		section.SizeOfRawData = section.Misc.VirtualSize;

		// �ض�λ
		if (!memcmp(section.Name, ".reloc\0\0", 8))
		{
			p_nt64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC] =
			{
				section.VirtualAddress,
				section.Misc.VirtualSize,
			};
		}
	}

	// ����һ���ļ�
	HANDLE dump = CreateFileA("apex.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_COMPRESSED, NULL);
	if (dump != INVALID_HANDLE_VALUE)
	{
		if (WriteFile(dump, data, target_len, NULL, NULL))
			std::cout << "[+] д��ɹ�" << std::endl;
		else
			std::cout << "[-] д��ʧ��" << std::endl;
		CloseHandle(dump);
	}

	// �ͷ��ڴ�
	delete[] data;

	std::cout << "�������" << std::endl;
}

int main(int argc, char* argv[])
{
	dump();

	system("pause");
	return 0;
}