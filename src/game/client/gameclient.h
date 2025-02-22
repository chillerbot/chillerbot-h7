/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_GAMECLIENT_H
#define GAME_CLIENT_GAMECLIENT_H

#include <base/vmath.h>
#include <engine/client.h>
#include <engine/console.h>
#include <game/layers.h>
#include <game/gamecore.h>

class CTeeRenderInfoChiller
{
public:
	CTeeRenderInfoChiller()
	{
		for(int i = 0; i < NUM_SKINPARTS; i++)
			m_aColors[i] = vec4(1,1,1,1);
		m_Size = 1.0f;
		m_GotAirJump = 1;
	};

	int m_HatSpriteIndex;
	vec4 m_BotColor;
	vec4 m_aColors[NUM_SKINPARTS];
	float m_Size;
	int m_GotAirJump;
};

class CGameClient : public IGameClient
{
	class CStack
	{
	public:
		enum
		{
			MAX_COMPONENTS = 64,
		};

		CStack();
		void Add(class CComponent *pComponent);

		class CComponent *m_paComponents[MAX_COMPONENTS];
		int m_Num;
	};

	CStack m_All;
	CStack m_Input;
	CNetObjHandler m_NetObjHandler;

	class IEngine *m_pEngine;
	class IInput *m_pInput;
	class IClient *m_pClient;
	class ISound *m_pSound;
	class IConsole *m_pConsole;
	class IStorage *m_pStorage;
	class IDemoPlayer *m_pDemoPlayer;
	class IDemoRecorder *m_pDemoRecorder;
	class IServerBrowser *m_pServerBrowser;
	class IEditor *m_pEditor;
	class IFriends *m_pFriends;

	CLayers m_Layers;
	class CCollision m_Collision;

	void ProcessEvents();
	void ProcessTriggeredEvents(int Events, vec2 Pos);
	void UpdatePositions();

	int m_PredictedTick;
	int m_LastNewPredictedTick;

	int m_LastGameStartTick;
	int m_LastFlagCarrierRed;
	int m_LastFlagCarrierBlue;

