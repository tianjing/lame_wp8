#pragma once
#include "CompressedContent.h"
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;


namespace Lame_WP8{

	public ref class LameWrapper sealed
	{
	public:
		LameWrapper();
		/**
		fdafdsaf
		*/
		//property int EncodeInSampleRate;
		//property int EncodeNumChannels;
		property int EncodeBrate;
		//property int EncodeMode;
		property int EncodeQuality;
		IAsyncOperation<CompressedContent^>^ EncodeMp3(IBuffer^ inPcm);
		IAsyncAction^ EncodeMp3(Platform::String^ p_InPath,Platform::String^ p_OutPath);

		IAsyncOperation<CompressedContent^>^ DecodeMp3(IBuffer^ inMp3);
		IAsyncAction^ DecodeMp3(Platform::String^ p_InPath,Platform::String^ p_OutPath);

	};

	static const int PCM_SIZE = 4096;
	static const int MP3_SIZE = 4096;
	static const int FILE_BUFFER_SIZE= 8192;

#define LOW__BYTE(x) (x & 0x00ff)
#define HIGH_BYTE(x) ((x >> 8) & 0x00ff)
}
