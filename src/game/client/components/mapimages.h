/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_MAPIMAGES_H
#define GAME_CLIENT_COMPONENTS_MAPIMAGES_H
#include <game/client/component.h>

class CMapImages : public CComponent
{
	enum
	{
		MAX_TEXTURES=64,

		MAP_TYPE_GAME=0,
		MAP_TYPE_MENU,
		NUM_MAP_TYPES
	};
	struct
	{
		int m_Count;
	} m_Info[NUM_MAP_TYPES];

	bool m_EasterIsLoaded;

	void LoadMapImages(class IMap *pMap, class CLayers *pLayers, int MapType);

public:
	CMapImages();

	int Num() const;

	virtual void OnMapLoad();
	void OnMenuMapLoad(class IMap *pMap);
};

#endif
