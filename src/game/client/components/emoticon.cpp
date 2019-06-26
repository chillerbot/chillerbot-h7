/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include "emoticon.h"

CEmoticon::CEmoticon()
{
	OnReset();
}

void CEmoticon::ConKeyEmoticon(IConsole::IResult *pResult, void *pUserData)
{
	CEmoticon *pSelf = (CEmoticon *)pUserData;
	if(!pSelf->m_pClient->m_Snap.m_SpecInfo.m_Active && pSelf->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		pSelf->m_Active = pResult->GetInteger(0) != 0;
}

void CEmoticon::ConEmote(IConsole::IResult *pResult, void *pUserData)
{
	((CEmoticon *)pUserData)->Emote(pResult->GetInteger(0));
}

void CEmoticon::OnConsoleInit()
{
	Console()->Register("+emote", "", CFGFLAG_CLIENT, ConKeyEmoticon, this, "Open emote selector");
	Console()->Register("emote", "i", CFGFLAG_CLIENT, ConEmote, this, "Use emote");
}

void CEmoticon::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_SelectedEmote = -1;
}

void CEmoticon::OnRelease()
{
	m_Active = false;
}

void CEmoticon::OnMessage(int MsgType, void *pRawMsg)
{

}

bool CEmoticon::OnMouseMove(float x, float y)
{
	return false;
}

void CEmoticon::DrawCircle(float x, float y, float r, int Segments)
{

}

void CEmoticon::OnRender()
{

}

void CEmoticon::Emote(int Emoticon)
{
	CNetMsg_Cl_Emoticon Msg;
	Msg.m_Emoticon = Emoticon;
	Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
}
