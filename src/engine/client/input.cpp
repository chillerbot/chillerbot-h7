/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/console.h>
#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>

#include "input.h"


// this header is protected so you don't include it from anywere
#define KEYS_INCLUDE
#include "keynames.h"
#undef KEYS_INCLUDE

void CInput::AddEvent(char *pText, int Key, int Flags)
{
	if(m_NumEvents != INPUT_BUFFER_SIZE)
	{
		m_aInputEvents[m_NumEvents].m_Key = Key;
		m_aInputEvents[m_NumEvents].m_Flags = Flags;
		if(!pText)
			m_aInputEvents[m_NumEvents].m_aText[0] = 0;
		else
			str_copy(m_aInputEvents[m_NumEvents].m_aText, pText, sizeof(m_aInputEvents[m_NumEvents].m_aText));
		m_aInputEvents[m_NumEvents].m_InputCount = m_InputCounter;
		m_NumEvents++;
	}
}

CInput::CInput()
{
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
	mem_zero(m_aInputState, sizeof(m_aInputState));

	m_InputCounter = 1;
	m_InputGrabbed = 0;
	m_pClipboardText = 0;

	m_PreviousHat = 0;

	m_MouseDoubleClick = false;

	m_NumEvents = 0;
}

CInput::~CInput()
{

}

void CInput::Init()
{
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	MouseModeRelative();
}

void CInput::MouseRelative(float *x, float *y)
{

}

void CInput::MouseModeAbsolute()
{

}

void CInput::MouseModeRelative()
{

}

int CInput::MouseDoubleClick()
{
	if(m_MouseDoubleClick)
	{
		m_MouseDoubleClick = false;
		return 1;
	}
	return 0;
}

const char *CInput::GetClipboardText()
{
	const char * pNothing = "";
	return pNothing;
}

void CInput::SetClipboardText(const char *pText)
{

}

void CInput::Clear()
{
	mem_zero(m_aInputState, sizeof(m_aInputState));
	mem_zero(m_aInputCount, sizeof(m_aInputCount));
	m_NumEvents = 0;
}

bool CInput::KeyState(int Key) const
{
	return false;
}

int CInput::Update()
{
	return 0;
}


IEngineInput *CreateEngineInput() { return new CInput; }