	static void ConTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConKill(IConsole::IResult *pResult, void *pUserData);
	static void ConReadyChange(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSkinChange(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainFriendUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainXmasHatUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	void EvolveCharacter(CNetObj_Character *pCharacter, int Tick);

public:
	IKernel *Kernel() { return IInterface::Kernel(); }
	IEngine *Engine() const { return m_pEngine; }
	class IClient *Client() const { return m_pClient; }
	class ISound *Sound() const { return m_pSound; }
	class IInput *Input() const { return m_pInput; }
	class IStorage *Storage() const { return m_pStorage; }
	class IConsole *Console() { return m_pConsole; }
	class IDemoPlayer *DemoPlayer() const { return m_pDemoPlayer; }
	class IDemoRecorder *DemoRecorder() const { return m_pDemoRecorder; }
	class IServerBrowser *ServerBrowser() const { return m_pServerBrowser; }
	class CLayers *Layers() { return &m_Layers; };
	class CCollision *Collision() { return &m_Collision; };
	class IEditor *Editor() { return m_pEditor; }
	class IFriends *Friends() { return m_pFriends; }

	const char *NetobjFailedOn() { return m_NetObjHandler.FailedObjOn(); };
	int NetobjNumFailures() { return m_NetObjHandler.NumObjFailures(); };
	const char *NetmsgFailedOn() { return m_NetObjHandler.FailedMsgOn(); };

	bool m_SuppressEvents;

	// TODO: move this
	CTuningParams m_Tuning;

	enum
	{
		SERVERMODE_PURE=0,
		SERVERMODE_MOD,
		SERVERMODE_PUREMOD,
	};
	int m_ServerMode;

	int m_DemoSpecMode;
	int m_DemoSpecID;

	vec2 m_LocalCharacterPos;

	// predicted players
	CCharacterCore m_PredictedPrevChar;
	CCharacterCore m_PredictedChar;

	struct CPlayerInfoItem
	{
		const CNetObj_PlayerInfo *m_pPlayerInfo;
		int m_ClientID;
	};

	// snap pointers
	struct CSnapState
	{
		const CNetObj_Character *m_pLocalCharacter;
		const CNetObj_Character *m_pLocalPrevCharacter;
		const CNetObj_PlayerInfo *m_pLocalInfo;
		const CNetObj_SpectatorInfo *m_pSpectatorInfo;
		const CNetObj_SpectatorInfo *m_pPrevSpectatorInfo;
		const CNetObj_Flag *m_paFlags[2];
		const CNetObj_GameData *m_pGameData;
		const CNetObj_GameDataTeam *m_pGameDataTeam;
		const CNetObj_GameDataFlag *m_pGameDataFlag;
		int m_GameDataFlagSnapID;

		int m_NotReadyCount;
		int m_AliveCount[NUM_TEAMS];

		const CNetObj_PlayerInfo *m_paPlayerInfos[MAX_CLIENTS];
		CPlayerInfoItem m_aInfoByScore[MAX_CLIENTS];

		// spectate data
		struct CSpectateInfo
		{
			bool m_Active;
			int m_SpecMode;
			int m_SpectatorID;
			bool m_UsePosition;
			vec2 m_Position;
		} m_SpecInfo;

		//
		struct CCharacterInfo
		{
			bool m_Active;

			// snapshots
			CNetObj_Character m_Prev;
			CNetObj_Character m_Cur;

			// interpolated position
			vec2 m_Position;
		};

		CCharacterInfo m_aCharacters[MAX_CLIENTS];
	};

	CSnapState m_Snap;

	// client data
	struct CClientData
	{
		char m_aName[MAX_NAME_LENGTH];
		char m_aClan[MAX_CLAN_LENGTH];
		int m_Country;
		char m_aaSkinPartNames[NUM_SKINPARTS][24];
		int m_aUseCustomColors[NUM_SKINPARTS];
		int m_aSkinPartColors[NUM_SKINPARTS];
		int m_SkinPartIDs[NUM_SKINPARTS];
		int m_Team;
		int m_Emoticon;
		int m_EmoticonStart;
		CCharacterCore m_Predicted;

		CTeeRenderInfoChiller m_RenderInfo; // this is what we use

		float m_Angle;
		bool m_Active;
		bool m_ChatIgnore;
		bool m_Friend;

		void UpdateRenderInfo(CGameClient *pGameClient, int ClientID, bool UpdateSkinInfo);
		void UpdateBotRenderInfo(CGameClient *pGameClient, int ClientID);
		void Reset(CGameClient *pGameClient, int CLientID);
	};

	CClientData m_aClients[MAX_CLIENTS];
	int m_LocalClientID;
	int m_TeamCooldownTick;
	bool m_MuteServerBroadcast;
	float m_TeamChangeTime;
	bool m_IsXmasDay;
	float m_LastSkinChangeTime;
	bool m_IsEasterDay;

	struct CGameInfo
	{
		int m_GameFlags;
		int m_ScoreLimit;
		int m_TimeLimit;
		int m_MatchNum;
		int m_MatchCurrent;

		int m_NumPlayers;
		int m_aTeamSize[NUM_TEAMS];
	};

	CGameInfo m_GameInfo;

	struct CServerSettings
	{
		bool m_KickVote;
		int m_KickMin;
		bool m_SpecVote;
		bool m_TeamLock;
		bool m_TeamBalance;
		int m_PlayerSlots;
	} m_ServerSettings;

	void OnReset();

	// hooks
	virtual void OnConnected();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();
	virtual void OnInit();
	virtual void OnConsoleInit();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnMessage(int MsgId, CUnpacker *pUnpacker);
	virtual void OnNewSnapshot();
	virtual void OnDemoRecSnap();
	virtual void OnPredict();
	virtual void OnActivateEditor();
	virtual int OnSnapInput(int *pData);
	virtual void OnShutdown();
	virtual void OnEnterGame();
	virtual void OnRconLine(const char *pLine);
	virtual void OnGameOver();
	virtual void OnStartGame();

	virtual const char *GetItemName(int Type) const;
	virtual const char *Version() const;
	virtual const char *NetVersion() const;
	virtual const char *NetVersionHashUsed() const;
	virtual const char *NetVersionHashReal() const;
	virtual int ClientVersion() const;
	static void GetPlayerLabel(char* aBuf, int BufferSize, int ClientID, const char* ClientName);
	bool IsXmas() const;
	bool IsEaster() const;

	//
	void DoEnterMessage(const char *pName, int ClientID, int Team);
	void DoLeaveMessage(const char *pName, int ClientID, const char *pReason);
	void DoTeamChangeMessage(const char *pName, int ClientID, int Team);

	// actions
	// TODO: move these
	void SendSwitchTeam(int Team);
	void SendStartInfo();
	void SendKill();
	void SendReadyChange();
	void SendSkinChange();

	// pointers to all systems
	class CGameConsole *m_pGameConsole;
	class CBinds *m_pBinds;
	class CBroadcast *m_pBroadcast;
	class CParticles *m_pParticles;
	class CMenus *m_pMenus;
	class CSkins *m_pSkins;
	class CCountryFlags *m_pCountryFlags;
	class CFlow *m_pFlow;
	class CChat *m_pChat;
	class CDamageInd *m_pDamageind;
	class CCamera *m_pCamera;
	class CControls *m_pControls;
	class CEffects *m_pEffects;
	class CSounds *m_pSounds;
	class CMotd *m_pMotd;
	class CMapImages *m_pMapimages;
	class CVoting *m_pVoting;
	class CScoreboard *m_pScoreboard;
	class CStats *m_pStats;
	class CItems *m_pItems;
	class CMapLayers *m_pMapLayersBackGround;
	class CMapLayers *m_pMapLayersForeGround;
};

const char *Localize(const char *pStr, const char *pContext="")
GNUC_ATTRIBUTE((format_arg(1)));

#endif
