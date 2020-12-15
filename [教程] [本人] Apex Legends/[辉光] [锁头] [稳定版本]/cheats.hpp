#pragma once

#include "entity.hpp"

constexpr const int NUM_ENT_ENTRIES = 0x10000;
constexpr const int MAX_PLAYERS = 100;

client_info g_clients[NUM_ENT_ENTRIES];		// �ͻ�����Ϣ

class apex_cheats
{
private:
	rm_driver m_driver;		// ��д
	HWND m_hwnd;			// ��Ϸ����

	entity m_local;				// �������

	entity m_players[MAX_PLAYERS];					// ����б�

public:
	apex_cheats() {}
	~apex_cheats() {}

	/* ��ʼ�� */
	bool initialize()
	{
		// ��ʼ���������
		bool state = m_driver.initialize(L"r5apex.exe", L"\\\\.\\{BE416943-E578-4EBA-81F7-597AE57EB483}");
		if (state == false) return false;

		// ������Ϸ����
		m_hwnd = FindWindowA("Respawn001", "Apex Legends");
		if (m_hwnd == 0) return false;

		IMAGE_DOS_HEADER dos = m_driver.read<IMAGE_DOS_HEADER>(m_driver.m_base);
		std::cout << "[+] DOS�ļ�ͷƫ�� : 0x" << std::hex << dos.e_lfanew << std::endl;

		IMAGE_NT_HEADERS64 nt64 = m_driver.read<IMAGE_NT_HEADERS64>(m_driver.m_base + dos.e_lfanew);
		std::cout << "[+] ʱ��� : 0x" << std::hex << nt64.FileHeader.TimeDateStamp << std::endl;
		if (apex_offsets::TimeDateStamp != nt64.FileHeader.TimeDateStamp)
		{
			std::cout << "[-] ʱ�������ͬ,�����ƫ��" << std::endl;
			system("pause");
			exit(0);
			return false;
		}

		std::cout << "[+] У��� : 0x" << std::hex << nt64.OptionalHeader.CheckSum << std::endl;
		if (apex_offsets::CheckSum != nt64.OptionalHeader.CheckSum)
		{
			std::cout << "[-] У��Ͳ���ͬ,�����ƫ��" << std::endl;
			system("pause");
			exit(0);
			return false;
		}

		return true;
	}

	/* ��Ϸ�����˳��ж� */
	bool is_game_exit()
	{
		return FindWindowA("Respawn001", "Apex Legends") == NULL;
	}

	/* ��ȡ��Ч��� */
	int get_visiable_player()
	{
		// �������
		int num = 0;

		// ������
		for (int i = 0; i < MAX_PLAYERS; i++) m_players[i].update(nullptr, 0);

		// �ҳ���������ҵ�ַ
		for (int i = 0; i < NUM_ENT_ENTRIES; i++)
		{
			// ���õ�ʵ����ַ
			DWORD64 addr = g_clients[i].pEntity;

			// ��ַΪ��
			if (addr == 0) continue;

			// ��ַ����
			if ((addr & 0x07) != 0 || addr >= (1ULL << 48)) continue;

			// ������Լ�
			if (addr == m_local.m_base) continue;

			entity e(&m_driver, addr);

			// �������һ��ߵ�����
			if (e.is_player() || e.is_npc())
			{
				// û������
				if (e.get_current_health() > 0 && e.is_life())
				{
					// ��������б�
					m_players[num++] = e;
					continue;
				}
			}

			// ����Թ�
			switch (e.get_item_index())
			{
				// ������
			case ItemID::R301:
			case ItemID::R301_GOLD:
			case ItemID::R99:
			case ItemID::R99_GOLD:

				// ͷ��
			case ItemID::HELMET_LV3:
			case ItemID::HELMET_LV4:

				// ����
			case ItemID::BODY_ARMOR_LV3:
			case ItemID::BODY_ARMOR_LV4:

				// ����
			case ItemID::EVO_SHIELD_LV3:
			case ItemID::EVO_SHIELD_LV4:

				// ��������
			case ItemID::KNOCKDOWN_SHIELD_LV3:
			case ItemID::KNOCKDOWN_SHIELD_LV4:

				// ����
			case ItemID::BACKPACK_LV3:
			case ItemID::BACKPACK_LV4:

				e.glow_item(true);
				break;
			}
		}

		return num;
	}

