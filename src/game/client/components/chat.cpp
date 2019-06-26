/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/engine.h>
#include <engine/keys.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <game/client/localization.h>

#include <game/client/components/scoreboard.h>
#include <game/client/components/sounds.h>

#include "menus.h"
#include "chat.h"
#include "binds.h"

CChat::CChat()
{
	// init chat commands (must be in alphabetical order)
	static CChatCommand s_aapCommands[] = {
		{"/all", " - Switch to all chat", &Com_All},
		{"/friend", " <player name>", &Com_Befriend},
		{"/m", " <player name>", &Com_Mute},
		{"/mute", " <player name>", &Com_Mute},
		{"/r", " - Reply to a whisper", &Com_Reply},
		{"/team", " - Switch to team chat", &Com_Team},
		{"/w", " <player name>", &Com_Whisper},
		{"/whisper", " <player name>", &Com_Whisper},
	};
	const int CommandsCount = sizeof(s_aapCommands) / sizeof(CChatCommand);
	m_pCommands = new CChatCommands(s_aapCommands, CommandsCount);
}

CChat::~CChat()
{
	delete m_pCommands;
}

void CChat::OnReset()
{
	if(Client()->State() == IClient::STATE_OFFLINE || Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		for(int i = 0; i < MAX_LINES; i++)
		{
			m_aLines[i].m_Time = 0;
			m_aLines[i].m_Size[0].y = -1.0f;
			m_aLines[i].m_Size[1].y = -1.0f;
			m_aLines[i].m_aText[0] = 0;
			m_aLines[i].m_aName[0] = 0;
		}

		m_Mode = CHAT_NONE;
		// m_WhisperTarget = -1;
		m_LastWhisperFrom = -1;
		m_ReverseCompletion = false;
		m_Show = false;
		m_InputUpdate = false;
		m_ChatStringOffset = 0;
		m_CompletionChosen = -1;
		m_CompletionFav = -1;
		m_aCompletionBuffer[0] = 0;
		m_PlaceholderOffset = 0;
		m_PlaceholderLength = 0;
		m_pHistoryEntry = 0x0;
		m_PendingChatCounter = 0;
		m_LastChatSend = 0;
		m_IgnoreCommand = false;
		m_pCommands->Reset();

		for(int i = 0; i < CHAT_NUM; ++i)
			m_aLastSoundPlayed[i] = 0;
	}
	else
	{
		for(int i = 0; i < MAX_LINES; i++)
		{
			m_aLines[i].m_Size[0].y = -1.0f;
			m_aLines[i].m_Size[1].y = -1.0f;
		}
	}
}

void CChat::OnRelease()
{
	m_Show = false;
}

void CChat::OnStateChange(int NewState, int OldState)
{
	if(OldState <= IClient::STATE_CONNECTING)
	{
		m_Mode = CHAT_NONE;
		for(int i = 0; i < MAX_LINES; i++)
			m_aLines[i].m_Time = 0;
		m_CurrentLine = 0;
		ClearChatBuffer();
	}
}

void CChat::ConSay(IConsole::IResult *pResult, void *pUserData)
{
	((CChat*)pUserData)->Say(CHAT_ALL, pResult->GetString(0));
}

void CChat::ConSayTeam(IConsole::IResult *pResult, void *pUserData)
{
	((CChat*)pUserData)->Say(CHAT_TEAM, pResult->GetString(0));
}

void CChat::ConWhisper(IConsole::IResult *pResult, void *pUserData)
{
	CChat *pChat = (CChat *)pUserData;

	int Target = pResult->GetInteger(0);
	if(Target < 0 || Target >= MAX_CLIENTS || !pChat->m_pClient->m_aClients[Target].m_Active || pChat->m_pClient->m_LocalClientID == Target)
		pChat->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", "please enter a valid ClientID");
	else
	{
		pChat->m_WhisperTarget = Target;
		pChat->Say(CHAT_WHISPER, pResult->GetString(1));
	}
}

