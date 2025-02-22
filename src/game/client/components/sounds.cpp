/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/engine.h>
#include <engine/sound.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>
#include <game/client/gameclient.h>
#include <game/client/components/camera.h>
#include <game/client/components/menus.h>
#include "sounds.h"


struct CUserData
{
	CGameClient *m_pGameClient;
	bool m_Render;
} g_UserData;

static int LoadSoundsThread(void *pUser)
{
	return 0;
}

ISound::CSampleHandle CSounds::GetSampleId(int SetId)
{
	if(!g_Config.m_SndEnable || !Sound()->IsSoundEnabled() || m_WaitForSoundJob || SetId < 0 || SetId >= g_pData->m_NumSounds)
		return ISound::CSampleHandle();
	
	CDataSoundset *pSet = &g_pData->m_aSounds[SetId];
	if(!pSet->m_NumSounds)
		return ISound::CSampleHandle();

	if(pSet->m_NumSounds == 1)
		return pSet->m_aSounds[0].m_Id;

	// return random one
	int Id;
	do
	{
		Id = random_int() % pSet->m_NumSounds;
	}
	while(Id == pSet->m_Last);
	pSet->m_Last = Id;
	return pSet->m_aSounds[Id].m_Id;
}

void CSounds::OnInit()
{
	// setup sound channels
	Sound()->SetChannelVolume(CSounds::CHN_GUI, 1.0f);
	Sound()->SetChannelVolume(CSounds::CHN_MUSIC, 1.0f);
	Sound()->SetChannelVolume(CSounds::CHN_WORLD, 0.9f);
	Sound()->SetChannelVolume(CSounds::CHN_GLOBAL, 1.0f);

	Sound()->SetListenerPos(0.0f, 0.0f);

	ClearQueue();

	// load sounds
	if(g_Config.m_ClThreadsoundloading)
	{
		g_UserData.m_pGameClient = m_pClient;
		g_UserData.m_Render = false;
		m_pClient->Engine()->AddJob(&m_SoundJob, LoadSoundsThread, &g_UserData);
		m_WaitForSoundJob = true;
	}
	else
	{
		g_UserData.m_pGameClient = m_pClient;
		g_UserData.m_Render = true;
		LoadSoundsThread(&g_UserData);
		m_WaitForSoundJob = false;
	}
}

void CSounds::OnReset()
{
	if(Client()->State() >= IClient::STATE_ONLINE)
	{
		Sound()->StopAll();
		ClearQueue();
	}
}

void CSounds::OnStateChange(int NewState, int OldState)
{
	if(NewState == IClient::STATE_ONLINE || NewState == IClient::STATE_DEMOPLAYBACK)
		OnReset();
}

void CSounds::OnRender()
{
	// check for sound initialisation
	if(m_WaitForSoundJob)
	{
		if(m_SoundJob.Status() == CJob::STATE_DONE)
			m_WaitForSoundJob = false;
		else
			return;
	}

	// set listner pos
	vec2 Pos = *m_pClient->m_pCamera->GetCenter();
	Sound()->SetListenerPos(Pos.x, Pos.y);

	// play sound from queue
	if(m_QueuePos > 0)
	{
		int64 Now = time_get();
		if(m_QueueWaitTime <= Now)
		{
			Play(m_aQueue[0].m_Channel, m_aQueue[0].m_SetId, 1.0f);
			m_QueueWaitTime = Now+time_freq()*3/10; // wait 300ms before playing the next one
			if(--m_QueuePos > 0)
				mem_move(m_aQueue, m_aQueue+1, m_QueuePos*sizeof(QueueEntry));
		}
	}
}

void CSounds::ClearQueue()
{
	mem_zero(m_aQueue, sizeof(m_aQueue));
	m_QueuePos = 0;
	m_QueueWaitTime = time_get();
}

void CSounds::Enqueue(int Channel, int SetId)
{
	// add sound to the queue
	if(m_QueuePos < QUEUE_SIZE)
	{
		if(Channel == CHN_MUSIC || !g_Config.m_ClEditor)
		{
			m_aQueue[m_QueuePos].m_Channel = Channel;
			m_aQueue[m_QueuePos++].m_SetId = SetId;
		}
	}
}

void CSounds::Play(int Chn, int SetId, float Vol)
{
	if(Chn == CHN_MUSIC && !g_Config.m_SndMusic)
		return;

	ISound::CSampleHandle SampleId = GetSampleId(SetId);
	if(!SampleId.IsValid())
		return;

	int Flags = 0;
	if(Chn == CHN_MUSIC)
		Flags = ISound::FLAG_LOOP;

	Sound()->Play(Chn, SampleId, Flags);
}

void CSounds::PlayAt(int Chn, int SetId, float Vol, vec2 Pos)
{
	if(Chn == CHN_MUSIC && !g_Config.m_SndMusic)
		return;
	
	ISound::CSampleHandle SampleId = GetSampleId(SetId);
	if(!SampleId.IsValid())
		return;

	int Flags = 0;
	if(Chn == CHN_MUSIC)
		Flags = ISound::FLAG_LOOP;

	Sound()->PlayAt(Chn, SampleId, Flags, Pos.x, Pos.y);
}

void CSounds::Stop(int SetId)
{
	if(m_WaitForSoundJob || SetId < 0 || SetId >= g_pData->m_NumSounds)
		return;

	CDataSoundset *pSet = &g_pData->m_aSounds[SetId];

	for(int i = 0; i < pSet->m_NumSounds; i++)
		Sound()->Stop(pSet->m_aSounds[i].m_Id);
}

bool CSounds::IsPlaying(int SetId)
{
	if(m_WaitForSoundJob || SetId < 0 || SetId >= g_pData->m_NumSounds)
		return false;

	CDataSoundset *pSet = &g_pData->m_aSounds[SetId];
	for(int i = 0; i < pSet->m_NumSounds; i++)
	{
		if(Sound()->IsPlaying(pSet->m_aSounds[i].m_Id))
			return true;
	}
	return false;
}
