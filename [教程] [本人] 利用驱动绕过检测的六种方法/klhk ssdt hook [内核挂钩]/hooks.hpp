#pragma once

#include "ssdt_index.hpp"
#include "klhk.hpp"

namespace hooks
{
	// ��ʼ��
	bool initialize();

	// all hook
	bool start_all_hook();

	// all unhook
	bool start_all_un_hook();
}