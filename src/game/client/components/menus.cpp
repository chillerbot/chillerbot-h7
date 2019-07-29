/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/friends.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/version.h>
#include <generated/protocol.h>

#include <generated/client_data.h>
#include <game/client/components/binds.h>
#include <game/client/components/camera.h>
#include <game/client/components/console.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <mastersrv/mastersrv.h>

#include "maplayers.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"

float CMenus::ms_ButtonHeight = 25.0f;
float CMenus::ms_ListheaderHeight = 17.0f;
float CMenus::ms_FontmodHeight = 0.8f;


CMenus::CMenus()
{
	m_Popup = POPUP_NONE;
	m_NextPopup = POPUP_NONE;
	m_ActivePage = PAGE_INTERNET;
	m_GamePage = PAGE_GAME;
	m_SidebarTab = 0;
	m_SidebarActive = true;
	m_ShowServerDetails = true;

	m_NeedRestartGraphics = false;
	m_NeedRestartSound = false;
	m_NeedRestartPlayer = false;
	m_TeePartSelected = SKINPART_BODY;
	m_aSaveSkinName[0] = 0;
	m_RefreshSkinSelector = true;
	m_pSelectedSkin = 0;
	m_MenuActive = true;
	m_SeekBarActivatedTime = 0;
	m_SeekBarActive = true;
	m_UseMouseButtons = true;
	m_SkinModified = false;

	SetMenuPage(PAGE_START);
	m_MenuPageOld = PAGE_START;

	m_PopupActive = false;

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_TabPressed = false;
	m_DeletePressed = false;
	m_UpArrowPressed = false;
	m_DownArrowPressed = false;

	m_LastInput = time_get();
	
	m_CursorActive = false;
	m_PrevCursorActive = false;

	str_copy(m_aCurrentDemoFolder, "demos", sizeof(m_aCurrentDemoFolder));
	m_aCallvoteReason[0] = 0;

	m_SelectedFilter = 0;

	m_SelectedServer.m_Filter = -1;
	m_SelectedServer.m_Index = -1;
	m_ActiveListBox = ACTLB_NONE;
}

float CMenus::ButtonFade(CButtonContainer *pBC, float Seconds, int Checked)
{
	return max(0.0f, pBC->m_FadeStartTime -  Client()->LocalTime() + Seconds);
}

int CMenus::DoIcon(int ImageId, int SpriteId)
{
	return 0;
}

void CMenus::DoIconColor(int ImageId, int SpriteId, const vec4& Color)
{

}

int CMenus::DoButton_Toggle(const void *pID, int Checked, bool Active)
{
	return 0;
}

int CMenus::DoButton_Menu(CButtonContainer *pBC, const char *pText, int Checked, const char *pImageName, int Corners, float r, float FontFactor, vec4 ColorHot, bool TextFade)
{
	return 0;
}

void CMenus::DoButton_KeySelect(CButtonContainer *pBC, const char *pText, int Checked)
{

}

int CMenus::DoButton_MenuTab(const void *pID, const char *pText, int Checked, int Corners)
{
	return 0;
}

int CMenus::DoButton_MenuTabTop(CButtonContainer *pBC, const char *pText, int Checked, float Alpha, float FontAlpha, int Corners, float r, float FontFactor)
{
	return 0;
}

void CMenus::DoButton_MenuTabTop_Dummy(const char *pText, int Checked, float Alpha)
{

}

int CMenus::DoButton_GridHeader(const void *pID, const char *pText, int Checked, CUI::EAlignment Align)
{
	return 0;
}

int CMenus::DoButton_CheckBox_Common(const void *pID, const char *pText, const char *pBoxText, bool Checked, bool Locked)
{
	return 0;
}

int CMenus::DoButton_CheckBox(const void *pID, const char *pText, int Checked, bool Locked)
{
	return 0;
}

int CMenus::DoButton_CheckBox_Number(const void *pID, const char *pText, int Checked)
{
	return 0;
}

int CMenus::DoButton_SpriteID(CButtonContainer *pBC, int ImageID, int SpriteID, bool Checked, int Corners, float r, bool Fade)
{
	return 0;
}

int CMenus::DoButton_SpriteClean(int ImageID, int SpriteID)
{
	return 0;
}

int CMenus::DoButton_SpriteCleanID(const void *pID, int ImageID, int SpriteID, bool Blend)
{
	return 0;
}

int CMenus::DoButton_MouseOver(int ImageID, int SpriteID)
{
	return 0;
}

int CMenus::DoEditBox(void *pID, char *pStr, unsigned StrSize, float FontSize, float *pOffset, bool Hidden, int Corners)
{
	return 0;
}

void CMenus::DoEditBoxOption(void *pID, char *pOption, int OptionLength, const char *pStr,  float VSplitVal, float *pOffset, bool Hidden)
{

}

