#pragma once

#pragma pack(push, 1)
struct CSteamNetworkConnectionBase
{
private:
	/* 0x0000 */ char pad_0x0[0x30];
public:
	/* 0x0030 */ __int64 m_hConnectionSelf;
private:
	/* 0x0038 */ char pad_0x38[0x4];
public:
	/* 0x003C */ __int64 m_identityRemote;
private:
	/* 0x0044 */ char pad_0x44[0x80];
public:
	/* 0x00C4 */ __int64 m_identityLocal;
}; // Size: 0xCC
#pragma pack(pop)

static_assert(offsetof(CSteamNetworkConnectionBase, CSteamNetworkConnectionBase::m_hConnectionSelf) == 0x30, "CSteamNetworkConnectionBase::m_hConnectionSelf: Incorrect offset");
static_assert(offsetof(CSteamNetworkConnectionBase, CSteamNetworkConnectionBase::m_identityRemote) == 0x3C, "CSteamNetworkConnectionBase::m_identityRemote: Incorrect offset");
static_assert(offsetof(CSteamNetworkConnectionBase, CSteamNetworkConnectionBase::m_identityLocal) == 0xC4, "CSteamNetworkConnectionBase::m_identityLocal: Incorrect offset");

static_assert(sizeof(CSteamNetworkConnectionBase) == 0xCC, "CSteamNetworkConnectionBase: Incorrect Size");