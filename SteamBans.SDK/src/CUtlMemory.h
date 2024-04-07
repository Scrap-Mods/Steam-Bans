#pragma once
#include "CSteamNetworkingBase.h"

struct Node_t
{
	__int16 m_Key;
	CSteamNetworkConnectionBase* m_pElement;
	int m_iNode;
};

struct CUtlMemory
{
	/* 0x0000 */ Node_t* m_pData;
private:
	/* 0x0008 */ char pad_0x8[0xC];
public:
	/* 0x0014 */ __int32 m_nCount;
	/* 0x0018 */ __int32 m_nCapacity;
};