void CMenus::DoScrollbarOption(void *pID, int *pOption, const char *pStr, int Min, int Max, bool Infinite)
{

}

float CMenus::DoDropdownMenu(void *pID, const char *pStr, float HeaderHeight, FDropdownCallback pfnCallback)
{
	return 0;
}

float CMenus::DoIndependentDropdownMenu(void *pID, const char *pStr, float HeaderHeight, FDropdownCallback pfnCallback, bool* pActive)
{
	return 0;
}

void CMenus::DoInfoBox(const char *pLabel, const char *pValue)
{

}

float CMenus::DoScrollbarV(const void *pID, float Current)
{
	return 0.0f;
}

float CMenus::DoScrollbarH(const void *pID, float Current)
{
	return 0.0f;
}

void CMenus::UiDoListboxHeader(CListBoxState* pState, const char *pTitle,
							   float HeaderHeight, float Spacing)
{

}

void CMenus::UiDoListboxStart(CListBoxState* pState, const void *pID, float RowHeight,
							  const char *pBottomText, int NumItems, int ItemsPerRow, int SelectedIndex,
							  bool Background)
{

}

int CMenus::UiDoListboxEnd(CListBoxState* pState, bool *pItemActivated)
{
	return 0;
}

int CMenus::DoKeyReader(CButtonContainer *pBC, int Key, int Modifier, int* NewModifier)
{
	return 0;
}

void CMenus::RenderMenubar()
{

}

void CMenus::RenderLoading()
{

}

void CMenus::RenderNews()
{

}

void CMenus::RenderBackButton()
{

}

int CMenus::MenuImageScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	if(IsDir || !str_endswith(pName, ".png"))
		return 0;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "skipping load of image from %s", pName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
}

const CMenus::CMenuImage *CMenus::FindMenuImage(const char *pName)
{
	for(int i = 0; i < m_lMenuImages.size(); i++)
	{
		if(str_comp(m_lMenuImages[i].m_aName, pName) == 0)
			return &m_lMenuImages[i];
	}
	return 0;
}

void CMenus::UpdateVideoFormats()
{
	m_NumVideoFormats = 0;
	for(int i = 0; i < m_NumModes; i++)
	{
		int G = gcd(m_aModes[i].m_Width, m_aModes[i].m_Height);
		int Width = m_aModes[i].m_Width/G;
		int Height = m_aModes[i].m_Height/G;

		// check if we already have the format
		bool Found = false;
		for(int j = 0; j < m_NumVideoFormats; j++)
		{
			if(Width == m_aVideoFormats[j].m_WidthValue && Height == m_aVideoFormats[j].m_HeightValue)
			{
				Found = true;
				break;
			}
		}

		if(!Found)
		{
			m_aVideoFormats[m_NumVideoFormats].m_WidthValue = Width;
			m_aVideoFormats[m_NumVideoFormats].m_HeightValue = Height;
			m_NumVideoFormats++;

			// sort the array
			for(int k = 0; k < m_NumVideoFormats-1; k++) // ffs, bubblesort
			{
				for(int j = 0; j < m_NumVideoFormats-k-1; j++)
				{
					if((float)m_aVideoFormats[j].m_WidthValue/(float)m_aVideoFormats[j].m_HeightValue > (float)m_aVideoFormats[j+1].m_WidthValue/(float)m_aVideoFormats[j+1].m_HeightValue)
					{
						CVideoFormat Tmp = m_aVideoFormats[j];
						m_aVideoFormats[j] = m_aVideoFormats[j+1];
						m_aVideoFormats[j+1] = Tmp;
					}
				}
			}
		}
	}
}

void CMenus::UpdatedFilteredVideoModes()
{

}

void CMenus::UpdateVideoModeSettings()
{

}