void CChat::ConChat(IConsole::IResult *pResult, void *pUserData)
{
	CChat *pChat = (CChat *)pUserData;

	const char *pMode = pResult->GetString(0);
	if(str_comp(pMode, "all") == 0)
		pChat->EnableMode(CHAT_ALL);
	else if(str_comp(pMode, "team") == 0)
		pChat->EnableMode(CHAT_TEAM);
	else if(str_comp(pMode, "whisper") == 0)
	{
		int Target = pChat->m_WhisperTarget; // default to ID of last target
		if(pResult->NumArguments() == 2)
			Target = pResult->GetInteger(1);
		else
		{
			// pick next valid player as target
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				int ClientID = (Target + i) % MAX_CLIENTS;
				if(pChat->m_pClient->m_aClients[ClientID].m_Active && pChat->m_pClient->m_LocalClientID != ClientID)
				{
					Target = ClientID;
					break;
				}
			}
		}
		if(Target < 0 || Target >= MAX_CLIENTS || !pChat->m_pClient->m_aClients[Target].m_Active || pChat->m_pClient->m_LocalClientID == Target)
		{
			if(pResult->NumArguments() == 2)
				pChat->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", "please enter a valid ClientID");
		}
		else
		{
			pChat->m_WhisperTarget = Target;
			pChat->EnableMode(CHAT_WHISPER);
		}
	}
	else
		((CChat*)pUserData)->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", "expected all, team or whisper as mode");
}

void CChat::ConShowChat(IConsole::IResult *pResult, void *pUserData)
{
	((CChat *)pUserData)->m_Show = pResult->GetInteger(0) != 0;
}

void CChat::OnInit()
{
	m_Input.Init(Input());
}

void CChat::OnConsoleInit()
{
	Console()->Register("say", "r", CFGFLAG_CLIENT, ConSay, this, "Say in chat");
	Console()->Register("say_team", "r", CFGFLAG_CLIENT, ConSayTeam, this, "Say in team chat");
	Console()->Register("whisper", "ir", CFGFLAG_CLIENT, ConWhisper, this, "Whisper to a client in chat");
	Console()->Register("chat", "s?i", CFGFLAG_CLIENT, ConChat, this, "Enable chat with all/team/whisper mode");
	Console()->Register("+show_chat", "", CFGFLAG_CLIENT, ConShowChat, this, "Show chat");
}

void CChat::ClearChatBuffer()
{
	mem_zero(m_ChatBuffer, sizeof(m_ChatBuffer));
	m_ChatBufferMode = CHAT_NONE;
}

bool CChat::OnInput(IInput::CEvent Event)
{
	return false;
}


void CChat::EnableMode(int Mode, const char* pText)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	m_Mode = Mode;
	ClearInput();

	if(pText) // optional text to initalize with
	{
		m_Input.Set(pText);
		m_Input.SetCursorOffset(str_length(pText));
		m_InputUpdate = true;
	}
	else if(m_Mode == m_ChatBufferMode)
	{
		m_Input.Set(m_ChatBuffer);
		m_Input.SetCursorOffset(str_length(m_ChatBuffer));
		m_InputUpdate = true;
	}
}

void CChat::ClearInput()
{
	m_Input.Clear();
	Input()->Clear();
	m_CompletionChosen = -1;

	m_IgnoreCommand = false;
	m_pCommands->Reset();
}

void CChat::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		AddLine(pMsg->m_ClientID, pMsg->m_Mode, pMsg->m_pMessage, pMsg->m_TargetID);
	}
}