	/* ����ר�� */
	void this_is_test_function()
	{
		// �ҵ�ַ
		// CPlayer
		// CAI_BaseNPC
		// CPropSurvival
		// CWeaponX
		// CWorld
		for (int i = 0; i < NUM_ENT_ENTRIES; i++)
		{
			// ���õ�ʵ����ַ
			DWORD64 addr = g_clients[i].pEntity;
			if (addr == 0) continue;

			// �����R301�����ͻԹ�
			entity e(&m_driver, addr);
			switch (e.get_item_index())
			{
				// ������
			case ItemID::R301:
			case ItemID::R301_GOLD:
			case ItemID::R99:
			case ItemID::R99_GOLD: e.glow_item(true); break;

				// ͷ��
			case ItemID::HELMET_LV3:
			case ItemID::HELMET_LV4: e.glow_item(true); break;

				// ����
			case ItemID::BODY_ARMOR_LV3:
			case ItemID::BODY_ARMOR_LV4: e.glow_item(true); break;

				// ����
			case ItemID::EVO_SHIELD_LV3:
			case ItemID::EVO_SHIELD_LV4: e.glow_item(true); break;

				// ��������
			case ItemID::KNOCKDOWN_SHIELD_LV3:
			case ItemID::KNOCKDOWN_SHIELD_LV4: e.glow_item(true); break;

				// ����
			case ItemID::BACKPACK_LV3:
			case ItemID::BACKPACK_LV4: e.glow_item(true); break;
			}
		}
	}

	/* �Ƕȼ��� */
	void calc_angle(Vec3& vecOrigin, Vec3& vecOther, Vec3& vecAngles)
	{
		Vec3 vecDelta = Vec3{ (vecOrigin[0] - vecOther[0]), (vecOrigin[1] - vecOther[1]), (vecOrigin[2] - vecOther[2]) };
		float hyp = sqrtf(vecDelta[0] * vecDelta[0] + vecDelta[1] * vecDelta[1]);

		vecAngles[0] = (float)atan(vecDelta[2] / hyp)		*(float)(180.f / 3.14159265358979323846);
		vecAngles[1] = (float)atan(vecDelta[1] / vecDelta[0])	*(float)(180.f / 3.14159265358979323846);
		vecAngles[2] = (float)0.f;

		if (vecDelta[0] >= 0.f) vecAngles[1] += 180.0f;
	}

	/* ����һ������ */
	void make_vector(Vec3& vecAngle, Vec3& out)
	{
		float pitch = float(vecAngle[0] * 3.14159265358979323846 / 180);
		float tmp = float(cos(pitch));
		float yaw = float(vecAngle[1] * 3.14159265358979323846 / 180);
		out[0] = float(-tmp * -cos(yaw));
		out[1] = float(sin(yaw)*tmp);
		out[2] = float(-sin(pitch));
	}

	/* ��ȡ����fov */
	float get_max_fov(Vec3& vecAngle, Vec3& vecOrigin, Vec3& vecOther)
	{
		Vec3 ang, aim;
		double fov = 0.0;

		calc_angle(vecOrigin, vecOther, ang);
		make_vector(vecAngle, aim);
		make_vector(ang, ang);

		float mag_s = sqrt((aim[0] * aim[0]) + (aim[1] * aim[1]) + (aim[2] * aim[2]));
		float mag_d = sqrt((aim[0] * aim[0]) + (aim[1] * aim[1]) + (aim[2] * aim[2]));

		float u_dot_v = aim[0] * ang[0] + aim[1] * ang[1] + aim[2] * ang[2];
		fov = acos(u_dot_v / (mag_s*mag_d)) * (180.f / 3.14159265358979323846f);
		fov *= 1.4;

		if (isnan(fov)) return 0.0f;
		return float(fov);
	}

	/* ��������Ƕ� */
	Vec3 get_aimbot_angle(Vec3& vecOrigin, Vec3& vecOther)
	{
		Vec3 vecAngles{};
		Vec3 vecDelta = Vec3{ (vecOrigin[0] - vecOther[0]), (vecOrigin[1] - vecOther[1]), (vecOrigin[2] - vecOther[2]) };
		float hyp = sqrtf(vecDelta[0] * vecDelta[0] + vecDelta[1] * vecDelta[1]);

		float  M_PI = 3.1415f;
		vecAngles[0] = (float)atan(vecDelta[2] / hyp) * (float)(180.f / M_PI);
		vecAngles[1] = (float)atan(vecDelta[1] / vecDelta[0]) * (float)(180.f / M_PI);
		vecAngles[2] = (float)0.f;

		if (vecDelta[0] >= 0.f) vecAngles[1] += 180.0f;
		return vecAngles;
	}

	/* �Ƕȹ�һ�� */
	void angle_normalize(Vec3& vAngles)
	{
		for (int i = 0; i < 3; i++)
		{
			if (vAngles[i] < -180.0f) vAngles[i] += 360.0f;
			if (vAngles[i] > 180.0f) vAngles[i] -= 360.0f;
		}

		if (vAngles.x < -89.0f) vAngles.x = 89.0f;
		if (vAngles.x > 89.0f) vAngles.x = 89.0f;

		vAngles.z = 0;
	}

