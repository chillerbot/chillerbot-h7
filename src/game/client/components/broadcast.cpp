/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>

#include "broadcast.h"
#include "chat.h"
#include "scoreboard.h"
#include "motd.h"

#define BROADCAST_FONTSIZE_BIG 11.0f
#define BROADCAST_FONTSIZE_SMALL 6.5f

inline bool IsCharANum(char c)
{
	return c >= '0' && c <= '9';
}

inline int WordLengthBack(const char *pText, int MaxChars)
{
	int s = 0;
	while(MaxChars--)
	{
		if((*pText == '\n' || *pText == '\t' || *pText == ' '))
			return s;
		pText--;
		s++;
	}
	return 0;
}

inline bool IsCharWhitespace(char c)
{
	return c == '\n' || c == '\t' || c == ' ';
}

CBroadcast::CBroadcast()
{
	OnReset();
}

void CBroadcast::DoBroadcast(const char *pText)
{
	dbg_msg("cl-broadcast", "%s", pText);
}

void CBroadcast::OnReset()
{

}

void CBroadcast::OnMessage(int MsgType, void* pRawMsg)
{
	// process server broadcast message
	if(MsgType == NETMSGTYPE_SV_BROADCAST && g_Config.m_ClShowServerBroadcast &&
	   !m_pClient->m_MuteServerBroadcast)
	{
		CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)pRawMsg;

		// new broadcast message
		int RcvMsgLen = str_length(pMsg->m_pMessage);
		mem_zero(m_aSrvBroadcastMsg, sizeof(m_aSrvBroadcastMsg));
		m_aSrvBroadcastMsgLen = 0;

		CBcLineInfo UserLines[MAX_BROADCAST_LINES];
		int UserLineCount = 0;
		int LastUserLineStartPoint = 0;

		// parse colors
		for(int i = 0; i < RcvMsgLen; i++)
		{
			const char* c = pMsg->m_pMessage + i;
			const char* pTmp = c;
			int CharUtf8 = str_utf8_decode(&pTmp);
			const int Utf8Len = pTmp-c;

			if(*c == CharUtf8 && *c == '^')
			{
				if(i+3 < RcvMsgLen && IsCharANum(c[1]) && IsCharANum(c[2])  && IsCharANum(c[3]))
				{
					u8 r = (c[1] - '0') * 24 + 39;
					u8 g = (c[2] - '0') * 24 + 39;
					u8 b = (c[3] - '0') * 24 + 39;
					CBcColor Color = { r, g, b, m_aSrvBroadcastMsgLen };
					if(m_SrvBroadcastColorCount < MAX_BROADCAST_COLORS)
						m_aSrvBroadcastColorList[m_SrvBroadcastColorCount++] = Color;
					i += 3;
					continue;
				}
			}

			if(*c == CharUtf8 && *c == '\\')
			{
				if(i+1 < RcvMsgLen && c[1] == 'n' && UserLineCount < MAX_BROADCAST_LINES)
				{
					CBcLineInfo Line = { m_aSrvBroadcastMsg+LastUserLineStartPoint,
										 m_aSrvBroadcastMsgLen-LastUserLineStartPoint, 0 };
					if(Line.m_StrLen > 0)
						UserLines[UserLineCount++] = Line;
					LastUserLineStartPoint = m_aSrvBroadcastMsgLen;
					i++;
					continue;
				}
			}

			if(*c == '\n')
			{
				CBcLineInfo Line = { m_aSrvBroadcastMsg+LastUserLineStartPoint,
									 m_aSrvBroadcastMsgLen-LastUserLineStartPoint, 0 };
				if(Line.m_StrLen > 0)
					UserLines[UserLineCount++] = Line;
				LastUserLineStartPoint = m_aSrvBroadcastMsgLen;
				continue;
			}

			if(m_aSrvBroadcastMsgLen+Utf8Len < MAX_BROADCAST_MSG_LENGTH)
				m_aSrvBroadcastMsg[m_aSrvBroadcastMsgLen++] = *c;
		}

		// last user defined line
		if(LastUserLineStartPoint > 0 && UserLineCount < 3)
		{
			CBcLineInfo Line = { m_aSrvBroadcastMsg+LastUserLineStartPoint,
								 m_aSrvBroadcastMsgLen-LastUserLineStartPoint, 0 };
			if(Line.m_StrLen > 0)
				UserLines[UserLineCount++] = Line;
		}

		dbg_msg("srv-broadcast", "%s", m_aSrvBroadcastMsg);
	}
}

void CBroadcast::OnRender()
{
	// render on event not in tick
}