void CChat::AddLine(int ClientID, int Mode, const char *pLine, int TargetID)
{
	if(*pLine == 0 || (ClientID >= 0 && (!g_Config.m_ClShowsocial || !m_pClient->m_aClients[ClientID].m_Active || // unknown client
		m_pClient->m_aClients[ClientID].m_ChatIgnore ||
		g_Config.m_ClFilterchat == 2 ||
		(m_pClient->m_LocalClientID != ClientID && g_Config.m_ClFilterchat == 1 && !m_pClient->m_aClients[ClientID].m_Friend))))
		return;
	if(Mode == CHAT_WHISPER)
	{
		// unknown client
		if(ClientID < 0 || !m_pClient->m_aClients[ClientID].m_Active || TargetID < 0 || !m_pClient->m_aClients[TargetID].m_Active)
			return;
		// should be sender or receiver
		if(ClientID != m_pClient->m_LocalClientID && TargetID != m_pClient->m_LocalClientID)
			return;
		// ignore and chat filter
		if(m_pClient->m_aClients[TargetID].m_ChatIgnore || g_Config.m_ClFilterchat == 2 ||
			(m_pClient->m_LocalClientID != TargetID && g_Config.m_ClFilterchat == 1 && !m_pClient->m_aClients[TargetID].m_Friend))
			return;
	}

	// trim right and set maximum length to 128 utf8-characters
	int Length = 0;
	const char *pStr = pLine;
	const char *pEnd = 0;
	while(*pStr)
	{
		const char *pStrOld = pStr;
		int Code = str_utf8_decode(&pStr);

		// check if unicode is not empty
		if(!str_utf8_is_whitespace(Code))
		{
			pEnd = 0;
		}
		else if(pEnd == 0)
			pEnd = pStrOld;

		if(++Length >= 127)
		{
			*(const_cast<char *>(pStr)) = 0;
			break;
		}
	}
	if(pEnd != 0)
		*(const_cast<char *>(pEnd)) = 0;

	bool Highlighted = false;
	char *p = const_cast<char*>(pLine);
	while(*p)
	{
		pLine = p;
		// find line seperator and strip multiline
		while(*p)
		{
			if(*p++ == '\n')
			{
				*(p-1) = 0;
				break;
			}
		}

		m_CurrentLine = (m_CurrentLine+1)%MAX_LINES;
		m_aLines[m_CurrentLine].m_Time = time_get();
		m_aLines[m_CurrentLine].m_Size[0].y = -1.0f;
		m_aLines[m_CurrentLine].m_Size[1].y = -1.0f;
		m_aLines[m_CurrentLine].m_ClientID = ClientID;
		m_aLines[m_CurrentLine].m_TargetID = TargetID;
		m_aLines[m_CurrentLine].m_Mode = Mode;
		m_aLines[m_CurrentLine].m_NameColor = -2;

		// check for highlighted name
		Highlighted = false;
		// do not highlight our own messages, whispers and system messages
		if(Mode != CHAT_WHISPER && ClientID >= 0 && ClientID != m_pClient->m_LocalClientID)
		{
			const char *pHL = str_find_nocase(pLine, m_pClient->m_aClients[m_pClient->m_LocalClientID].m_aName);
			if(pHL)
			{
				int Length = str_length(m_pClient->m_aClients[m_pClient->m_LocalClientID].m_aName);
				if((pLine == pHL || pHL[-1] == ' ')) // "" or " " before
				{
					if((pHL[Length] == 0 || pHL[Length] == ' ')) // "" or " " after
						Highlighted = true;
					if(pHL[Length] == ':' && (pHL[Length+1] == 0 || pHL[Length+1] == ' ')) // ":" or ": " after
						Highlighted = true;
				}
				m_CompletionFav = ClientID;
			}
		}

		m_aLines[m_CurrentLine].m_Highlighted =  Highlighted;

		int NameCID = ClientID;
		if(Mode == CHAT_WHISPER && ClientID == m_pClient->m_LocalClientID && TargetID >= 0)
			NameCID = TargetID;

		if(ClientID == SERVER_MSG)
		{
			m_aLines[m_CurrentLine].m_aName[0] = 0;
			str_format(m_aLines[m_CurrentLine].m_aText, sizeof(m_aLines[m_CurrentLine].m_aText), "*** %s", pLine);
		}
		else if(ClientID == CLIENT_MSG)
		{
			m_aLines[m_CurrentLine].m_aName[0] = 0;
			str_format(m_aLines[m_CurrentLine].m_aText, sizeof(m_aLines[m_CurrentLine].m_aText), "— %s", pLine);
		}
		else
		{
			if(m_pClient->m_aClients[ClientID].m_Team == TEAM_SPECTATORS)
				m_aLines[m_CurrentLine].m_NameColor = TEAM_SPECTATORS;

			if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
			{
				if(m_pClient->m_aClients[ClientID].m_Team == TEAM_RED)
					m_aLines[m_CurrentLine].m_NameColor = TEAM_RED;
				else if(m_pClient->m_aClients[ClientID].m_Team == TEAM_BLUE)
					m_aLines[m_CurrentLine].m_NameColor = TEAM_BLUE;
			}

			str_format(m_aLines[m_CurrentLine].m_aName, sizeof(m_aLines[m_CurrentLine].m_aName), "%s", m_pClient->m_aClients[NameCID].m_aName);
			str_format(m_aLines[m_CurrentLine].m_aText, sizeof(m_aLines[m_CurrentLine].m_aText), "%s", pLine);
		}

		char aBuf[1024];
		char aBufMode[32];
		if(Mode == CHAT_WHISPER)
			str_copy(aBufMode, "whisper", sizeof(aBufMode));
		else if(Mode == CHAT_TEAM)
			str_copy(aBufMode, "teamchat", sizeof(aBufMode));
		else
			str_copy(aBufMode, "chat", sizeof(aBufMode));

		str_format(aBuf, sizeof(aBuf), "%2d: %s: %s", NameCID, m_aLines[m_CurrentLine].m_aName, m_aLines[m_CurrentLine].m_aText);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, aBufMode, aBuf, Highlighted || Mode == CHAT_WHISPER);
	}

	if(Mode == CHAT_WHISPER && m_pClient->m_LocalClientID != ClientID)
		m_LastWhisperFrom = ClientID; // we received a a whisper

	// play sound
	int64 Now = time_get();
	if(ClientID < 0)
	{
		if(Now-m_aLastSoundPlayed[CHAT_SERVER] >= time_freq()*3/10)
		{
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_CHAT_SERVER, 0);
			m_aLastSoundPlayed[CHAT_SERVER] = Now;
		}
	}
	else if(Highlighted || Mode == CHAT_WHISPER)
	{
		if(Now-m_aLastSoundPlayed[CHAT_HIGHLIGHT] >= time_freq()*3/10)
		{
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_CHAT_HIGHLIGHT, 0);
			m_aLastSoundPlayed[CHAT_HIGHLIGHT] = Now;
		}
	}
	else
	{
		if(Now-m_aLastSoundPlayed[CHAT_CLIENT] >= time_freq()*3/10)
		{
			m_pClient->m_pSounds->Play(CSounds::CHN_GUI, SOUND_CHAT_CLIENT, 0);
			m_aLastSoundPlayed[CHAT_CLIENT] = Now;
		}
	}
}

