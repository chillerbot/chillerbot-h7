/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm> // sort  TODO: remove this

#include <engine/external/json-parser/json.h>

#include <engine/config.h>
#include <engine/friends.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/version.h>
#include <game/client/components/countryflags.h>

#include "menus.h"

CMenus::CColumn CMenus::ms_aBrowserCols[] = {  // Localize("Server"); Localize("Type"); Localize("Map"); Localize("Players"); Localize("Ping"); - these strings are localized within CLocConstString
	{COL_BROWSER_FLAG,		-1,									" ",		-1, 4*16.0f+3*2.0f, 0},
	{COL_BROWSER_NAME,		IServerBrowser::SORT_NAME,			"Server",	0, 310.0f, 0},
	{COL_BROWSER_GAMETYPE,	IServerBrowser::SORT_GAMETYPE,		"Type",		1, 70.0f,  0},
	{COL_BROWSER_MAP,		IServerBrowser::SORT_MAP,			"Map",		1, 100.0f, 0},
	{COL_BROWSER_PLAYERS,	IServerBrowser::SORT_NUMPLAYERS,	"Players",	1, 50.0f,  0},
	{COL_BROWSER_PING,		IServerBrowser::SORT_PING,			"Ping",		1, 40.0f,  0},
};

CServerFilterInfo CMenus::CBrowserFilter::ms_FilterStandard = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_PURE|IServerBrowser::FILTER_PURE_MAP, 999, -1, 0, {{0}}, {0}};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterFavorites = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_FAVORITE, 999, -1, 0, {{0}}, {0}};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterAll = {IServerBrowser::FILTER_COMPAT_VERSION, 999, -1, 0, {{0}}, {0}};


// filters
CMenus::CBrowserFilter::CBrowserFilter(int Custom, const char* pName, IServerBrowser *pServerBrowser)
{
	m_Extended = false;
	m_Custom = Custom;
	str_copy(m_aName, pName, sizeof(m_aName));
	m_pServerBrowser = pServerBrowser;
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterFavorites);
		break;
	default:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterAll);
	}

	// init buttons
	m_SwitchButton = 0;
}

void CMenus::CBrowserFilter::Reset()
{
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterFavorites);
		break;
	default:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterAll);
	}
}

void CMenus::CBrowserFilter::Switch()
{
	m_Extended ^= 1;
}

bool CMenus::CBrowserFilter::Extended() const
{
	return m_Extended;
}

int CMenus::CBrowserFilter::Custom() const
{
	return m_Custom;
}

int CMenus::CBrowserFilter::Filter() const
{
	return m_Filter;
}

const char* CMenus::CBrowserFilter::Name() const
{
	return m_aName;
}

const void *CMenus::CBrowserFilter::ID(int Index) const
{
	return m_pServerBrowser->GetID(m_Filter, Index);
}

int CMenus::CBrowserFilter::NumSortedServers() const
{
	return m_pServerBrowser->NumSortedServers(m_Filter);
}

int CMenus::CBrowserFilter::NumPlayers() const
{
	return m_pServerBrowser->NumSortedPlayers(m_Filter);
}

const CServerInfo *CMenus::CBrowserFilter::SortedGet(int Index) const
{
	if(Index < 0 || Index >= m_pServerBrowser->NumSortedServers(m_Filter))
		return 0;
	return m_pServerBrowser->SortedGet(m_Filter, Index);
}

void CMenus::CBrowserFilter::SetFilterNum(int Num)
{
	m_Filter = Num;
}

void CMenus::CBrowserFilter::GetFilter(CServerFilterInfo *pFilterInfo) const
{
	m_pServerBrowser->GetFilter(m_Filter, pFilterInfo);
}

void CMenus::CBrowserFilter::SetFilter(const CServerFilterInfo *pFilterInfo)
{
	m_pServerBrowser->SetFilter(m_Filter, pFilterInfo);
}