void CMenus::OnInit()
{
	UpdateVideoModeSettings();

	m_MousePos.x = 2;
	m_MousePos.y = 2;

	// load menu images
	m_lMenuImages.clear();
	Storage()->ListDirectory(IStorage::TYPE_ALL, "ui/menuimages", MenuImageScan, this);

	// clear filter lists
	//m_lFilters.clear();

	if(g_Config.m_ClShowWelcome)
		m_Popup = POPUP_LANGUAGE;
	g_Config.m_ClShowWelcome = 0;
	if(g_Config.m_ClSkipStartMenu)
		SetMenuPage(g_Config.m_UiBrowserPage);

	Console()->Chain("add_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("remove_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("add_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("remove_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("snd_enable_music", ConchainToggleMusic, this);

	// setup load amount
	m_LoadCurrent = 0;
	m_LoadTotal = g_pData->m_NumImages;
	if(!g_Config.m_ClThreadsoundloading)
		m_LoadTotal += g_pData->m_NumSounds;
}

void CMenus::PopupMessage(const char *pTopic, const char *pBody, const char *pButton, int Next)
{
	// reset active item
	UI()->SetActiveItem(0);

	str_copy(m_aMessageTopic, pTopic, sizeof(m_aMessageTopic));
	str_copy(m_aMessageBody, pBody, sizeof(m_aMessageBody));
	str_copy(m_aMessageButton, pButton, sizeof(m_aMessageButton));
	m_Popup = POPUP_MESSAGE;
	m_NextPopup = Next;
}

int CMenus::Render()
{
	return 0;
}

void CMenus::SetActive(bool Active)
{
	m_MenuActive = Active;
	if(!m_MenuActive)
	{
		if(Client()->State() == IClient::STATE_ONLINE)
		{
			m_pClient->OnRelease();
			if(Client()->State() == IClient::STATE_ONLINE && m_SkinModified)
			{
				m_SkinModified = false;
				m_pClient->SendSkinChange();
			}
		}
	}
	else
	{
		m_SkinModified = false;
		if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		{
			m_pClient->OnRelease();
		}
	}
}

void CMenus::OnReset()
{
}

bool CMenus::OnMouseMove(float x, float y)
{
	return false;
}

bool CMenus::OnInput(IInput::CEvent e)
{
	return false;
}

void CMenus::OnConsoleInit()
{
	// load filters
	LoadFilters();

	// add standard filters in case they are missing
	bool FilterStandard = false;
	bool FilterFav = false;
	bool FilterAll = false;
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		switch(m_lFilters[i].Custom())
		{
		case CBrowserFilter::FILTER_STANDARD:
			FilterStandard = true;
			break;
		case CBrowserFilter::FILTER_FAVORITES:
			FilterFav = true;
			break;
		case CBrowserFilter::FILTER_ALL:
			FilterAll = true;
		}
	}
	if(!FilterStandard)
	{
		// put it on top
		int Pos = m_lFilters.size();
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_STANDARD, "Teeworlds", ServerBrowser()));
		for(; Pos > 0; --Pos)
			Move(true, Pos);
	}
	if(!FilterFav)
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_FAVORITES, Localize("Favorites"), ServerBrowser()));
	if(!FilterAll)
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_ALL, Localize("All"), ServerBrowser()));
}

void CMenus::OnShutdown()
{
	// save filters
	SaveFilters();
}

void CMenus::OnStateChange(int NewState, int OldState)
{

}

extern "C" void font_debug_render();

void CMenus::OnRender()
{

}

bool CMenus::CheckHotKey(int Key)
{
	return false;
}

void CMenus::RenderBackground()
{

}

void CMenus::ConchainToggleMusic(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	CMenus *pSelf = (CMenus *)pUserData;
	if(pResult->NumArguments())
	{
		pSelf->ToggleMusic();
	}
}

void CMenus::ToggleMusic()
{
	if(Client()->State() == IClient::STATE_OFFLINE)
	{
		if(g_Config.m_SndMusic && !m_pClient->m_pSounds->IsPlaying(SOUND_MENU))
			m_pClient->m_pSounds->Play(CSounds::CHN_MUSIC, SOUND_MENU, 1.0f);
		else if(!g_Config.m_SndMusic && m_pClient->m_pSounds->IsPlaying(SOUND_MENU))
			m_pClient->m_pSounds->Stop(SOUND_MENU);
	}
}

void CMenus::SetMenuPage(int NewPage) {
	if(NewPage == m_MenuPage)
		return;

	m_MenuPage = NewPage;

	// update camera position
	{
		int CameraPos = -1;

		switch(m_MenuPage)
		{
		case PAGE_START: CameraPos = CCamera::POS_START; break;
		case PAGE_DEMOS: CameraPos = CCamera::POS_DEMOS; break;
		case PAGE_SETTINGS: CameraPos = CCamera::POS_SETTINGS_GENERAL+g_Config.m_UiSettingsPage; break;
		case PAGE_INTERNET: CameraPos = CCamera::POS_INTERNET; break;
		case PAGE_LAN: CameraPos = CCamera::POS_LAN;
		}

		if(CameraPos != -1 && m_pClient && m_pClient->m_pCamera)
			m_pClient->m_pCamera->ChangePosition(CameraPos);
	}
}


void CMenus::BeginScrollRegion(CScrollRegion* pSr, vec2* pOutOffset, const CScrollRegionParams* pParams)
{

}

void CMenus::EndScrollRegion(CScrollRegion* pSr)
{

}

void CMenus::ScrollRegionAddRect(CScrollRegion* pSr)
{

}

void CMenus::ScrollRegionScrollHere(CScrollRegion* pSr, int Option)
{

}

bool CMenus::ScrollRegionIsRectClipped(CScrollRegion* pSr)
{
	return false;
}
