#pragma once
#include <Windows.h>

/*
APEXƫ��
*/

namespace apex_offsets
{
	static DWORD64 TimeDateStamp = 0x5f6d432f;					// ʱ���
	static DWORD64 CheckSum = 0x1fc1a53;								// У���

	static DWORD64 ClientState = 0x1261f80;								// �ͻ���״̬
	static DWORD64 SignonState = 0x1262018;							// ��½״̬
	static DWORD64 LevelName = 0x1262130;							// ��Ϸ�汾

	static DWORD64 ViewRender = 0x40d5d98;							// ��Ⱦ��ͼ
	static DWORD64 ViewMatrix = 0x1b3bd0;								// ������ͼ
	static DWORD64 GlobalVars = 0x1261c80;							// ȫ�ֱ���

	static DWORD64 LocalPlayer = 0x1c5bcc8;							// �������
	static DWORD64 cl_entitylist = 0x18ad3a8;							// ����б�

	static DWORD64 m_ModelName = 0x0030;							// ģ������
	static DWORD64 m_fFlags = 0x0098;										// ��ʶ
	static DWORD64 m_vecAbsOrigin = 0x014c;							// absԭ��
	static DWORD64 m_iTeamNum = 0x0430;								// �Ŷӱ�ʶ		97��ʱ���ǵ�����
	static DWORD64 m_iName = 0x0561;									// ���Ʊ�ʶ		125780153691248��ʱ�������
	static DWORD64 m_vecVelocity = 0x0460;								// �ٶ�
	static DWORD64 m_bConstrainBetweenEndpoints = 0x0f18;//����
	static DWORD64 m_localOrigin = 0x0158;								// ����ԭ��
	static DWORD64 m_localAngles = 0x0164;							// ���ؽǶ�
	static DWORD64 m_vecPunchWeapon_Angle = 0x23c8;		// ������

	static DWORD64 m_shieldHealth = 0x0170;							// ����ֵ
	static DWORD64 m_shieldHealthMax = 0x0174;					// ��󻤶�ֵ
	static DWORD64 m_iHealth = 0x0420;									// Ѫ��
	static DWORD64 m_iMaxHealth = 0x0550;							// ���Ѫ��
	static DWORD64 m_lifeState = 0x0770;									// ���״̬
	static DWORD64 m_latestPrimaryWeapons = 0x1a0c;			// ����
	static DWORD64 m_iObserverMode = 0x32bc;						// �۲�ģʽ
	static DWORD64 m_helmetType = 0x4274;							// ͷ������
	static DWORD64 m_armorType = 0x4278;								// ��������
	static DWORD64 m_bleedoutState = 0x2610;						// ��Ѫ״̬
	static DWORD64 m_bleedoutStartTime = 0x2614;					// ��Ѫʱ��

	static DWORD64 m_highlightFunctionBits = 0x2A8;
	static DWORD64 m_highlight_t1 = m_highlightFunctionBits - 0x46;
	static DWORD64 m_highlight_t2 = m_highlightFunctionBits + 0x1C;
	static DWORD64 m_highlight_enable = m_highlightFunctionBits + 0xA8;
	static DWORD64 m_highlight_wall = m_highlight_enable + 0x10;

	static DWORD64 m_highlightParams = 0x01b8;
	static DWORD64 m_highlightServerFadeStartTimes = 0x0300;
	static DWORD64 m_highlightServerContextID = 0x0348;
	static DWORD64 m_highlightTeamBits = 0x0354;

	static DWORD64 m_weaponOwner = 0x1600;						// ����
	static DWORD64 m_weaponNameIndex = 0x17b0;				// ��������
	static DWORD64 m_flProjectileSpeed = 0x1e0c;					//
	static DWORD64 m_flProjectileScale = 0x1e14;						//
};