void CMenus::LoadFilters()
{
	// read file data into buffer
	const char *pFilename = "ui_settings.json";
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
		return;
	int FileSize = (int)io_length(File);
	char *pFileData = (char *)mem_alloc(FileSize, 1);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);

	if(pJsonData == 0)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, pFilename, aError);
		return;
	}

	// extract settings data
	const json_value &rSettingsEntry = (*pJsonData)["settings"];
	if(rSettingsEntry["sidebar_active"].type == json_integer)
		m_SidebarActive = rSettingsEntry["sidebar_active"].u.integer;
	if(rSettingsEntry["sidebar_tab"].type == json_integer)
		m_SidebarTab = clamp(int(rSettingsEntry["sidebar_tab"].u.integer), 0, 2);

	// extract filter data
	int Extended = 0;
	const json_value &rFilterEntry = (*pJsonData)["filter"];
	for(unsigned i = 0; i < rFilterEntry.u.array.length; ++i)
	{
		char *pName = rFilterEntry[i].u.object.values[0].name;
		const json_value &rStart = *(rFilterEntry[i].u.object.values[0].value);
		if(rStart.type != json_object)
			continue;

		int Type = 0;
		if(rStart["type"].type == json_integer)
			Type = rStart["type"].u.integer;
		if(rStart["extended"].type == json_integer && rStart["extended"].u.integer)
			Extended = i;

		// filter setting
		CServerFilterInfo FilterInfo;
		for(int j = 0; j < CServerFilterInfo::MAX_GAMETYPES; ++j)
			FilterInfo.m_aGametype[j][0] = 0;
		const json_value &rSubStart = rStart["settings"];
		if(rSubStart.type == json_object)
		{
			if(rSubStart["filter_hash"].type == json_integer)
				FilterInfo.m_SortHash = rSubStart["filter_hash"].u.integer;
			const json_value &rGametypeEntry = rSubStart["filter_gametype"];
			if(rGametypeEntry.type == json_array)
			{
				for(unsigned j = 0; j < rGametypeEntry.u.array.length && j < CServerFilterInfo::MAX_GAMETYPES; ++j)
				{
					if(rGametypeEntry[j].type == json_string)
						str_copy(FilterInfo.m_aGametype[j], rGametypeEntry[j], sizeof(FilterInfo.m_aGametype[j]));
				}
			}
			if(rSubStart["filter_ping"].type == json_integer)
				FilterInfo.m_Ping = rSubStart["filter_ping"].u.integer;
			if(rSubStart["filter_serverlevel"].type == json_integer)
				FilterInfo.m_ServerLevel = rSubStart["filter_serverlevel"].u.integer;
			if(rSubStart["filter_address"].type == json_string)
				str_copy(FilterInfo.m_aAddress, rSubStart["filter_address"], sizeof(FilterInfo.m_aAddress));
			if(rSubStart["filter_country"].type == json_integer)
				FilterInfo.m_Country = rSubStart["filter_country"].u.integer;
		}

		// add filter
		m_lFilters.add(CBrowserFilter(Type, pName, ServerBrowser()));
		if(Type == CBrowserFilter::FILTER_STANDARD)		//	make sure the pure filter is enabled in the Teeworlds-filter
			FilterInfo.m_SortHash |= IServerBrowser::FILTER_PURE;
		m_lFilters[i].SetFilter(&FilterInfo);
	}

	// clean up
	json_value_free(pJsonData);

	m_lFilters[Extended].Switch();
}

void CMenus::SaveFilters()
{
	IOHANDLE File = Storage()->OpenFile("ui_settings.json", IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;

	char aBuf[512];

	// settings
	const char *p = "{\"settings\": {\n";
	io_write(File, p, str_length(p));

	str_format(aBuf, sizeof(aBuf), "\t\"sidebar_active\": %d,\n", m_SidebarActive);
	io_write(File, aBuf, str_length(aBuf));
	str_format(aBuf, sizeof(aBuf), "\t\"sidebar_tab\": %d,\n", m_SidebarTab);
	io_write(File, aBuf, str_length(aBuf));

	// settings end
	p = "\t},\n";
	io_write(File, p, str_length(p));
	
	// filter
	p = " \"filter\": [";
	io_write(File, p, str_length(p));

	for(int i = 0; i < m_lFilters.size(); i++)
	{
		// part start
		if(i == 0)
			p = "\n";
		else
			p = ",\n";
		io_write(File, p, str_length(p));

		str_format(aBuf, sizeof(aBuf), "\t{\"%s\": {\n", m_lFilters[i].Name());
		io_write(File, aBuf, str_length(aBuf));

		str_format(aBuf, sizeof(aBuf), "\t\t\"type\": %d,\n", m_lFilters[i].Custom());
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\"extended\": %d,\n", m_lFilters[i].Extended()?1:0);
		io_write(File, aBuf, str_length(aBuf));

		// filter setting
		CServerFilterInfo FilterInfo;
		m_lFilters[i].GetFilter(&FilterInfo);

		str_format(aBuf, sizeof(aBuf), "\t\t\"settings\": {\n");
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_hash\": %d,\n", FilterInfo.m_SortHash);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_gametype\": [");
		io_write(File, aBuf, str_length(aBuf));
		for(unsigned j = 0; j < CServerFilterInfo::MAX_GAMETYPES && FilterInfo.m_aGametype[j][0]; ++j)
		{
			str_format(aBuf, sizeof(aBuf), "\n\t\t\t\t\"%s\",", FilterInfo.m_aGametype[j]);
			io_write(File, aBuf, str_length(aBuf));
		}
		p = "\n\t\t\t],\n";
		io_write(File, p, str_length(p));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_ping\": %d,\n", FilterInfo.m_Ping);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_serverlevel\": %d,\n", FilterInfo.m_ServerLevel);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_address\": \"%s\",\n", FilterInfo.m_aAddress);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_country\": %d,\n\t\t\t}", FilterInfo.m_Country);
		io_write(File, aBuf, str_length(aBuf));

		// part end
		p = "\n\t\t}\n\t}";
		io_write(File, p, str_length(p));
	}

	// filter end
	p = "]\n}";
	io_write(File, p, str_length(p));

	io_close(File);
}

