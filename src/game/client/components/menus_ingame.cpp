/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/config.h>
#include <engine/demo.h>
#include <engine/friends.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>

#include "menus.h"
#include "motd.h"
#include "voting.h"

void CMenus::GetSwitchTeamInfo(CSwitchTeamInfo *pInfo)
{
	pInfo->m_aNotification[0] = 0;
	int TeamMod = m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS ? -1 : 0;
	pInfo->m_AllowSpec = true;
	pInfo->m_TimeLeft = 0;

	if(m_pClient->m_ServerSettings.m_TeamLock)
	{
		str_copy(pInfo->m_aNotification, Localize("Teams are locked"), sizeof(pInfo->m_aNotification));
		pInfo->m_AllowSpec = false;
	}
	else if(TeamMod + m_pClient->m_GameInfo.m_aTeamSize[TEAM_RED] + m_pClient->m_GameInfo.m_aTeamSize[TEAM_BLUE] >= m_pClient->m_ServerSettings.m_PlayerSlots)
	{
		str_format(pInfo->m_aNotification, sizeof(pInfo->m_aNotification), Localize("Only %d active players are allowed"), m_pClient->m_ServerSettings.m_PlayerSlots);
	}
	else if(m_pClient->m_TeamCooldownTick + 1 >= Client()->GameTick())
	{
		pInfo->m_TimeLeft = (m_pClient->m_TeamCooldownTick - Client()->GameTick()) / Client()->GameTickSpeed() + 1;
		str_format(pInfo->m_aNotification, sizeof(pInfo->m_aNotification), Localize("Teams are locked. Time to wait before changing team: %02d:%02d"), pInfo->m_TimeLeft / 60, pInfo->m_TimeLeft % 60);
		pInfo->m_AllowSpec = false;
	}
}

void CMenus::RenderGame()
{

}

void CMenus::RenderPlayers()
{
	// player options
	dbg_msg(Localize("Player"), "%s", Localize("Player options"));

	// options
	int Teams[3] = { TEAM_RED, TEAM_BLUE, TEAM_SPECTATORS };
	for(int Team = 0, Count = 0; Team < 3; ++Team)
	{
		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if(i == m_pClient->m_LocalClientID || !m_pClient->m_aClients[i].m_Active || m_pClient->m_aClients[i].m_Team != Teams[Team])
				continue;

			if(Count++ % 2 == 0)
				dbg_msg(Localize("Player"), "+-------------------------------------+");

			// player info
			// CTeeRenderInfo Info = m_pClient->m_aClients[i].m_RenderInfo;
			// RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(Label.x + Label.h / 2, Label.y + Label.h / 2));

			if(g_Config.m_ClShowUserId)
			{
				// RenderTools()->DrawClientID(TextRender(), &Cursor, i);
				dbg_msg(Localize("Player"), "id: %d", i);
			}
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[i].m_aName : "");
			dbg_msg(Localize("Player"), "Name: %s", aBuf);
			str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[i].m_aClan : "");
			dbg_msg(Localize("Player"), "Clan: %s", aBuf);

			// ignore button
			if(g_Config.m_ClFilterchat == 2 || (g_Config.m_ClFilterchat == 1 && !m_pClient->m_aClients[i].m_Friend))
				dbg_msg(Localize("Player"), "[x] ignore"); // here was untoggleable button
			else
				if(!"toggle") // m_pClient->m_aClients[i].m_ChatIgnore
					m_pClient->m_aClients[i].m_ChatIgnore ^= 1;
				dbg_msg(Localize("Player"), "[%s] ignore", m_pClient->m_aClients[i].m_ChatIgnore ? "x" : " ");

			// friend button
			if(!"toggle") // m_pClient->m_aClients[i].m_Friend
			{
				if(m_pClient->m_aClients[i].m_Friend)
					m_pClient->Friends()->RemoveFriend(m_pClient->m_aClients[i].m_aName, m_pClient->m_aClients[i].m_aClan);
				else
					m_pClient->Friends()->AddFriend(m_pClient->m_aClients[i].m_aName, m_pClient->m_aClients[i].m_aClan);

				m_pClient->m_aClients[i].m_Friend ^= 1;
			}
		}
	}
}