	/* �淶�Ƕ� */
	void clamp_angles(Vec3& vAngles)
	{
		while (vAngles.y < -180.0f) vAngles.y += 360.0f;
		while (vAngles.y > 180.0f) vAngles.y -= 360.0f;

		if (vAngles.x < -89.0f) vAngles.x = 89.0f;
		if (vAngles.x > 89.0f) vAngles.x = 89.0f;

		vAngles.z = 0;
	}

	/* ��Ϣ�ĸ��� */
	void info_update()
	{
		// ��ȡ�Լ���ַ
		DWORD64 addr = m_driver.read<DWORD64>(m_driver.m_base + apex_offsets::LocalPlayer);
		m_local.update(&m_driver, addr);

		// ��ȡȫ���ͻ�����Ϣ
		m_driver.read_array(m_driver.m_base + apex_offsets::cl_entitylist, g_clients, sizeof(client_info) * NUM_ENT_ENTRIES);

		// �������
		int num = get_visiable_player();
		std::cout << std::oct << "[+] ʣ����� [ " << num << " ] ��" << std::endl;

		// ����
		// this_is_test_function();
	}

	/* ��һԹ� */
	void glow_player(bool state = true)
	{
		// �Լ���ַΪ��
		if (m_local.empty()) return;

		// �������
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			// ���ж�
			if (m_players[i].empty()) break;

			// ��Ҵ���ж�
			if (m_players[i].get_current_health() <= 0) continue;
			if (m_players[i].is_life() == false) continue;

			// ����Ƕ���
			if (m_players[i].get_team_id() == m_local.get_team_id()) continue;

			// ��һԹ�
			m_players[i].glow_player(state);
		}
	}

	/* ������� */
	void aim_player(int bone = 8)
	{
		// �Լ���ַΪ��
		if (m_local.empty()) return;

		// �������
		if (m_local.get_current_health() <= 0) return;

		// ��ȡͷ������
		Vec3 local_head = m_local.get_bone_position1(bone);

		// ��ȡ��ǰ�Ƕ�
		Vec3 current_angle = m_local.get_angle();

		// ��ȡ�������Ƕ�
		Vec3 recoil_angle = m_local.get_recoil_angle();

		Vec3 vest;
		float max_fov = 20.0f;
		bool state = false;

		// �������
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			// �����ȫ��Ϊ��
			if (m_players[i].empty()) break;

			// �������
			if (m_players[i].get_current_health() <= 0) continue;
			if (m_players[i].is_life() == false) continue;

			// ��ҵ���
			if (m_players[i].is_bleed_out()) continue;

			// ����Ƕ���
			if (m_players[i].get_team_id() == m_local.get_team_id()) continue;

			// ��ȡ��ҹ���
			Vec3 v = m_players[i].get_bone_position1(bone);

			// ���볬��3000�Ͳ�Ҫ������
			float dis = local_head.distance(v);
			if (dis > 3000.0f) continue;

			// �ҵ���������׼���������һ������
			float f = get_max_fov(current_angle, local_head, v);
			if (f < max_fov)
			{
				max_fov = f;
				vest = v;
				state = true;
			}
		}

		if (state)
		{
			// ��������Ƕ�
			Vec3 angle = get_aimbot_angle(local_head, vest) - recoil_angle;
			angle.x += 0.5f;

			// ��һ���Ƕ�
			angle_normalize(angle);
			clamp_angles(angle);

			// ���ýǶ�
			m_local.set_angle(angle);
		}
	}

	/* ��ʼ���� */
	void start_cheats()
	{
		// ��ʼ��
		if (initialize() == false) return;

		// ׼����,������ʾ��
		Beep(500, 500);

		// ״̬����
		info_update();

		// ����ѭ��
		while (is_game_exit() == false)
		{
			// ��Ϸ�����Ƕ��㴰��
			if (m_hwnd == GetForegroundWindow())
			{
				// ��Ծ��������Ϣ��������һԹ�
				// ��ΪAPEX��64λ����Ϸ,һֱ״̬���µĻ�,�����ٶȸ����ϰ�
				// ����Ҫ�����ڰ�һ����Ծ��!!!!!��������б�
				if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				{
					// ״̬����
					info_update();

					// ��һԹ�
					glow_player();
				}

				// �������
				if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) aim_player(8);//6 8

			   // �˳�����
				if (GetAsyncKeyState(VK_F9) & 0x8000) break;
			}

			// �Ź�CPU
			Sleep(3);
		}
	}
};