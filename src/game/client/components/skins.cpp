/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/color.h>
#include <base/system.h>
#include <base/math.h>

#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>

#include "skins.h"


const char * const CSkins::ms_apSkinPartNames[NUM_SKINPARTS] = {"body", "marking", "decoration", "hands", "feet", "eyes"}; /* Localize("body","skins");Localize("marking","skins");Localize("decoration","skins");Localize("hands","skins");Localize("feet","skins");Localize("eyes","skins"); */
const char * const CSkins::ms_apColorComponents[NUM_COLOR_COMPONENTS] = {"hue", "sat", "lgt", "alp"};

char *const CSkins::ms_apSkinVariables[NUM_SKINPARTS] = {g_Config.m_PlayerSkinBody, g_Config.m_PlayerSkinMarking, g_Config.m_PlayerSkinDecoration,
													g_Config.m_PlayerSkinHands, g_Config.m_PlayerSkinFeet, g_Config.m_PlayerSkinEyes};
int *const CSkins::ms_apUCCVariables[NUM_SKINPARTS] = {&g_Config.m_PlayerUseCustomColorBody, &g_Config.m_PlayerUseCustomColorMarking, &g_Config.m_PlayerUseCustomColorDecoration,
													&g_Config.m_PlayerUseCustomColorHands, &g_Config.m_PlayerUseCustomColorFeet, &g_Config.m_PlayerUseCustomColorEyes};
int *const CSkins::ms_apColorVariables[NUM_SKINPARTS] = {&g_Config.m_PlayerColorBody, &g_Config.m_PlayerColorMarking, &g_Config.m_PlayerColorDecoration,
													&g_Config.m_PlayerColorHands, &g_Config.m_PlayerColorFeet, &g_Config.m_PlayerColorEyes};


int CSkins::SkinPartScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

int CSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}


void CSkins::OnInit()
{

}

void CSkins::AddSkin(const char *pSkinName)
{

}

void CSkins::RemoveSkin(const CSkin *pSkin)
{

}

int CSkins::Num()
{
	return 0;
}

int CSkins::NumSkinPart(int Part)
{
	return 0;
}

const CSkins::CSkin *CSkins::Get(int Index)
{
	return NULL;
}

int CSkins::Find(const char *pName, bool AllowSpecialSkin)
{
	return -1;
}

const CSkins::CSkinPart *CSkins::GetSkinPart(int Part, int Index)
{
	return NULL;
}

int CSkins::FindSkinPart(int Part, const char *pName, bool AllowSpecialPart)
{
	return -1;
}

vec3 CSkins::GetColorV3(int v) const
{
	float Dark = DARKEST_COLOR_LGT/255.0f;
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, Dark+(v&0xff)/255.0f*(1.0f-Dark)));
}

vec4 CSkins::GetColorV4(int v, bool UseAlpha) const
{
	vec3 r = GetColorV3(v);
	float Alpha = UseAlpha ? ((v>>24)&0xff)/255.0f : 1.0f;
	return vec4(r.r, r.g, r.b, Alpha);
}

int CSkins::GetTeamColor(int UseCustomColors, int PartColor, int Team, int Part) const
{
	static const int s_aTeamColors[3] = {0xC4C34E, 0x00FF6B, 0x9BFF6B};

	int TeamHue = (s_aTeamColors[Team+1]>>16)&0xff;
	int TeamSat = (s_aTeamColors[Team+1]>>8)&0xff;
	int TeamLgt = s_aTeamColors[Team+1]&0xff;
	int PartSat = (PartColor>>8)&0xff;
	int PartLgt = PartColor&0xff;

	if(!UseCustomColors)
	{
		PartSat = 255;
		PartLgt = 255;
	}

	int MinSat = 160;
	int MaxSat = 255;

	int h = TeamHue;
	int s = clamp(mix(TeamSat, PartSat, 0.2), MinSat, MaxSat);
	int l = clamp(mix(TeamLgt, PartLgt, 0.2), (int)DARKEST_COLOR_LGT, 200);

	int ColorVal = (h<<16) + (s<<8) + l;
	if(Part == SKINPART_MARKING) // keep alpha
		ColorVal += PartColor&0xff000000;

	return ColorVal;
}
