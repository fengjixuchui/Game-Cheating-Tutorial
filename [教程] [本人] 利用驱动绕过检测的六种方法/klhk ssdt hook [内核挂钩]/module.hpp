#pragma once

#include "util.hpp"

#include <ntifs.h>

/* LDR��ṹ */
typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY	 InLoadOrderLinks;
	void		 *ExceptionTable;
	unsigned int	 ExceptionTableSize;
	void		 *GpValue;
	void		 *NonPagedDebugInfo;
	void	     *DllBase;										//��ַ
	void		 *EntryPoint;								//��ڵ�
	unsigned int	 SizeOfImage;					//ӳ���С
	UNICODE_STRING	 FullDllName;			//ȫ·��
	UNICODE_STRING	 BaseDllName;			//����
	unsigned int	 Flags;
	unsigned __int16 LoadCount;
	unsigned __int16 u1;
	void	         *SectionPointer;
	unsigned int	 CheckSum;
	unsigned int	 CoverageSectionSize;
	void		 *CoverageSection;
	void		 *LoadedImports;
	void		 *Spare;
	unsigned int	 SizeOfImageNotRounded;
	unsigned int	 TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

namespace modules
{
	/* �������ƻ�ȡLDRģ�� */
	PKLDR_DATA_TABLE_ENTRY get_ldr_data_by_name(const wchar_t* szmodule);

	/* �������ƻ�ȡ��ַ */
	uintptr_t get_kernel_module_base(const wchar_t* szmodule);
}

EXTERN_C PLIST_ENTRY PsLoadedModuleList;