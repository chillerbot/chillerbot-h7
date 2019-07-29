/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "binds.h"
#include "menus.h"

typedef struct
{
	CLocConstString m_Name;
	const char *m_pCommand;
	int m_KeyId;
	int m_Modifier;
	CMenus::CButtonContainer m_BC;
} CKeyInfo;

static CKeyInfo gs_aKeys[] =
{
	{ "Move left", "+left", 0, 0},		// Localize - these strings are localized within CLocConstString
	{ "Move right", "+right", 0, 0},
	{ "Jump", "+jump", 0, 0},
	{ "Fire", "+fire", 0, 0},
	{ "Hook", "+hook", 0, 0},
	{ "Hammer", "+weapon1", 0, 0},
	{ "Pistol", "+weapon2", 0, 0},
	{ "Shotgun", "+weapon3", 0, 0},
	{ "Grenade", "+weapon4", 0, 0},
	{ "Laser", "+weapon5", 0, 0},
	{ "Next weapon", "+nextweapon", 0, 0},
	{ "Prev. weapon", "+prevweapon", 0, 0},
	{ "Vote yes", "vote yes", 0, 0},
	{ "Vote no", "vote no", 0, 0},
	{ "Chat", "chat all", 0, 0},
	{ "Team chat", "chat team", 0, 0},
	{ "Whisper", "chat whisper", 0, 0},
	{ "Show chat", "+show_chat", 0, 0},
	{ "Scoreboard", "+scoreboard", 0, 0},
	{ "Statboard", "+stats", 0, 0},
	{ "Emoticon", "+emote", 0, 0},
	{ "Spectator mode", "+spectate", 0, 0},
	{ "Spectate next", "spectate_next", 0, 0},
	{ "Spectate previous", "spectate_previous", 0, 0},
	{ "Console", "toggle_local_console", 0, 0},
	{ "Remote console", "toggle_remote_console", 0, 0},
	{ "Screenshot", "screenshot", 0, 0},
	{ "Respawn", "kill", 0, 0},
	{ "Ready", "ready_change", 0, 0},
	{ "Add demo marker", "add_demomarker", 0, 0},
	{ "Toggle sounds", "snd_toggle", 0, 0},
};

/*	This is for scripts/update_localization.py to work, don't remove!
	Localize("Move left");Localize("Move right");Localize("Jump");Localize("Fire");Localize("Hook");Localize("Hammer");
	Localize("Pistol");Localize("Shotgun");Localize("Grenade");Localize("Laser");Localize("Next weapon");Localize("Prev. weapon");
	Localize("Vote yes");Localize("Vote no");Localize("Chat");Localize("Team chat");Localize("Whisper");Localize("Show chat");Localize("Emoticon");
	Localize("Spectator mode");Localize("Spectate next");Localize("Spectate previous");Localize("Console");Localize("Remote console");
	Localize("Screenshot");Localize("Scoreboard");Localize("Statboard");Localize("Respawn");Localize("Ready");Localize("Add demo marker");
*/

void CMenus::UiDoGetButtons(int Start, int Stop, float ButtonHeight, float Spaceing)
{

}

float CMenus::RenderSettingsControlsMovement(void *pUser)
{
	return 0.0f;
}

float CMenus::RenderSettingsControlsWeapon(void *pUser)
{
	return 0.0f;
}

float CMenus::RenderSettingsControlsVoting(void *pUser)
{
	return 0.0f;
}

float CMenus::RenderSettingsControlsChat(void *pUser)
{
	return 0.0f;
}

float CMenus::RenderSettingsControlsScoreboard(void *pUser)
{
	return 0.0f;
}

float CMenus::RenderSettingsControlsMisc(void *pUser)
{
	return 0.0f;
}