void CMenus::RenderServerInfo()
{
	if(!m_pClient->m_Snap.m_pLocalInfo)
		return;

	// fetch server info
	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);

	// serverinfo
	bool IsFavorite = ServerBrowser()->IsFavorite(CurrentServerInfo.m_NetAddr);
	dbg_msg(Localize("Server info"), "%s: %s", "Name", CurrentServerInfo.m_aName);
	dbg_msg(Localize("Server info"), "%s: %s", Localize("Address"), g_Config.m_UiServerAddress);
	dbg_msg(Localize("Server info"), "%s: %d", Localize("Ping"), m_pClient->m_Snap.m_pLocalInfo->m_Latency);
	dbg_msg(Localize("Server info"), "%s: %s", Localize("Version"), CurrentServerInfo.m_aVersion);
	dbg_msg(Localize("Server info"), "%s: %s", Localize("Password"), CurrentServerInfo.m_Flags&IServerBrowser::FLAG_PASSWORD ? Localize("Yes", "With") : Localize("No", "Without/None"));
	dbg_msg(Localize("Server info"), "%s: %s", Localize("Favorite"), IsFavorite ? "[x]" : "[ ]");
	if(!"toggle")
	{
		if(IsFavorite)
			ServerBrowser()->RemoveFavorite(&CurrentServerInfo);
		else
			ServerBrowser()->AddFavorite(&CurrentServerInfo);
	}
	dbg_msg(Localize("Server info"), "%s: %s", Localize("Mute broadcasts"), m_pClient->m_MuteServerBroadcast ? "[x]" : "[ ]");
	if(!"toggle")
	{
		m_pClient->m_MuteServerBroadcast ^= 1;
	}

	// gameinfo
	dbg_msg(Localize("Game info"), "%s: %s", Localize("Game type"), CurrentServerInfo.m_aGameType);
	dbg_msg(Localize("Game info"), "%s: %s", Localize("Map"), CurrentServerInfo.m_aMap);
	dbg_msg(Localize("Game info"), "%s: %s", Localize("Difficulty"), (CurrentServerInfo.m_ServerLevel == 0) ? Localize("Casual", "Server difficulty") : 
		(CurrentServerInfo.m_ServerLevel == 1 ? Localize("Normal", "Server difficulty") : Localize("Competitive", "Server difficulty")));
	dbg_msg(Localize("Game info"), "%s: %d", Localize("Score limit"), m_pClient->m_GameInfo.m_ScoreLimit);
	dbg_msg(Localize("Game info"), "%s: %d", Localize("Time limit"), m_pClient->m_GameInfo.m_TimeLimit);
	dbg_msg(Localize("Game info"), "%s: %d/%d", Localize("Players"), m_pClient->m_GameInfo.m_NumPlayers, CurrentServerInfo.m_MaxClients);

	// motd
	dbg_msg(Localize("MOTD"), "%s", m_pClient->m_pMotd->GetMotd());
}

bool CMenus::RenderServerControlServer()
{
	bool doCallVote = false;
	for(CVoteOptionClient *pOption = m_pClient->m_pVoting->m_pFirst; pOption; pOption = pOption->m_pNext)
	{
    dbg_msg("vote", "%s", pOption->m_aDescription);
	}
	return doCallVote;
}

void CMenus::RenderServerControlKick(bool FilterSpectators)
{
	int NumOptions = 0;
	int Selected = -1;
	static int aPlayerIDs[MAX_CLIENTS];
	int Teams[3] = { TEAM_RED, TEAM_BLUE, TEAM_SPECTATORS };
	for(int Team = 0; Team < 3; ++Team)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(i == m_pClient->m_LocalClientID || !m_pClient->m_aClients[i].m_Active || m_pClient->m_aClients[i].m_Team != Teams[Team] ||
				(FilterSpectators && m_pClient->m_aClients[i].m_Team == TEAM_SPECTATORS) ||
				(!FilterSpectators && m_pClient->m_Snap.m_paPlayerInfos[i] && (m_pClient->m_Snap.m_paPlayerInfos[i]->m_PlayerFlags&PLAYERFLAG_ADMIN)))
				continue;
			if(m_CallvoteSelectedPlayer == i)
				Selected = NumOptions;
			aPlayerIDs[NumOptions++] = i;
		}
	}

  dbg_msg("kick", "===] %s [===", Localize("Player"));
	for(int i = 0; i < NumOptions; i++)
	{
    dbg_msg("kick", "+-----------+");
    if(g_Config.m_ClShowUserId)
    {
      dbg_msg("kick", "id: %d", aPlayerIDs[i]);
    }
    char aBuf[64];
    str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[aPlayerIDs[i]].m_aName : "");
    dbg_msg("kick", "name: %s", aBuf);
    str_format(aBuf, sizeof(aBuf), "%s", g_Config.m_ClShowsocial ? m_pClient->m_aClients[aPlayerIDs[i]].m_aClan : "");
    dbg_msg("kick", "clan: %s", aBuf);
	}
	m_CallvoteSelectedPlayer = Selected != -1 ? aPlayerIDs[Selected] : -1;
}

void CMenus::HandleCallvote(int Page, bool Force)
{
	if(Page == 0)
		m_pClient->m_pVoting->CallvoteOption(m_CallvoteSelectedOption, m_aCallvoteReason, Force);
	else if(Page == 1)
	{
		if(m_CallvoteSelectedPlayer >= 0 && m_CallvoteSelectedPlayer < MAX_CLIENTS &&
			m_pClient->m_aClients[m_CallvoteSelectedPlayer].m_Active)
		{
			m_pClient->m_pVoting->CallvoteKick(m_CallvoteSelectedPlayer, m_aCallvoteReason, Force);
			SetActive(false);
		}
	}
	else if(Page == 2)
	{
		if(m_CallvoteSelectedPlayer >= 0 && m_CallvoteSelectedPlayer < MAX_CLIENTS &&
			m_pClient->m_aClients[m_CallvoteSelectedPlayer].m_Active)
		{
			m_pClient->m_pVoting->CallvoteSpectate(m_CallvoteSelectedPlayer, m_aCallvoteReason, Force);
			SetActive(false);
		}
	}
}

void CMenus::RenderServerControl()
{

}
