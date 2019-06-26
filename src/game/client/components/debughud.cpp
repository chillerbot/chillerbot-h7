/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/layers.h>

#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <game/client/render.h>

#include "debughud.h"

void CDebugHud::RenderNetCorrections()
{

}

void CDebugHud::RenderTuning()
{

}

void CDebugHud::OnRender()
{
	RenderTuning();
	RenderNetCorrections();
}
