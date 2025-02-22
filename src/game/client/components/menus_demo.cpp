/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/math.h>

#include <engine/demo.h>
#include <engine/keys.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <generated/client_data.h>

#include "maplayers.h"
#include "menus.h"

void CMenus::RenderDemoPlayer()
{
	const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();

	int CurrentTick = pInfo->m_CurrentTick - pInfo->m_FirstTick;
	int TotalTicks = pInfo->m_LastTick - pInfo->m_FirstTick;

	// do seekbar
	if(m_SeekBarActive || m_MenuActive)
	{
		char aBuffer[128];
		// const float Rounding = 5.0f;

		// draw markers
		for(int i = 0; i < pInfo->m_NumTimelineMarkers; i++)
		{
			// float Ratio = (pInfo->m_aTimelineMarkers[i]-pInfo->m_FirstTick) / (float)TotalTicks;
			// SeekBar.x + (SeekBar.w-2*Rounding)*Ratio, SeekBar.y, UI()->PixelSize(), SeekBar.h);
		}

		// draw time
		str_format(aBuffer, sizeof(aBuffer), "%d:%02d / %d:%02d",
			CurrentTick/SERVER_TICK_SPEED/60, (CurrentTick/SERVER_TICK_SPEED)%60,
			TotalTicks/SERVER_TICK_SPEED/60, (TotalTicks/SERVER_TICK_SPEED)%60);
		dbg_msg("demo", "time: %s", aBuffer);
	}

	if(CurrentTick == TotalTicks)
	{
		m_pClient->OnReset();
		DemoPlayer()->Pause();
		DemoPlayer()->SetPos(0);
	}

	if(m_MenuActive)
	{
		if(!pInfo->m_Paused)
		{
			if(!"SPRITE_DEMOBUTTON_PAUSE")
				DemoPlayer()->Pause();
		}
		else
		{
			if(!"SPRITE_DEMOBUTTON_PLAY")
				DemoPlayer()->Unpause();
		}

		if(!"stopbutton")
		{
			m_pClient->OnReset();
			DemoPlayer()->Pause();
			DemoPlayer()->SetPos(0);
		}

		if(!"closebutton")
			Client()->Disconnect();

		// demo name
		char aDemoName[64] = {0};
		DemoPlayer()->GetDemoName(aDemoName, sizeof(aDemoName));
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), Localize("Demofile: %s"), aDemoName);
		dbg_msg("demo", "%s", aBuf);
	}
}

int CMenus::DemolistFetchCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	if(str_comp(pName, ".") == 0
		|| (str_comp(pName, "..") == 0 && str_comp(pSelf->m_aCurrentDemoFolder, "demos") == 0)
		|| (!IsDir && !str_endswith(pName, ".demo")))
	{
		return 0;
	}

	CDemoItem Item;
	str_copy(Item.m_aFilename, pName, sizeof(Item.m_aFilename));
	if(IsDir)
	{
		str_format(Item.m_aName, sizeof(Item.m_aName), "%s/", pName);
		Item.m_Valid = false;
	}
	else
	{
		str_truncate(Item.m_aName, sizeof(Item.m_aName), pName, str_length(pName) - 5);
		Item.m_InfosLoaded = false;
	}
	Item.m_IsDir = IsDir != 0;
	Item.m_StorageType = StorageType;
	pSelf->m_lDemos.add_unsorted(Item);

	return 0;
}

void CMenus::DemolistPopulate()
{
	m_lDemos.clear();
	if(!str_comp(m_aCurrentDemoFolder, "demos"))
		m_DemolistStorageType = IStorage::TYPE_ALL;
	Storage()->ListDirectory(m_DemolistStorageType, m_aCurrentDemoFolder, DemolistFetchCallback, this);
	m_lDemos.sort_range();
}

