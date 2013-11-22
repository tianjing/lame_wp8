
#include "pch.h"
#include "WaveHeader.h"

using namespace Lame_WP8;


void write_16_bits_low_high(std::vector<unsigned char>* fp, int val)
{
	fp->push_back((val & 0xff));
	fp->push_back((val >> 8) & 0xff);


	//unsigned char bytes[2];
	//bytes[0] = (val & 0xff);
	//bytes[1] = ((val >> 8) & 0xff);
	//fwrite(bytes, 1, 2, fp);
}

void write_32_bits_low_high(std::vector<unsigned char>* fp, int val)
{
	fp->push_back(val & 0xff);
	fp->push_back((val >> 8) & 0xff);
	fp->push_back((val >> 16) & 0xff);
	fp->push_back((val >> 24) & 0xff);
	//unsigned char bytes[4];

	//bytes[0] = (val & 0xff);
	//bytes[1] = ((val >> 8) & 0xff);
	//bytes[2] = ((val >> 16) & 0xff);
	//bytes[3] = ((val >> 24) & 0xff);
	//fwrite(bytes, 1, 4, fp);
}

static void WriteWaveHeader(std::vector<unsigned char> *fp, int pcmbytes, int freq, int channels, int bits)
{

	int bytes = (bits + 7) / 8;
	fp->push_back('R');
	fp->push_back('I');
	fp->push_back('F');
	fp->push_back('F');

	write_32_bits_low_high(fp, pcmbytes + 44 - 8); /* length in bytes without header */

	fp->push_back('W');
	fp->push_back('A');
	fp->push_back('V');
	fp->push_back('E');
	fp->push_back('f');
	fp->push_back('m');
	fp->push_back('t');
	fp->push_back(' ');

	write_32_bits_low_high(fp, 2 + 2 + 4 + 4 + 2 + 2); /* length of PCM format declaration area */
	write_16_bits_low_high(fp, 1); /* is PCM? */
	write_16_bits_low_high(fp, channels); /* number of channels */
	write_32_bits_low_high(fp, freq); /* sample frequency in [Hz] */
	write_32_bits_low_high(fp, freq * channels * bytes); /* bytes per second */
	write_16_bits_low_high(fp, channels * bytes); /* bytes per sample time */
	write_16_bits_low_high(fp, bits); /* bits per sample */

	fp->push_back('d');
	fp->push_back('a');
	fp->push_back('t');
	fp->push_back('a');

	write_32_bits_low_high(fp, pcmbytes); /* length in bytes of raw PCM data */


}

WaveHeader::WaveHeader()
{}

WaveHeader::WaveHeader(unsigned char wavedata[])
{
	ParseWaveHeade(wavedata);
}

void WaveHeader::ParseWaveHeade (unsigned char wavedata[]) 
{
	int channels=((int)wavedata[22])+(((int)wavedata[23]<<8));
	SetChannels( channels);

	
	int bits=((int)wavedata[34])+(((int)wavedata[35]<<8));
	SetBits(bits);

	int hz=	((int)(wavedata[24])+
		((int)(wavedata[25] << 8))+
		((int)(wavedata[26] << 16))+
		((int)(wavedata[27] << 24)));

	SetSampleRate(hz);


}

void WaveHeader::GetWaveHeade(std::vector<unsigned char> * wavedata,int datasize)
{
	//only Support 16 bits 
	WriteWaveHeader(wavedata,datasize,this->GetSampleRate(),this->GetChannels(),16);
}
