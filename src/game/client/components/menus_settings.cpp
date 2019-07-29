/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/math.h>

#include <engine/engine.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/components/maplayers.h>
#include <game/client/components/sounds.h>
#include <game/client/components/stats.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>

#include "binds.h"
#include "countryflags.h"
#include "menus.h"

CMenusKeyBinder CMenus::m_Binder;

CMenusKeyBinder::CMenusKeyBinder()
{
	m_TakeKey = false;
	m_GotKey = false;
}

bool CMenusKeyBinder::OnInput(IInput::CEvent Event)
{
	if(m_TakeKey)
	{
		int TriggeringEvent = (Event.m_Key == KEY_MOUSE_1) ? IInput::FLAG_PRESS : IInput::FLAG_RELEASE;
		if(Event.m_Flags&TriggeringEvent) // delay to RELEASE to support composed binds
		{
			m_Key = Event;
			m_GotKey = true;
			m_TakeKey = false;

			int Mask = CBinds::GetModifierMask(Input()); // always > 0
			m_Modifier = 0;
			while(!(Mask&1)) // this computes a log2, we take the first modifier flag in mask.
			{
				Mask >>= 1;
				m_Modifier++;
			}
			// prevent from adding e.g. a control modifier to lctrl
			if(CBinds::ModifierMatchesKey(m_Modifier, Event.m_Key))
				m_Modifier = 0;
		}
		return true;
	}

	return false;
}

void CMenus::SaveSkinfile()
{
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "skins/%s.json", m_aSaveSkinName);
	IOHANDLE File = Storage()->OpenFile(aBuf, IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;

	// file start
	const char *p = "{\"skin\": {";
	io_write(File, p, str_length(p));
	int Count = 0;

	for(int PartIndex = 0; PartIndex < NUM_SKINPARTS; PartIndex++)
	{
		if(!CSkins::ms_apSkinVariables[PartIndex][0])
			continue;

		// part start
		if(Count == 0)
		{
			p = "\n";
			io_write(File, p, str_length(p));
		}
		else
		{
			p = ",\n";
			io_write(File, p, str_length(p));
		}
		str_format(aBuf, sizeof(aBuf), "\t\"%s\": {\n", CSkins::ms_apSkinPartNames[PartIndex]);
		io_write(File, aBuf, str_length(aBuf));

		// part content
		str_format(aBuf, sizeof(aBuf), "\t\t\"filename\": \"%s\",\n", CSkins::ms_apSkinVariables[PartIndex]);
		io_write(File, aBuf, str_length(aBuf));

		str_format(aBuf, sizeof(aBuf), "\t\t\"custom_colors\": \"%s\"", *CSkins::ms_apUCCVariables[PartIndex]?"true":"false");
		io_write(File, aBuf, str_length(aBuf));

		if(*CSkins::ms_apUCCVariables[PartIndex])
		{
			for(int c = 0; c < CSkins::NUM_COLOR_COMPONENTS-1; c++)
			{
				int Val = (*CSkins::ms_apColorVariables[PartIndex] >> (2-c)*8) & 0xff;
				str_format(aBuf, sizeof(aBuf), ",\n\t\t\"%s\": %d", CSkins::ms_apColorComponents[c], Val);
				io_write(File, aBuf, str_length(aBuf));
			}
			if(PartIndex == SKINPART_MARKING)
			{
				int Val = (*CSkins::ms_apColorVariables[PartIndex] >> 24) & 0xff;
				str_format(aBuf, sizeof(aBuf), ",\n\t\t\"%s\": %d", CSkins::ms_apColorComponents[3], Val);
				io_write(File, aBuf, str_length(aBuf));
			}
		}

		// part end
		p = "\n\t}";
		io_write(File, p, str_length(p));

		++Count;
	}

	// file end
	p = "}\n}\n";
	io_write(File, p, str_length(p));

	io_close(File);

	// add new skin to the skin list
	m_pClient->m_pSkins->AddSkin(m_aSaveSkinName);
}

void CMenus::RenderSkinPartSelection()
{

}

class CLanguage
{
public:
	CLanguage() {}
	CLanguage(const char *n, const char *f, int Code) : m_Name(n), m_FileName(f), m_CountryCode(Code) {}

	string m_Name;
	string m_FileName;
	int m_CountryCode;

	bool operator<(const CLanguage &Other) { return m_Name < Other.m_Name; }
};


int CMenus::ThemeScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	const char *pSuffix = str_endswith(pName, ".map");
	if(IsDir || !pSuffix)
		return 0;
	char aFullName[128];
	char aThemeName[128];
	str_truncate(aFullName, sizeof(aFullName), pName, pSuffix - pName);

	bool IsDay = false;
	bool IsNight = false;
	if((pSuffix = str_endswith(aFullName, "_day")))
	{
		str_truncate(aThemeName, sizeof(aThemeName), pName, pSuffix - aFullName);
		IsDay = true;
	}
	else if((pSuffix = str_endswith(aFullName, "_night")))
	{
		str_truncate(aThemeName, sizeof(aThemeName), pName, pSuffix - aFullName);
		IsNight = true;
	}
	else
		str_copy(aThemeName, aFullName, sizeof(aThemeName));

	if(str_comp(aThemeName, "none") == 0) // "none" is reserved, disallowed for maps
		return 0;

	// try to edit an existing theme
	for(int i = 0; i < pSelf->m_lThemes.size(); i++)
	{
		if(str_comp(pSelf->m_lThemes[i].m_Name, aThemeName) == 0)
		{
			if(IsDay)
				pSelf->m_lThemes[i].m_HasDay = true;
			if(IsNight)
				pSelf->m_lThemes[i].m_HasNight = true;
			return 0;
		}
	}

	// make new theme
	CTheme Theme(aThemeName, IsDay, IsNight);
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "added theme %s from ui/themes/%s", aThemeName, pName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	pSelf->m_lThemes.add(Theme);
	return 0;
}

int CMenus::ThemeIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0; // no existing theme
}

void LoadLanguageIndexfile(IStorage *pStorage, IConsole *pConsole, sorted_array<CLanguage> *pLanguages)
{
	// read file data into buffer
	const char *pFilename = "languages/index.json";
	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "couldn't open index file");
		return;
	}
	int FileSize = (int)io_length(File);
	char *pFileData = (char *)mem_alloc(FileSize, 1);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);

	if(pJsonData == 0)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, pFilename, aError);
		return;
	}

	// extract data
	const json_value &rStart = (*pJsonData)["language indices"];
	if(rStart.type == json_array)
	{
		for(unsigned i = 0; i < rStart.u.array.length; ++i)
		{
			char aFileName[128];
			str_format(aFileName, sizeof(aFileName), "languages/%s.json", (const char *)rStart[i]["file"]);
			pLanguages->add(CLanguage((const char *)rStart[i]["name"], aFileName, (json_int_t)rStart[i]["code"]));
		}
	}

	// clean up
	json_value_free(pJsonData);
}

void CMenus::RenderLanguageSelection(bool Header)
{

}

void CMenus::RenderThemeSelection(bool Header)
{

}

void CMenus::RenderSettingsPlayer()
{

}

void CMenus::RenderSettingsTeeBasic()
{

}

void CMenus::RenderSettingsTeeCustom()
{

}

void CMenus::RenderSettingsTee()
{

}

void CMenus::RenderSettingsControls()
{

}

float CMenus::RenderSettingsControlsStats(void *pUser)
{
	return 11*20.0f;
}

void CMenus::RenderSettingsGraphics()
{

}

void CMenus::RenderSettingsSound()
{

}

void CMenus::RenderSettings()
{

}
