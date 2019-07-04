/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>

#include <game/client/components/flow.h>
#include <game/client/components/effects.h>

#include "items.h"


void CItems::RenderProjectile(const CNetObj_Projectile *pCurrent, int ItemID)
{

}

void CItems::RenderPickup(const CNetObj_Pickup *pPrev, const CNetObj_Pickup *pCurrent)
{

}

void CItems::RenderFlag(const CNetObj_Flag *pPrev, const CNetObj_Flag *pCurrent, const CNetObj_GameDataFlag *pPrevGameDataFlag, const CNetObj_GameDataFlag *pCurGameDataFlag)
{

}

void CItems::RenderLaser(const struct CNetObj_Laser *pCurrent)
{

}

void CItems::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_PROJECTILE)
		{
			RenderProjectile((const CNetObj_Projectile *)pData, Item.m_ID);
		}
		else if(Item.m_Type == NETOBJTYPE_PICKUP)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if(pPrev)
				RenderPickup((const CNetObj_Pickup *)pPrev, (const CNetObj_Pickup *)pData);
		}
		else if(Item.m_Type == NETOBJTYPE_LASER)
		{
			RenderLaser((const CNetObj_Laser *)pData);
		}
	}

	// render flag
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if(Item.m_Type == NETOBJTYPE_FLAG)
		{
			const void *pPrev = Client()->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);
			if (pPrev)
			{
				const void *pPrevGameDataFlag = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_GAMEDATAFLAG, m_pClient->m_Snap.m_GameDataFlagSnapID);
				RenderFlag(static_cast<const CNetObj_Flag *>(pPrev), static_cast<const CNetObj_Flag *>(pData),
							static_cast<const CNetObj_GameDataFlag *>(pPrevGameDataFlag), m_pClient->m_Snap.m_pGameDataFlag);
			}
		}
	}
}