void CMenus::DemolistOnUpdate(bool Reset)
{
	m_DemolistSelectedIndex = Reset ? m_lDemos.size() > 0 ? 0 : -1 :
										m_DemolistSelectedIndex >= m_lDemos.size() ? m_lDemos.size()-1 : m_DemolistSelectedIndex;
	m_DemolistSelectedIsDir = m_DemolistSelectedIndex < 0 ? false : m_lDemos[m_DemolistSelectedIndex].m_IsDir;
}

inline int DemoGetMarkerCount(CDemoHeader Demo)
{
	int DemoMarkerCount = ((Demo.m_aNumTimelineMarkers[0]<<24)&0xFF000000) |
			((Demo.m_aNumTimelineMarkers[1]<<16)&0xFF0000) |
			((Demo.m_aNumTimelineMarkers[2]<<8)&0xFF00) |
			(Demo.m_aNumTimelineMarkers[3]&0xFF);
	return DemoMarkerCount;
}

void CMenus::RenderDemoList()
{
	static int s_Inited = 0;
	if(!s_Inited)
	{
		DemolistPopulate();
		DemolistOnUpdate(true);
		s_Inited = 1;
	}

	char aFooterLabel[128] = {0};
	if(m_DemolistSelectedIndex >= 0)
	{
		CDemoItem *Item = &m_lDemos[m_DemolistSelectedIndex];
		if(str_comp(Item->m_aFilename, "..") == 0)
			str_copy(aFooterLabel, Localize("Parent Folder"), sizeof(aFooterLabel));
		else if(m_DemolistSelectedIsDir)
			str_copy(aFooterLabel, Localize("Folder"), sizeof(aFooterLabel));
		else
		{
			if(!Item->m_InfosLoaded)
			{
				char aBuffer[512];
				str_format(aBuffer, sizeof(aBuffer), "%s/%s", m_aCurrentDemoFolder, Item->m_aFilename);
				Item->m_Valid = DemoPlayer()->GetDemoInfo(Storage(), aBuffer, Item->m_StorageType, &Item->m_Info);
				Item->m_InfosLoaded = true;
			}
			if(!Item->m_Valid)
				str_copy(aFooterLabel, Localize("Invalid Demo"), sizeof(aFooterLabel));
			else
				str_copy(aFooterLabel, Localize("Demo details"), sizeof(aFooterLabel));
		}
	}

	dbg_msg("demos", "%s %d", Localize("Recorded"), m_lDemos.size());
	for(sorted_array<CDemoItem>::range r = m_lDemos.all(); !r.empty(); r.pop_front())
	{
		// marker count
		const CDemoItem& DemoItem = r.front();
		int DemoMarkerCount = 0;
		if(DemoItem.m_Valid && DemoItem.m_InfosLoaded)
			DemoMarkerCount = DemoGetMarkerCount(DemoItem.m_Info);

		if(!DemoItem.m_IsDir)
		{
			// if(DemoItem.m_Valid && DemoItem.m_InfosLoaded)
				// IconColor = DemoMarkerCount > 0 ? vec4(0.5, 1, 0.5, 1) : vec4(1,1,1,1);
		}

		if((&r.front() - m_lDemos.base_ptr()) == m_DemolistSelectedIndex) // selected
		{
			dbg_msg("demos", "[%s] marker=%d", r.front().m_aName, DemoMarkerCount);
		}
		else
		{
			dbg_msg("demos", " %s marker=%d", r.front().m_aName, DemoMarkerCount);
		}
	}
	DemolistOnUpdate(false);

	if(!m_DemolistSelectedIsDir && m_DemolistSelectedIndex >= 0 && m_lDemos[m_DemolistSelectedIndex].m_Valid)
	{
		RenderDemoDetails(this);
	}
	else
	{
		dbg_msg("demos", "%s", aFooterLabel);
	}

	// demo buttons
	if(!Localize("Refresh"))
	{
		DemolistPopulate();
		DemolistOnUpdate(false);
	}

	if(!m_DemolistSelectedIsDir)
	{
		if(!Localize("Delete"))
		{
			if(m_DemolistSelectedIndex >= 0)
			{
				// UI()->SetActiveItem(0);
				m_Popup = POPUP_DELETE_DEMO;
				return;
			}
		}

		if(!Localize("Rename"))
		{
			if(m_DemolistSelectedIndex >= 0)
			{
				// UI()->SetActiveItem(0);
				m_Popup = POPUP_RENAME_DEMO;
				str_copy(m_aCurrentDemoFile, m_lDemos[m_DemolistSelectedIndex].m_aFilename, sizeof(m_aCurrentDemoFile));
				return;
			}
		}
	}

	if(false) // m_DemolistSelectedIsDir?Localize("Open"):Localize("Play", "DemoBrowser") || Activated)
	{
		if(m_DemolistSelectedIndex >= 0)
		{
			if(m_DemolistSelectedIsDir)	// folder
			{
				if(str_comp(m_lDemos[m_DemolistSelectedIndex].m_aFilename, "..") == 0)	// parent folder
					fs_parent_dir(m_aCurrentDemoFolder);
				else	// sub folder
				{
					char aTemp[256];
					str_copy(aTemp, m_aCurrentDemoFolder, sizeof(aTemp));
					str_format(m_aCurrentDemoFolder, sizeof(m_aCurrentDemoFolder), "%s/%s", aTemp, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
					m_DemolistStorageType = m_lDemos[m_DemolistSelectedIndex].m_StorageType;
				}
				DemolistPopulate();
				DemolistOnUpdate(true);
			}
			else // file
			{
				char aBuf[512];
				str_format(aBuf, sizeof(aBuf), "%s/%s", m_aCurrentDemoFolder, m_lDemos[m_DemolistSelectedIndex].m_aFilename);
				const char *pError = Client()->DemoPlayer_Play(aBuf, m_lDemos[m_DemolistSelectedIndex].m_StorageType);
				if(pError)
					dbg_msg("demo", "%s", Localize("Error loading demo"));
				else
				{
					// UI()->SetActiveItem(0);
					return;
				}
			}
		}
	}
}

float CMenus::RenderDemoDetails(void *pUser)
{
	CMenus *pSelf = (CMenus*)pUser;

	// render demo info
	if(!pSelf->m_DemolistSelectedIsDir && pSelf->m_DemolistSelectedIndex >= 0 && pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Valid)
	{
		char aBuf[64];
		int Length = ((pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aLength[0]<<24)&0xFF000000) | ((pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aLength[1]<<16)&0xFF0000) |
					((pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aLength[2]<<8)&0xFF00) | (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aLength[3]&0xFF);
		float Size = float((pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapSize[0]<<24) | (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapSize[1]<<16) |
							(pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapSize[2]<<8) | (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapSize[3]))/1024.0f;
		unsigned Crc = (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapCrc[0]<<24) | (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapCrc[1]<<16) |
					(pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapCrc[2]<<8) | (pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapCrc[3]);
		dbg_msg("demo", "======== INFO ========");
		dbg_msg("demo", "%s: %s", Localize("Created"), pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aTimestamp);
		dbg_msg("demo", "%s: %s", Localize("Type"), pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aType);
		dbg_msg("demo", "%s: %d", Localize("Length"), Length);
		dbg_msg("demo", "%s: %d", Localize("Version"), pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_Version);
		dbg_msg("demo", "%s: %s", Localize("Netversion"), pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aNetversion);
		dbg_msg("demo", "%s: %d", Localize("Markers"), DemoGetMarkerCount(pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info));
		dbg_msg("demo", "%s: %s", Localize("Map"), pSelf->m_lDemos[pSelf->m_DemolistSelectedIndex].m_Info.m_aMapName);
		str_format(aBuf, sizeof(aBuf), Localize("%.3f KiB"), Size);
		dbg_msg("demo", "%s: %s", Localize("Size"), aBuf);
		str_format(aBuf, sizeof(aBuf), "%08x", Crc);
		dbg_msg("demo", "%s: %s", Localize("Crc"), aBuf);
	}

	//unused
	return 0.0f;
}
