/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/config.h>
#include <engine/demo.h>
#include <engine/friends.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>

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

void CMenus::RenderServerInfo(CUIRect MainView)
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

void CMenus::RenderServerControl(CUIRect MainView)
{
	if(m_pClient->m_LocalClientID == -1)
		return;

	static int s_ControlPage = 0;
	const char *pNotification = 0;
	char aBuf[64];

	if(m_pClient->m_pVoting->IsVoting())
		pNotification = Localize("Wait for current vote to end before calling a new one.");
	else if(m_pClient->m_pVoting->CallvoteBlockTime() != 0)
	{
		str_format(aBuf, sizeof(aBuf), Localize("You must wait %d seconds before making another vote"), m_pClient->m_pVoting->CallvoteBlockTime());
		pNotification = aBuf;
	}

	bool Authed = Client()->RconAuthed();
	MainView.HSplitBottom(80.0f, &MainView, 0);
	if(pNotification && !Authed)
	{
		MainView.HSplitTop(20.0f, 0, &MainView);
		// only print notice
		CUIRect Bar;
		MainView.HSplitTop(45.0f, &Bar, &MainView);
		RenderTools()->DrawUIRect(&Bar, vec4(0.0f, 0.0f, 0.0f, 0.25f+g_Config.m_ClMenuAlpha/100.0f), CUI::CORNER_ALL, 5.0f);
		Bar.HMargin(15.0f, &Bar);
		UI()->DoLabel(&Bar, pNotification, 14.0f, CUI::ALIGN_CENTER);
		return;
	}

	// tab bar
	const float NotActiveAlpha = 0.5f;
	CUIRect Bottom, Extended, Button, Row, Note;
	MainView.HSplitTop(3.0f, 0, &MainView);
	MainView.HSplitTop(25.0f, &Row, &MainView);
	Row.VSplitLeft(Row.w/3-1.5f, &Button, &Row);
	static CButtonContainer s_Button0;
	if(DoButton_MenuTabTop(&s_Button0, Localize("Change settings"), false, &Button, s_ControlPage == 0 ? 1.0f : NotActiveAlpha, 1.0f, CUI::CORNER_T, 5.0f, 0.25f))
		s_ControlPage = 0;

	Row.VSplitLeft(1.5f, 0, &Row);
	Row.VSplitMid(&Button, &Row);
	Button.VMargin(1.5f, &Button);
	static CButtonContainer s_Button1;
	if(DoButton_MenuTabTop(&s_Button1, Localize("Kick player"), false, &Button, s_ControlPage == 1 ? 1.0f : NotActiveAlpha, 1.0f, CUI::CORNER_T, 5.0f, 0.25f))
		s_ControlPage = 1;

	Row.VSplitLeft(1.5f, 0, &Button);
	static CButtonContainer s_Button2;
	if(DoButton_MenuTabTop(&s_Button2, Localize("Move player to spectators"), false, &Button, s_ControlPage == 2 ? 1.0f : NotActiveAlpha, 1.0f, CUI::CORNER_T, 5.0f, 0.25f))
		s_ControlPage = 2;

	if(s_ControlPage == 1)
	{
		if(!m_pClient->m_ServerSettings.m_KickVote)
			pNotification = Localize("Server does not allow voting to kick players");
		else if(m_pClient->m_GameInfo.m_aTeamSize[TEAM_RED]+m_pClient->m_GameInfo.m_aTeamSize[TEAM_BLUE] < m_pClient->m_ServerSettings.m_KickMin)
		{
			str_format(aBuf, sizeof(aBuf), Localize("Kick voting requires %d players on the server"), m_pClient->m_ServerSettings.m_KickMin);
			pNotification = aBuf;
		}
	}
	else if(s_ControlPage == 2 && !m_pClient->m_ServerSettings.m_SpecVote)
		pNotification = Localize("Server does not allow voting to move players to spectators");

	if(pNotification && !Authed)
	{
		MainView.HSplitTop(20.0f+45.0f, &MainView, 0);
	}
	RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, g_Config.m_ClMenuAlpha/100.0f), CUI::CORNER_B, 5.0f);
	MainView.HSplitTop(20.0f, 0, &MainView);
	if(pNotification && !Authed)
	{
		// only print notice
		RenderTools()->DrawUIRect(&MainView, vec4(0.0f, 0.0f, 0.0f, 0.25f), CUI::CORNER_ALL, 5.0f);
		MainView.HMargin(15.0f, &MainView);
		UI()->DoLabel(&MainView, pNotification, 14.0f, CUI::ALIGN_CENTER);
		return;
	}

	if(m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == TEAM_SPECTATORS)
		pNotification = Localize("Spectators aren't allowed to start a vote.");

	// render background
	MainView.HSplitBottom(90.0f+2*20.0f, &MainView, &Extended);
	RenderTools()->DrawUIRect(&Extended, vec4(0.0f, 0.0f, 0.0f, 0.25f), CUI::CORNER_ALL, 5.0f);
	
	bool doCallVote = false;
	// render page
	if(s_ControlPage == 0)
		// double click triggers vote if not spectating
		doCallVote = RenderServerControlServer() && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS; 
	else if(s_ControlPage == 1)
		RenderServerControlKick(false);
	else if(s_ControlPage == 2)
		RenderServerControlKick(true);

	// vote menu
	Extended.Margin(5.0f, &Extended);
	Extended.HSplitTop(20.0f, &Note, &Extended);
	Extended.HSplitTop(20.0f, &Bottom, &Extended);
	{
		if(Authed || m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS){
			Bottom.VSplitRight(120.0f, &Bottom, &Button);
			
			// render kick reason
			CUIRect Reason, ClearButton, Label;
			Bottom.VSplitRight(40.0f, &Bottom, 0);
			Bottom.VSplitRight(160.0f, &Bottom, &Reason);
			Reason.VSplitRight(Reason.h, &Reason, &ClearButton);
			const char *pLabel = Localize("Reason:");
			float w = TextRender()->TextWidth(0, Reason.h*ms_FontmodHeight*0.8f, pLabel, -1, -1.0f);
			Reason.VSplitLeft(w + 10.0f, &Label, &Reason);
			Label.y += 2.0f;
			UI()->DoLabel(&Label, pLabel, Reason.h*ms_FontmodHeight*0.8f, CUI::ALIGN_LEFT);
			static float s_Offset = 0.0f;
			DoEditBox(&m_aCallvoteReason, &Reason, m_aCallvoteReason, sizeof(m_aCallvoteReason), Reason.h*ms_FontmodHeight*0.8f, &s_Offset, false, CUI::CORNER_L);

			// clear button
			{
				static CButtonContainer s_ClearButton;
				float Fade = ButtonFade(&s_ClearButton, 0.6f);
				RenderTools()->DrawUIRect(&ClearButton, vec4(1.0f, 1.0f, 1.0f, 0.33f+(Fade/0.6f)*0.165f), CUI::CORNER_R, 3.0f);
				Label = ClearButton;
				Label.y += 2.0f;
				UI()->DoLabel(&Label, "x", Label.h*ms_FontmodHeight*0.8f, CUI::ALIGN_CENTER);
				if(UI()->DoButtonLogic(s_ClearButton.GetID(), "x", 0, &ClearButton))
					m_aCallvoteReason[0] = 0;
			}
		}
 
		if(pNotification == 0)
		{
			// call vote
			static CButtonContainer s_CallVoteButton;
			if(DoButton_Menu(&s_CallVoteButton, Localize("Call vote"), 0, &Button) || doCallVote)
			{
				HandleCallvote(s_ControlPage, false);
				m_aCallvoteReason[0] = 0;
			}
		}
		else if (!Authed)
		{
			// print notice
			UI()->DoLabel(&Note, pNotification, Note.h*ms_FontmodHeight*0.8f, CUI::ALIGN_CENTER);
		}

		// extended features (only available when authed in rcon)
		if(Authed)
		{
			// background
			Extended.Margin(10.0f, &Extended);
			Extended.HSplitTop(20.0f, &Bottom, &Extended);
			Extended.HSplitTop(5.0f, 0, &Extended);

			// force vote
			Bottom.VSplitLeft(5.0f, 0, &Bottom);
			Bottom.VSplitLeft(120.0f, &Button, &Bottom);
			static CButtonContainer s_ForceVoteButton;
			if(DoButton_Menu(&s_ForceVoteButton, Localize("Force vote"), 0, &Button))
			{
				HandleCallvote(s_ControlPage, true);
				m_aCallvoteReason[0] = 0;
			}

			if(s_ControlPage == 0)
			{
				// remove vote
				Bottom.VSplitRight(10.0f, &Bottom, 0);
				Bottom.VSplitRight(120.0f, 0, &Button);
				static CButtonContainer s_RemoveVoteButton;
				if(DoButton_Menu(&s_RemoveVoteButton, Localize("Remove"), 0, &Button))
					m_pClient->m_pVoting->RemovevoteOption(m_CallvoteSelectedOption);


				// add vote
				Extended.HSplitTop(20.0f, &Bottom, &Extended);
				Bottom.VSplitLeft(5.0f, 0, &Bottom);
				Bottom.VSplitLeft(250.0f, &Button, &Bottom);
				UI()->DoLabel(&Button, Localize("Vote description:"), 14.0f, CUI::ALIGN_LEFT);

				Bottom.VSplitLeft(20.0f, 0, &Button);
				UI()->DoLabel(&Button, Localize("Vote command:"), 14.0f, CUI::ALIGN_LEFT);

				static char s_aVoteDescription[64] = {0};
				static char s_aVoteCommand[512] = {0};
				Extended.HSplitTop(20.0f, &Bottom, &Extended);
				Bottom.VSplitRight(10.0f, &Bottom, 0);
				Bottom.VSplitRight(120.0f, &Bottom, &Button);
				static CButtonContainer s_AddVoteButton;
				if(DoButton_Menu(&s_AddVoteButton, Localize("Add"), 0, &Button))
					if(s_aVoteDescription[0] != 0 && s_aVoteCommand[0] != 0)
						m_pClient->m_pVoting->AddvoteOption(s_aVoteDescription, s_aVoteCommand);

				Bottom.VSplitLeft(5.0f, 0, &Bottom);
				Bottom.VSplitLeft(250.0f, &Button, &Bottom);
				static float s_OffsetDesc = 0.0f;
				DoEditBox(&s_aVoteDescription, &Button, s_aVoteDescription, sizeof(s_aVoteDescription), 14.0f, &s_OffsetDesc, false, CUI::CORNER_ALL);

				Bottom.VMargin(20.0f, &Button);
				static float s_OffsetCmd = 0.0f;
				DoEditBox(&s_aVoteCommand, &Button, s_aVoteCommand, sizeof(s_aVoteCommand), 14.0f, &s_OffsetCmd, false, CUI::CORNER_ALL);
			}
		}
	}
}