const char* CChat::GetCommandName(int Mode)
{
	switch(Mode)
	{
		case CHAT_ALL: return "chat all";
		case CHAT_WHISPER: return "chat whisper";
		case CHAT_TEAM: return "chat team";
		default: break;
	}
	return "";
}

void CChat::OnRender()
{

}

void CChat::Say(int Mode, const char *pLine)
{
	m_LastChatSend = time_get();

	// send chat message
	CNetMsg_Cl_Say Msg;
	Msg.m_Mode = Mode;
	Msg.m_Target = Mode==CHAT_WHISPER ? m_WhisperTarget : -1;
	Msg.m_pMessage = pLine;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}

bool CChat::IsTypingCommand() const
{
	return m_Input.GetString()[0] == '/' && !m_IgnoreCommand;
}

bool CChat::ExecuteCommand()
{
	if(m_pCommands->CountActiveCommands() == 0)
		return false;

	const char* pCommandStr = m_Input.GetString();
	const CChatCommand* pCommand = m_pCommands->GetSelectedCommand();
	dbg_assert(pCommand != 0, "selected command does not exist");
	bool IsFullMatch = str_find(pCommandStr, pCommand->m_pCommandText); // if the command text is fully inside pCommandStr (aka, not a shortcut)

	if(IsFullMatch)
	{
		// execute command
		if(pCommand->m_pfnFunc != 0)
			pCommand->m_pfnFunc(this, pCommandStr);
	}
	else
	{
		// autocomplete command
		char aBuf[128];
		str_copy(aBuf, pCommand->m_pCommandText, sizeof(aBuf));
		str_append(aBuf, " ", sizeof(aBuf));

		m_Input.Set(aBuf);
		m_Input.SetCursorOffset(str_length(aBuf));
		m_InputUpdate = true;
	}
	return true;
}

