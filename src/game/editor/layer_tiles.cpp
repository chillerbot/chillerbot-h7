/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/color.h>
#include <base/math.h>

#include <engine/client.h>
#include <engine/console.h>

#include <generated/client_data.h>
#include <game/client/localization.h>
#include "editor.h"


CLayerTiles::CLayerTiles(int w, int h)
{
	m_Type = LAYERTYPE_TILES;
	str_copy(m_aName, "Tiles", sizeof(m_aName));
	m_Width = w;
	m_Height = h;
	m_Image = -1;
	m_Game = 0;
	m_Color.r = 255;
	m_Color.g = 255;
	m_Color.b = 255;
	m_Color.a = 255;
	m_ColorEnv = -1;
	m_ColorEnvOffset = 0;

	m_pTiles = new CTile[m_Width*m_Height];
	mem_zero(m_pTiles, m_Width*m_Height*sizeof(CTile));

	m_pSaveTiles = 0;
	m_SaveTilesSize = 0;

	m_SelectedRuleSet = 0;
	m_LiveAutoMap = false;
	m_SelectedAmount = 50;
}

CLayerTiles::~CLayerTiles()
{
	delete [] m_pTiles;
	m_pTiles = 0;
	delete [] m_pSaveTiles;
	m_pSaveTiles = 0;
	m_SaveTilesSize = 0;
}

void CLayerTiles::PrepareForSave()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
		{
			m_pTiles[y*m_Width+x].m_Flags &= TILEFLAG_VFLIP|TILEFLAG_HFLIP|TILEFLAG_ROTATE;
			if(m_pTiles[y*m_Width+x].m_Index == 0)
				m_pTiles[y*m_Width+x].m_Flags = 0;
		}

	if(m_Image != -1 && m_Color.a == 255)
	{
		for(int y = 0; y < m_Height; y++)
			for(int x = 0; x < m_Width; x++)
				m_pTiles[y*m_Width+x].m_Flags |= m_pEditor->m_Map.m_lImages[m_Image]->m_aTileFlags[m_pTiles[y*m_Width+x].m_Index];
	}

	int NumSaveTiles = 0; // number of unique tiles that we have to save
	CTile Tile; // current tile to be duplicated
	Tile.m_Skip = MAX_SKIP; // tell the code that we can't skip the first tile

	int NumHitMaxSkip = -1;

	for(int i = 0; i < m_Width * m_Height; i++)
	{
		// we can only store MAX_SKIP empty tiles in one tile
		if(Tile.m_Skip == MAX_SKIP)
		{
			Tile = m_pTiles[i];
			Tile.m_Skip = 0;
			NumSaveTiles++;
			NumHitMaxSkip++;
		}
		// tile is different from last one? - can't skip it
		else if(m_pTiles[i].m_Index != Tile.m_Index || m_pTiles[i].m_Flags != Tile.m_Flags)
		{
			Tile = m_pTiles[i];
			Tile.m_Skip = 0;
			NumSaveTiles++;
		}
		// if the tile is the same as the previous one - no need to
		// save it separately
		else
			Tile.m_Skip++;
	}

	if(m_pSaveTiles)
		delete [] m_pSaveTiles;

	m_pSaveTiles = new CTile[NumSaveTiles];
	m_SaveTilesSize = sizeof(CTile) * NumSaveTiles;

	int NumWrittenSaveTiles = 0;
	Tile.m_Skip = MAX_SKIP;
	for(int i = 0; i < m_Width * m_Height + 1; i++)
	{
		// again, if an tile is the same as the previous one
		// and we have place to store it, skip it!
		// if we are at the end of the layer, write one more tile
		if(i != m_Width * m_Height && Tile.m_Skip != MAX_SKIP && m_pTiles[i].m_Index == Tile.m_Index && m_pTiles[i].m_Flags == Tile.m_Flags)
		{
			Tile.m_Skip++;
		}
		// tile is not skippable
		else
		{
			// if this is not the first tile, we have to save the previous
			// tile beforehand
			if(i != 0)
				m_pSaveTiles[NumWrittenSaveTiles++] = Tile;

			// if this isn't the last tile, store it so we can check how
			// many tiles to skip
			if(i != m_Width * m_Height)
			{
				Tile = m_pTiles[i];
				Tile.m_Skip = 0;
			}
		}
	}
}

void CLayerTiles::ExtractTiles(CTile *pSavedTiles)
{
	int i = 0;
	while(i < m_Width * m_Height)
	{
		for(unsigned Counter = 0; Counter <= pSavedTiles->m_Skip && i < m_Width * m_Height; Counter++)
		{
			m_pTiles[i] = *pSavedTiles;
			m_pTiles[i++].m_Skip = 0;
		}

		pSavedTiles++;
	}
}

void CLayerTiles::MakePalette()
{
	for(int y = 0; y < m_Height; y++)
		for(int x = 0; x < m_Width; x++)
			m_pTiles[y*m_Width+x].m_Index = y*16+x;
}

void CLayerTiles::Render()
{

}

int CLayerTiles::ConvertX(float x) const { return (int)(x/32.0f); }
int CLayerTiles::ConvertY(float y) const { return (int)(y/32.0f); }

void CLayerTiles::Convert(RECTi *pOut)
{

}

void CLayerTiles::Snap()
{

}

void CLayerTiles::Clamp(RECTi *pRect)
{
	if(pRect->x < 0)
	{
		pRect->w += pRect->x;
		pRect->x = 0;
	}

	if(pRect->y < 0)
	{
		pRect->h += pRect->y;
		pRect->y = 0;
	}

	if(pRect->x+pRect->w > m_Width)
		pRect->w = m_Width - pRect->x;

	if(pRect->y+pRect->h > m_Height)
		pRect->h = m_Height - pRect->y;

	if(pRect->h < 0)
		pRect->h = 0;
	if(pRect->w < 0)
		pRect->w = 0;
}

void CLayerTiles::BrushSelecting()
{

}

int CLayerTiles::BrushGrab(CLayerGroup *pBrush)
{
	return 0;
}

void CLayerTiles::FillSelection(bool Empty, CLayer *pBrush)
{

}

void CLayerTiles::BrushDraw(CLayer *pBrush, float wx, float wy)
{

}

void CLayerTiles::BrushFlipX()
{

}

void CLayerTiles::BrushFlipY()
{

}

void CLayerTiles::BrushRotate(float Amount)
{

}

void CLayerTiles::Resize(int NewW, int NewH)
{

}

void CLayerTiles::Shift(int Direction)
{

}

void CLayerTiles::ShowInfo()
{

}

int CLayerTiles::RenderProperties()
{
	return 0;
}


void CLayerTiles::ModifyImageIndex(INDEX_MODIFY_FUNC Func)
{
	Func(&m_Image);
}

void CLayerTiles::ModifyEnvelopeIndex(INDEX_MODIFY_FUNC Func)
{
}
