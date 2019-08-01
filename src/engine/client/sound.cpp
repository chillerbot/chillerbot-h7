/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "sound.h"

#include <math.h>

enum
{
	NUM_SAMPLES = 512,
	NUM_VOICES = 64,
	NUM_CHANNELS = 16,
};

struct CSample
{
	short *m_pData;
	int m_NumFrames;
	int m_Rate;
	int m_Channels;
	int m_LoopStart;
	int m_LoopEnd;
	int m_PausedAt;
};

struct CChannel
{
	int m_Vol; // 0 - 255
};

struct CVoice
{
	CSample *m_pSample;
	CChannel *m_pChannel;
	int m_Tick;
	int m_Vol; // 0 - 255
	int m_Flags;
	int m_X, m_Y;
};

static CChannel m_aChannels[NUM_CHANNELS];

static int m_CenterX = 0;
static int m_CenterY = 0;

static float m_MaxDistance = 1500.0f;

static volatile int m_SoundVolume = 100;


int CSound::Init()
{
	return 0;
}

int CSound::Update()
{
	return 0;
}

int CSound::Shutdown()
{
	return 0;
}

int CSound::AllocID()
{
	return -1;
}

void CSound::RateConvert(int SampleID)
{

}

#if defined(CONF_WAVPACK_OPEN_FILE_INPUT_EX)
static IOHANDLE s_File;
static int ReadData(void *pId, void *pBuffer, int Size)
{
	(void)pId;
	return ReadDataOld(pBuffer, Size);
}

static int ReturnFalse(void *pId)
{
	(void)pId;
	return 0;
}

static unsigned int GetPos(void *pId)
{
	(void)pId;
	return io_tell(s_File);
}

static unsigned int GetLength(void *pId)
{
	(void)pId;
	return io_length(s_File);
}

static int PushBackByte(void *pId, int Char)
{
	(void)pId;
	return io_unread_byte(s_File, Char);
}
#endif


void CSound::SetListenerPos(float x, float y)
{
	m_CenterX = (int)x;
	m_CenterY = (int)y;
}

void CSound::SetMaxDistance(float Distance)
{
	m_MaxDistance = Distance;
}

void CSound::SetChannelVolume(int ChannelID, float Vol)
{
	m_aChannels[ChannelID].m_Vol = (int)(Vol*255.0f);
}

int CSound::Play(int ChannelID, CSampleHandle SampleID, int Flags, float x, float y)
{
	return 0;
}

int CSound::PlayAt(int ChannelID, CSampleHandle SampleID, int Flags, float x, float y)
{
	return Play(ChannelID, SampleID, Flags|ISound::FLAG_POS, x, y);
}

int CSound::Play(int ChannelID, CSampleHandle SampleID, int Flags)
{
	return Play(ChannelID, SampleID, Flags, 0, 0);
}

void CSound::Stop(CSampleHandle SampleID)
{

}

void CSound::StopAll()
{

}

bool CSound::IsPlaying(CSampleHandle SampleID)
{
	return false;
}

IEngineSound *CreateEngineSound() { return new CSound; }
