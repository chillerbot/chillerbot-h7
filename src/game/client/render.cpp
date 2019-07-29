/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/map.h>
#include <engine/textrender.h>
#include <generated/client_data.h>
#include <game/layers.h>
#include "animstate.h"
#include "render.h"

static float gs_SpriteWScale;
static float gs_SpriteHScale;

void CRenderTools::SelectSprite(CDataSprite *pSpr, int Flags, int sx, int sy)
{
	int x = pSpr->m_X+sx;
	int y = pSpr->m_Y+sy;
	int w = pSpr->m_W;
	int h = pSpr->m_H;
	int cx = pSpr->m_pSet->m_Gridx;
	int cy = pSpr->m_pSet->m_Gridy;

	float f = sqrtf(h*h + w*w);
	gs_SpriteWScale = w/f;
	gs_SpriteHScale = h/f;

	float x1 = x/(float)cx;
	float x2 = (x+w-1/32.0f)/(float)cx;
	float y1 = y/(float)cy;
	float y2 = (y+h-1/32.0f)/(float)cy;
	float Temp = 0;

	if(Flags&SPRITE_FLAG_FLIP_Y)
	{
		Temp = y1;
		y1 = y2;
		y2 = Temp;
	}

	if(Flags&SPRITE_FLAG_FLIP_X)
	{
		Temp = x1;
		x1 = x2;
		x2 = Temp;
	}
}

void CRenderTools::SelectSprite(int Id, int Flags, int sx, int sy)
{
	if(Id < 0 || Id >= g_pData->m_NumSprites)
		return;
	SelectSprite(&g_pData->m_aSprites[Id], Flags, sx, sy);
}

void CRenderTools::DrawSprite(float x, float y, float Size)
{
}

void CRenderTools::DrawRoundRectExt(float x, float y, float w, float h, float r, int Corners)
{
}

void CRenderTools::DrawRoundRectExt4(float x, float y, float w, float h, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, float r, int Corners)
{
}

void CRenderTools::DrawRoundRect(const CUIRect *r, vec4 Color, float Rounding)
{
}

void CRenderTools::DrawUIRect(const CUIRect *r, vec4 Color, int Corners, float Rounding)
{
}

void CRenderTools::DrawUIRect4(const CUIRect *r, vec4 ColorTopLeft, vec4 ColorTopRight, vec4 ColorBottomLeft, vec4 ColorBottomRight, int Corners, float Rounding)
{
}

void CRenderTools::RenderTee(CAnimState *pAnim, CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos)
{

}

void CRenderTools::RenderTeeHand(CTeeRenderInfo *pInfo, vec2 CenterPos, vec2 Dir, float AngleOffset,
								 vec2 PostRotOffset)
{

}

void CRenderTools::MapScreenToWorld(float CenterX, float CenterY, float ParallaxX, float ParallaxY,
	float OffsetX, float OffsetY, float Aspect, float Zoom, float aPoints[4])
{

}

void CRenderTools::MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup, float Zoom)
{

}

void CRenderTools::RenderTilemapGenerateSkip(class CLayers *pLayers)
{

}

float CRenderTools::GetClientIdRectSize(float FontSize)
{
	if(!g_Config.m_ClShowUserId) return 0;
	return 1.4f * FontSize + 0.2f * FontSize;
}