// returns -1 if not found or duplicate
int CChat::IdentifyNameParameter(const char* pCommand) const
{
	// retrieve name parameter
	const char* pParameter = str_skip_to_whitespace_const(pCommand);
	if(!pParameter)
		return -1;

	// do not count leading and trailing whitespaces
	char aName[MAX_NAME_LENGTH];
	str_copy(aName, pParameter+1, sizeof(aName));
	str_clean_whitespaces_simple(aName);

	int TargetID = -1;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!m_pClient->m_aClients[i].m_Active || i == m_pClient->m_LocalClientID) // skip local user
			continue;
		if(str_length(m_pClient->m_aClients[i].m_aName) == str_length(aName) && str_comp(m_pClient->m_aClients[i].m_aName, aName) == 0)
		{
			// name strictly matches
			if(TargetID != -1)
			{
				// duplicate; be conservative
				dbg_msg("chat", "name duplicate found, aborting command");
				return -1;
			}
			TargetID = i;
		}
	}
	return TargetID;
}


// callback functions for commands
void CChat::Com_All(CChat *pChatData, const char* pCommand)
{
	const char* pParameter = str_skip_to_whitespace_const(pCommand);
	char *pBuf = 0x0;
	if(pParameter++ && *pParameter) // skip the first space
	{
		// save the parameter in a buffer before EnableMode clears it
		pBuf = (char*)mem_alloc(str_length(pParameter) + 1, 1);
		str_copy(pBuf, pParameter, str_length(pParameter) + 1);
	}
	pChatData->EnableMode(CHAT_ALL, pBuf);
	mem_free(pBuf);
}

void CChat::Com_Team(CChat *pChatData, const char* pCommand)
{
	const char* pParameter = str_skip_to_whitespace_const(pCommand);
	char *pBuf = 0x0;
	if(pParameter++ && *pParameter) // skip the first space
	{
		// save the parameter in a buffer before EnableMode clears it
		pBuf = (char*)mem_alloc(str_length(pParameter) + 1, 1);
		str_copy(pBuf, pParameter, str_length(pParameter) + 1);
	}
	pChatData->EnableMode(CHAT_TEAM, pBuf);
	mem_free(pBuf);
}

void CChat::Com_Reply(CChat *pChatData, const char* pCommand)
{
	if(pChatData->m_LastWhisperFrom == -1)
		pChatData->ClearInput(); // just reset the chat
	else
	{
		pChatData->m_WhisperTarget = pChatData->m_LastWhisperFrom;

		const char* pParameter = str_skip_to_whitespace_const(pCommand);
		char *pBuf = 0x0;
		if(pParameter++ && *pParameter) // skip the first space
		{
			// save the parameter in a buffer before EnableMode clears it
			pBuf = (char*)mem_alloc(str_length(pParameter) + 1, 1);
			str_copy(pBuf, pParameter, sizeof(pBuf));
		}
		pChatData->EnableMode(CHAT_WHISPER, pBuf);
		mem_free(pBuf);
	}
}

void CChat::Com_Whisper(CChat *pChatData, const char* pCommand)
{
	int TargetID = pChatData->IdentifyNameParameter(pCommand);
	if(TargetID != -1)
	{
		pChatData->m_WhisperTarget = TargetID;
		pChatData->EnableMode(CHAT_WHISPER);
	}
}

void CChat::Com_Mute(CChat *pChatData, const char* pCommand)
{
	int TargetID = pChatData->IdentifyNameParameter(pCommand);
	if(TargetID != -1)
	{
		pChatData->m_pClient->m_aClients[TargetID].m_ChatIgnore ^= 1;

		pChatData->ClearInput();

		char aMsg[128];
		str_format(aMsg, sizeof(aMsg), pChatData->m_pClient->m_aClients[TargetID].m_ChatIgnore ? Localize("'%s' was muted") : Localize("'%s' was unmuted"), pChatData->m_pClient->m_aClients[TargetID].m_aName);
		pChatData->AddLine(CLIENT_MSG, CHAT_ALL, aMsg, -1);
	}
}