void CMenus::RemoveFilter(int FilterIndex)
{
	int Filter = m_lFilters[FilterIndex].Filter();
	ServerBrowser()->RemoveFilter(Filter);
	m_lFilters.remove_index(FilterIndex);

	// update filter indexes
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		CBrowserFilter *pFilter = &m_lFilters[i];
		if(pFilter->Filter() > Filter)
			pFilter->SetFilterNum(pFilter->Filter()-1);
	}
}

void CMenus::Move(bool Up, int Filter)
{
	// move up
	CBrowserFilter Temp = m_lFilters[Filter];
	if(Up)
	{
		if(Filter > 0)
		{
			m_lFilters[Filter] = m_lFilters[Filter-1];
			m_lFilters[Filter-1] = Temp;
		}
	}
	else // move down
	{
		if(Filter < m_lFilters.size()-1)
		{
			m_lFilters[Filter] = m_lFilters[Filter+1];
			m_lFilters[Filter+1] = Temp;
		}
	}
}

void CMenus::SetOverlay(int Type, float x, float y, const void *pData)
{
	if(!m_PopupActive && m_InfoOverlay.m_Reset)
	{
		m_InfoOverlayActive = true;
		m_InfoOverlay.m_Type = Type;
		m_InfoOverlay.m_X = x;
		m_InfoOverlay.m_Y = y;
		m_InfoOverlay.m_pData = pData;
		m_InfoOverlay.m_Reset = false;
	}
}

// 1 = browser entry click, 2 = server info click
int CMenus::DoBrowserEntry(const void *pID, const CServerInfo *pEntry, const CBrowserFilter *pFilter, bool Selected)
{
	return 0;
}

void CMenus::RenderFilterHeader(int FilterIndex)
{

}

void CMenus::RenderServerbrowserOverlay()
{

}

void CMenus::RenderServerbrowserServerList()
{

}

void CMenus::RenderServerbrowserSidebar()
{

}

void CMenus::RenderServerbrowserFriendTab()
{

}

void CMenus::RenderServerbrowserFilterTab()
{

}

void CMenus::RenderServerbrowserInfoTab()
{

}

void CMenus::RenderDetailInfo(const CServerInfo *pInfo)
{

}

void CMenus::RenderDetailScoreboard(const CServerInfo *pInfo, int RowCount, vec4 TextColor)
{

}

void CMenus::RenderServerbrowserServerDetail(const CServerInfo *pInfo)
{

}

void CMenus::FriendlistOnUpdate()
{
	// fill me
}

void CMenus::RenderServerbrowserBottomBox()
{

}

void CMenus::DoGameIcon(const char *pName)
{

}

int CMenus::GameIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	const char *pSuffix = str_endswith(pName, ".png");
	if(IsDir || !pSuffix)
	{
		return 0;
	}
	char aGameIconName[128];
	str_truncate(aGameIconName, sizeof(aGameIconName), pName, pSuffix - pName);

	// add new game icon
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "ui/gametypes/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType) || Info.m_Width != CGameIcon::GAMEICON_SIZE || (Info.m_Height != CGameIcon::GAMEICON_SIZE && Info.m_Height != CGameIcon::GAMEICON_OLDHEIGHT))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load gametype icon '%s'", aGameIconName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}
	CGameIcon GameIcon(aGameIconName);
	str_format(aBuf, sizeof(aBuf), "loaded gametype icon '%s'", aGameIconName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);

	GameIcon.m_IconTexture = pSelf->Graphics()->LoadTextureRaw(CGameIcon::GAMEICON_SIZE, CGameIcon::GAMEICON_SIZE, Info.m_Format, Info.m_pData, Info.m_Format, IGraphics::TEXLOAD_LINEARMIPMAPS);
	pSelf->m_lGameIcons.add(GameIcon);
	if(!str_comp_nocase(aGameIconName, "mod"))
		pSelf->m_GameIconDefault = GameIcon.m_IconTexture;
	return 0;
}

void CMenus::RenderServerbrowser()
{
	/*
		+---------------------------+-------+
		|							|		|
		|							| side-	|
		|		server list			| bar	|
		|							| <->	|
		|---------------------------+-------+
		| back |	   | bottom box |
		+------+       +------------+
	*/
}

void CMenus::ConchainFriendlistUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments() == 2 && ((CMenus *)pUserData)->Client()->State() == IClient::STATE_OFFLINE)
	{
		((CMenus *)pUserData)->FriendlistOnUpdate();
		((CMenus *)pUserData)->Client()->ServerBrowserUpdate();
	}
}

void CMenus::ConchainServerbrowserUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	/*if(pResult->NumArguments() && ((CMenus *)pUserData)->m_MenuPage == PAGE_FAVORITES && ((CMenus *)pUserData)->Client()->State() == IClient::STATE_OFFLINE)
		((CMenus *)pUserData)->ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);*/
}
