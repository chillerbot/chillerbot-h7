/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>

#include "menus.h"
#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;

	m_WarmupHideTick = 0;
}

void CHud::OnReset()
{
	m_WarmupHideTick = 0;
}

void CHud::RenderGameTimer()
{
	if(!(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH))
	{
		char Buf[32];
		int Time = 0;
		if(m_pClient->m_GameInfo.m_TimeLimit && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_WARMUP))
		{
			Time = m_pClient->m_GameInfo.m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameData->m_GameStartTick)/Client()->GameTickSpeed());

			if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER))
				Time = 0;
		}
		else if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER))
			Time = m_pClient->m_Snap.m_pGameData->m_GameStateEndTick/Client()->GameTickSpeed();
		else
			Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameData->m_GameStartTick)/Client()->GameTickSpeed();

		str_format(Buf, sizeof(Buf), "%d:%02d", Time/60, Time%60);
		// dbg_msg("gametimer", "%s", Buf);
	}
}

void CHud::RenderPauseTimer()
{
	if((m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_STARTCOUNTDOWN|GAMESTATEFLAG_PAUSED)) == GAMESTATEFLAG_PAUSED)
	{
		char aBuf[256];

		if(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick == 0)
		{
			if(m_pClient->m_Snap.m_NotReadyCount == 1)
				str_format(aBuf, sizeof(aBuf), Localize("%d player not ready"), m_pClient->m_Snap.m_NotReadyCount);
			else if(m_pClient->m_Snap.m_NotReadyCount > 1)
				str_format(aBuf, sizeof(aBuf), Localize("%d players not ready"), m_pClient->m_Snap.m_NotReadyCount);
			else
				return;
			RenderReadyUpNotification();
		}
		else
		{
			float Seconds = static_cast<float>(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick-Client()->GameTick())/SERVER_TICK_SPEED;
			if(Seconds < 5)
				str_format(aBuf, sizeof(aBuf), "%.1f", Seconds);
			else
				str_format(aBuf, sizeof(aBuf), "%d", round_to_int(Seconds));
		}
		// dbg_msg("hud", "%s", aBuf);
	}
}

void CHud::RenderStartCountdown()
{
	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_STARTCOUNTDOWN)
	{
		const char *pText = Localize("Game starts in");

		if(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick == 0)
			return;

		char aBuf[32];
		int Seconds = (m_pClient->m_Snap.m_pGameData->m_GameStateEndTick-Client()->GameTick()+SERVER_TICK_SPEED-1)/SERVER_TICK_SPEED;
		str_format(aBuf, sizeof(aBuf), "%s %d", pText, Seconds);
		// dbg_msg("hud", "%s", aBuf);
	}
}

void CHud::RenderDeadNotification()
{
	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags == 0 && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS &&
		m_pClient->m_Snap.m_pLocalInfo && (m_pClient->m_Snap.m_pLocalInfo->m_PlayerFlags&PLAYERFLAG_DEAD))
	{
		// dbg_msg("hud", "%s", Localize("Wait for next round"));
	}
}

void CHud::RenderSuddenDeath()
{
	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		// dbg_msg("hud", "%s", Localize("Sudden Death"));
	}
}

void CHud::RenderScoreHud()
{

}

void CHud::RenderWarmupTimer()
{
	// render warmup timer
	if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_WARMUP)
	{
		char aBuf[256];

		if(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick == 0)
		{
			if(m_pClient->m_Snap.m_NotReadyCount == 1)
			{
				str_format(aBuf, sizeof(aBuf), Localize("%d player not ready"), m_pClient->m_Snap.m_NotReadyCount);
				RenderReadyUpNotification();
			}
			else if(m_pClient->m_Snap.m_NotReadyCount > 1)
			{
				str_format(aBuf, sizeof(aBuf), Localize("%d players not ready"), m_pClient->m_Snap.m_NotReadyCount);
				RenderReadyUpNotification();
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), Localize("wait for more players"));
				if(m_WarmupHideTick == 0)
					m_WarmupHideTick = time_get();
			}
		}
		else
		{
			float Seconds = static_cast<float>(m_pClient->m_Snap.m_pGameData->m_GameStateEndTick-Client()->GameTick())/SERVER_TICK_SPEED;
			if(Seconds < 5)
				str_format(aBuf, sizeof(aBuf), "%.1f", Seconds);
			else
				str_format(aBuf, sizeof(aBuf), "%d", round_to_int(Seconds));
		}

		// dbg_msg("hud", "%s", aBuf);
	}
	else if((m_pClient->m_Snap.m_pGameData->m_GameStateEndTick == 0 && m_pClient->m_Snap.m_NotReadyCount > 0) || m_pClient->m_Snap.m_pGameData->m_GameStateEndTick != 0)
		m_WarmupHideTick = 0;
}

void CHud::RenderFps()
{

}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		dbg_msg("hud", "%s", Localize("Connection Problems..."));
	}
}

void CHud::RenderTeambalanceWarning()
{
	// render prompt about team-balance
	if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS && g_Config.m_ClWarningTeambalance && m_pClient->m_ServerSettings.m_TeamBalance &&
		absolute(m_pClient->m_GameInfo.m_aTeamSize[TEAM_RED]-m_pClient->m_GameInfo.m_aTeamSize[TEAM_BLUE]) >= NUM_TEAMS)
	{
		// const char *pText = Localize("Please balance teams!");
	}
}


void CHud::RenderVoting()
{

}

void CHud::RenderCursor()
{

}

void CHud::RenderNinjaBar(float x, float y, float Progress)
{

}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{

}

void CHud::RenderSpectatorHud()
{

}

void CHud::RenderSpectatorNotification()
{

}

void CHud::RenderReadyUpNotification()
{
	if(!(m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_LocalClientID]->m_PlayerFlags&PLAYERFLAG_READY))
	{
		char aKey[64], aText[128];
		m_pClient->m_pBinds->GetKey("ready_change", aKey, sizeof(aKey));
		str_format(aText, sizeof(aText), Localize("When ready, press <%s>"), aKey);
		// dbg_msg("hud", "%s", aText);
	}
}

void CHud::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameData)
		return;

	// dont render hud if the menu is active
	if(m_pClient->m_pMenus->IsActive())
		return;

	if(g_Config.m_ClShowhud)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && !(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER)))
			RenderHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter);
		else if(m_pClient->m_Snap.m_SpecInfo.m_Active)
		{
			if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != -1)
				RenderHealthAndAmmo(&m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_Cur);
			RenderSpectatorHud();
			RenderSpectatorNotification();
		}

		RenderGameTimer();
		RenderPauseTimer();
		RenderStartCountdown();
		RenderDeadNotification();
		RenderSuddenDeath();
		RenderScoreHud();
		RenderWarmupTimer();
		RenderFps();
		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
			RenderConnectionWarning();
		RenderTeambalanceWarning();
		RenderVoting();
	}
	RenderCursor();
}
