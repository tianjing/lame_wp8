#pragma once
#include "CompressedContent.h"


class WaveHeader
{
private :
	int m_Channels;
	int m_Bits;
	int m_SampleRate;
	int m_DataSize;
public:
	WaveHeader(void);
	WaveHeader(unsigned char wavedata[]);
	int GetChannels(){return m_Channels;};
	void SetChannels(int channels){m_Channels=channels;};

	int GetBits(){return m_Bits;};
	void SetBits(int bits){m_Bits=bits;};

	int GetSampleRate(){return m_SampleRate;};
	void SetSampleRate(int samplerate){m_SampleRate=samplerate;};

	int GetDataSize(){return m_DataSize;};
	void SetDataSize(int datasize){m_DataSize=datasize;};


	void ParseWaveHeade(unsigned char wavedata[]);


	void GetWaveHeade(std::vector<unsigned char> *wavedata,int datasize);
};