void CChat::Com_Befriend(CChat *pChatData, const char* pCommand)
{
	int TargetID = pChatData->IdentifyNameParameter(pCommand);
	if(TargetID != -1)
	{
		bool isFriend = pChatData->m_pClient->m_aClients[TargetID].m_Friend;
		if(isFriend)
			pChatData->m_pClient->Friends()->RemoveFriend(pChatData->m_pClient->m_aClients[TargetID].m_aName, pChatData->m_pClient->m_aClients[TargetID].m_aClan);
		else
			pChatData->m_pClient->Friends()->AddFriend(pChatData->m_pClient->m_aClients[TargetID].m_aName, pChatData->m_pClient->m_aClients[TargetID].m_aClan);
		pChatData->m_pClient->m_aClients[TargetID].m_Friend ^= 1;

		pChatData->ClearInput();

		char aMsg[128];
		str_format(aMsg, sizeof(aMsg), !isFriend ? Localize("'%s' was added as a friend") : Localize("'%s' was removed as a friend"), pChatData->m_pClient->m_aClients[TargetID].m_aName);
		pChatData->AddLine(CLIENT_MSG, CHAT_ALL, aMsg, -1);
	}
}


// CChatCommands methods
CChat::CChatCommands::CChatCommands(CChatCommand apCommands[], int Count) : m_apCommands(apCommands), m_Count(Count), m_pSelectedCommand(0x0) { }
CChat::CChatCommands::~CChatCommands() { }

// selection
void CChat::CChatCommands::Reset()
{
	m_pSelectedCommand = 0x0;
}

// Example: /whisper command will match "/whi", "/whisper" and "/whisper tee"
void CChat::CChatCommands::Filter(const char* pLine)
{
	char aCommandStr[64];
	str_copy(aCommandStr, pLine, sizeof(aCommandStr));
	// truncate the string at the first whitespace to get the command
	char* pFirstWhitespace = str_skip_to_whitespace(aCommandStr);
	if(pFirstWhitespace)
		*pFirstWhitespace = 0;

	for(int i = 0; i < m_Count; i++)
		m_apCommands[i].m_aFiltered = (str_find_nocase(m_apCommands[i].m_pCommandText, aCommandStr) != m_apCommands[i].m_pCommandText);

	// also update selected command
	if(!GetSelectedCommand() || GetSelectedCommand()->m_aFiltered)
	{
		if(CountActiveCommands() > 0)
			m_pSelectedCommand = &m_apCommands[GetActiveIndex(0)]; // default to first command
		else
			m_pSelectedCommand = 0x0;
	}
}

// this will not return a correct value if we are not writing a command (m_Input.GetString()[0] == '/')
int CChat::CChatCommands::CountActiveCommands() const
{
	int n = m_Count;
	for(int i = 0; i < m_Count; i++)
		n -= m_apCommands[i].m_aFiltered;
	return n;
}

const CChat::CChatCommand* CChat::CChatCommands::GetCommand(int index) const
{
	return &m_apCommands[GetActiveIndex(index)];
}

const CChat::CChatCommand* CChat::CChatCommands::GetSelectedCommand() const
{
	return m_pSelectedCommand;
}

void CChat::CChatCommands::SelectPreviousCommand()
{
	CChatCommand* LastCommand = 0x0;
	for(int i = 0; i < m_Count; i++)
	{
		if(m_apCommands[i].m_aFiltered)
			continue;
		if(&m_apCommands[i] == m_pSelectedCommand)
		{
			m_pSelectedCommand = LastCommand;
			return;
		}
		LastCommand = &m_apCommands[i];
	}
}

void CChat::CChatCommands::SelectNextCommand()
{
	bool FoundSelected = false;
	for(int i = 0; i < m_Count; i++)
	{
		if(m_apCommands[i].m_aFiltered)
			continue;
		if(FoundSelected)
		{
			m_pSelectedCommand = &m_apCommands[i];
			return;
		}
		if(&m_apCommands[i] == m_pSelectedCommand)
			FoundSelected = true;
	}
}

int CChat::CChatCommands::GetActiveIndex(int index) const
{
	for(int i = 0; i < m_Count; i++)
	{
		if(m_apCommands[i].m_aFiltered)
			index++;
		if(i == index)
			return i;
	}
	dbg_break();
	return -1;
}
