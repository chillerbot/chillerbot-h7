/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/array.h>

#include <engine/keys.h>
#include <engine/demo.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include <game/client/components/camera.h>
#include <game/client/components/mapimages.h>


#include "maplayers.h"

CMapLayers::CMapLayers(int t)
{
	m_Type = t;
	m_CurrentLocalTick = 0;
	m_LastLocalTick = 0;
	m_EnvelopeUpdate = false;
	m_pMenuMap = 0;
	m_pMenuLayers = 0;
	m_OnlineStartTime = 0;
}

void CMapLayers::OnStateChange(int NewState, int OldState)
{
	if(NewState == IClient::STATE_ONLINE)
		m_OnlineStartTime = Client()->LocalTime(); // reset time for non-scynchronized envelopes
}

void CMapLayers::LoadBackgroundMap()
{
	if(!g_Config.m_ClShowMenuMap)
		return;

	int HourOfTheDay = time_houroftheday();
	char aBuf[128];
	// check for the appropriate day/night map
	str_format(aBuf, sizeof(aBuf), "ui/themes/%s_%s.map", g_Config.m_ClMenuMap, (HourOfTheDay >= 6 && HourOfTheDay < 18) ? "day" : "night");
	if(!m_pMenuMap->Load(aBuf, m_pClient->Storage()))
	{
		// fall back on generic map
		str_format(aBuf, sizeof(aBuf), "ui/themes/%s.map", g_Config.m_ClMenuMap);
		if(!m_pMenuMap->Load(aBuf, m_pClient->Storage()))
		{
			// fall back on day/night alternative map
			str_format(aBuf, sizeof(aBuf), "ui/themes/%s_%s.map", g_Config.m_ClMenuMap, (HourOfTheDay >= 6 && HourOfTheDay < 18) ? "night" : "day");
			if(!m_pMenuMap->Load(aBuf, m_pClient->Storage()))
			{
				str_format(aBuf, sizeof(aBuf), "map '%s' not found", g_Config.m_ClMenuMap);
				Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "client", aBuf);
				return;
			}
		}
	}

	str_format(aBuf, sizeof(aBuf), "loaded map '%s'", g_Config.m_ClMenuMap);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "client", aBuf);

	m_pMenuLayers->Init(Kernel(), m_pMenuMap);
	RenderTools()->RenderTilemapGenerateSkip(m_pMenuLayers);
	m_pClient->m_pMapimages->OnMenuMapLoad(m_pMenuMap);
	LoadEnvPoints(m_pMenuLayers, m_lEnvPointsMenu);
}

void CMapLayers::OnInit()
{
	if(m_Type == TYPE_BACKGROUND)
	{
		m_pMenuLayers = new CLayers;
		m_pMenuMap = CreateEngineMap();

		LoadBackgroundMap();
	}
}

void CMapLayers::OnMapLoad()
{
	if(Layers())
		LoadEnvPoints(Layers(), m_lEnvPoints);
}

void CMapLayers::OnShutdown()
{

}

void CMapLayers::LoadEnvPoints(const CLayers *pLayers, array<CEnvPoint>& lEnvPoints)
{
	lEnvPoints.clear();

	// get envelope points
	CEnvPoint *pPoints = 0x0;
	{
		int Start, Num;
		pLayers->Map()->GetType(MAPITEMTYPE_ENVPOINTS, &Start, &Num);

		if(!Num)
			return;

		pPoints = (CEnvPoint *)pLayers->Map()->GetItem(Start, 0, 0);
	}

	// get envelopes
	int Start, Num;
	pLayers->Map()->GetType(MAPITEMTYPE_ENVELOPE, &Start, &Num);
	if(!Num)
		return;


	for(int env = 0; env < Num; env++)
	{
		CMapItemEnvelope *pItem = (CMapItemEnvelope *)pLayers->Map()->GetItem(Start+env, 0, 0);

		if(pItem->m_Version >= 3)
		{
			for(int i = 0; i < pItem->m_NumPoints; i++)
				lEnvPoints.add(pPoints[i + pItem->m_StartPoint]);
		}
		else
		{
			// backwards compatibility
			for(int i = 0; i < pItem->m_NumPoints; i++)
			{
				// convert CEnvPoint_v1 -> CEnvPoint
				CEnvPoint_v1 *pEnvPoint_v1 = &((CEnvPoint_v1 *)pPoints)[i + pItem->m_StartPoint];
				CEnvPoint p;

				p.m_Time = pEnvPoint_v1->m_Time;
				p.m_Curvetype = pEnvPoint_v1->m_Curvetype;

				for(int c = 0; c < pItem->m_Channels; c++)
				{
					p.m_aValues[c] = pEnvPoint_v1->m_aValues[c];
					p.m_aInTangentdx[c] = 0;
					p.m_aInTangentdy[c] = 0;
					p.m_aOutTangentdx[c] = 0;
					p.m_aOutTangentdy[c] = 0;
				}

				lEnvPoints.add(p);
			}
		}
	}
}

void CMapLayers::EnvelopeUpdate()
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		m_CurrentLocalTick = pInfo->m_CurrentTick;
		m_LastLocalTick = pInfo->m_CurrentTick;
		m_EnvelopeUpdate = true;
	}
}

void CMapLayers::EnvelopeEval(float TimeOffset, int Env, float *pChannels, void *pUser)
{

}

void CMapLayers::OnRender()
{

}

void CMapLayers::ConchainBackgroundMap(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
		((CMapLayers*)pUserData)->BackgroundMapUpdate();
}

void CMapLayers::OnConsoleInit()
{
	Console()->Chain("cl_menu_map", ConchainBackgroundMap, this);
	Console()->Chain("cl_show_menu_map", ConchainBackgroundMap, this);
}

void CMapLayers::BackgroundMapUpdate()
{
	if(m_Type == TYPE_BACKGROUND && m_pMenuMap)
	{
		// unload map
		m_pMenuMap->Unload();

		LoadBackgroundMap();
	}
}
