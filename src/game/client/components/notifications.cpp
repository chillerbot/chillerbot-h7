/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <generated/client_data.h>
#include <game/client/gameclient.h>

#include "notifications.h"

CNotifications::CNotifications()
{
	m_SoundToggleTime = -99.0f;
}

void CNotifications::OnConsoleInit()
{
	IConsole* pConsole = Kernel()->RequestInterface<IConsole>();

	pConsole->Register("snd_toggle", "", CFGFLAG_CLIENT, Con_SndToggle, this, "Toggle sounds on and off");
}

void CNotifications::Con_SndToggle(IConsole::IResult *pResult, void *pUserData)
{
	CNotifications *pSelf = (CNotifications *)pUserData;

	g_Config.m_SndEnable ^= 1;
	pSelf->m_SoundToggleTime = pSelf->Client()->LocalTime();
}

void CNotifications::RenderSoundNotification()
{

}

void CNotifications::OnRender()
{
	RenderSoundNotification();
}
