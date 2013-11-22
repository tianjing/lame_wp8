#pragma once

#include "pch.h"
#include "LameWrapper.h"
#include "WaveHeader.h"
#include "lame.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <list>
using namespace Lame_WP8;
using namespace Platform;

using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;


/* read data from file and skip ID3 data if exists */
static int read_buffer_with_skip_id3_data(FILE* pFile, void* pvBuffer, size_t* pnBufferSize, size_t* pnBufferPos)
{
	size_t nSize, nPos, headerSize;

	nSize = fread(pvBuffer, 1, FILE_BUFFER_SIZE, pFile);
	if (nSize < 4)
	{
		if (!nSize)
			perror("lametest: fread");
		else
			fprintf(stderr, "lametest: too small file size");
		return -1;
	}

	nPos = 0;
	/* check if the file has ID3 header */
	if (memcmp(pvBuffer, "ID3", 3) == 0)
	{
		if (nSize < 10)
		{
			nPos = fread(((unsigned char*) pvBuffer) + nSize, 1, 10 - nSize, pFile);
			if (nPos + nSize < 10)
			{
				if (!nPos)
					perror("lametest: fread");
				else
					fprintf(stderr, "lametest: too small file size");
				return -1;
			}

			nSize += nPos;
		}
		/* retrieve size of ID3 data */
		headerSize = ((size_t)(((unsigned char*) pvBuffer)[6] & 0x7F) << 21) |
			((size_t)(((unsigned char*) pvBuffer)[7] & 0x7F) << 14) |
			((size_t)(((unsigned char*) pvBuffer)[8] & 0x7F) << 7) |
			(size_t)(((unsigned char*) pvBuffer)[9] & 0x7F);
		/* if entire ID3 data has been read, then set current position */
		if (nSize - 10 >= headerSize)
		{
			nPos = 10 + headerSize;
			nSize -= nPos;
		}
		else
		{
			/* skip existing ID3 data */
			while (headerSize)
			{
				if (headerSize > FILE_BUFFER_SIZE)
					nSize = FILE_BUFFER_SIZE;
				else
					nSize = headerSize;
				nPos = fread(pvBuffer, 1, nSize, pFile);
				if (!nPos)
				{
					perror("lametest: fread");
					return -1;
				}
				headerSize -= nPos;
			}
			if (!headerSize)
			{
				nPos = 0;
				nSize = 0;
			}
		}
	}
	*pnBufferSize = nSize;
	*pnBufferPos = nPos;
	return 0;
}
//将String类型转换成 char
static const char*  PlatformStringToCharArray(String^ string)
{
	const wchar_t *W = string->Data();

	int Size = wcslen( W );
	char *CString= new char[Size + 1];
	CString[ Size ] = 0;
	for(int y=0;y<Size; y++)
	{
		CString[y] = (char)W[y];
	}
	return CString;
}

LameWrapper::LameWrapper()
{};
//计算id3长度
static size_t
	lenOfId3v2Tag(unsigned char const* buf)
{
	unsigned int b0 = buf[0] & 127;
	unsigned int b1 = buf[1] & 127;
	unsigned int b2 = buf[2] & 127;
	unsigned int b3 = buf[3] & 127;
	return (((((b0 << 7) + b1) << 7) + b2) << 7) + b3;
}

//获取mp3文件ID3的数据长度
static int GetLenOfId3v2Tag(FILE* fd)
{
	unsigned char buf[100];

	int	len = 4;
	if (fread(buf, 1, len, fd) != len)
		return -1;      /* failed */

	while (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') {
		len = 6;
		if (fread(&buf[4], 1, len, fd) != len)
			return -1;  /* failed */
		len = lenOfId3v2Tag(&buf[6]);
		return len+10;
	}
	fseek(fd, len, SEEK_CUR);
	len = 4;
	if (fread(&buf, 1, len, fd) != len)
		return -1;  /* failed */
}
//MP3解码
static int decode1( 
	hip_t   gfp
	, unsigned char*  mp3buf
	, size_t          mp3_len
	,std::vector<short>* pcm_l
	,std::vector<short>* pcm_r
	, mp3data_struct* mp3data)
{
	short left[1152],right[1152];
	int size=0,len=0;
	do
	{
		len = hip_decode1_headers(gfp,mp3buf,mp3_len,left, right, mp3data);
		if(len>0)
		{
			size+=len;
			pcm_l->insert(pcm_l->end(),left,left+len);
			pcm_r->insert(pcm_r->end(),right,right+len);
		}
		//调整输入长度为0，只接收
		mp3_len=0;
	}while(len>0);
	return size;
}

//编码时编码初始化
lame_global_flags* GetEncodeLameFlags(unsigned char waveheaddata[],LameWrapper^ p_Wrapper)
{
	WaveHeader pcmhead(waveheaddata);

	lame_global_flags* lame = lame_init();
	//if(pcmhead.GetSampleRate()>1){
	//	lame_set_in_samplerate(lame, pcmhead.GetSampleRate());
	//}
	if(pcmhead.GetChannels()>0){
		lame_set_num_channels(lame, pcmhead.GetChannels());
	}
	if(p_Wrapper->EncodeBrate>31){
		lame_set_brate(lame,p_Wrapper->EncodeBrate); //128 
	}
	if(p_Wrapper->EncodeQuality>=0&&p_Wrapper->EncodeQuality<=9){
		lame_set_quality(lame, p_Wrapper->EncodeQuality);
	}

	lame_init_params(lame);
	return lame;
}
//解码时解码初始化
lame_global_flags* GetDecodeLameFlags()
{
	lame_t lame = lame_init();
	lame_set_decode_only(lame, 1);
	if(lame_init_params(lame) == -1)
	{
		throw ref new Exception(-2,"FATAL ERROR: parameters failed to initialize properly in lame. Aborting!\n");
	}
	return lame;
}
//将wav头数据 写入wavData（put_back）
static int WriteWaveHeader(std::vector<unsigned char>* wavData, int totalsize,mp3data_struct* mp3data)
{
	WaveHeader header;
	header.SetChannels(mp3data->stereo);
	header.SetSampleRate(mp3data->samplerate);
	header.SetBits(16);
	header.GetWaveHeade(wavData, (int) totalsize);
	return wavData->size();
}
//写wav数据 将左声道和右声道数据 写入wavdata （put_back）
static void WriteWave(std::vector<unsigned char>* wavdata, std::vector<short>* left,std::vector<short>* right,mp3data_struct* mp3data)
{
	int nChannels=0,totalsize=0,size=0,headlen=0;

	nChannels = mp3data->stereo;
	size=left->size();

	int point=headlen;
	for(int i = 0 ; i < size; i++)
	{
		short  l =(*left)[i];
		/* write 16 Bits Low High */
		wavdata->push_back(LOW__BYTE(l));
		//point++;
		wavdata->push_back(HIGH_BYTE(l));
		//point++;

		//如果有第二个声道的数据则交替写入
		if(nChannels == 2)
		{
			/* write 16 Bits Low High */
			short r=(*right)[i];

			wavdata->push_back(LOW__BYTE(r));
			//point++;
			wavdata->push_back(HIGH_BYTE(r));
			//point++;
		}
	}
}
//写wav数据到pcm文件中
static void WriteWave(FILE* pcm, std::vector<short>* left,std::vector<short>* right,int datasize,mp3data_struct* mp3data)
{
	int length=left->size(),channels=mp3data->stereo;

	for (int i = 0; i < length; i++)
	{
		short  l =(*left)[i];
		unsigned char ll=LOW__BYTE(l);
		fwrite(&ll, 1, 1, pcm);
		ll=HIGH_BYTE(l);
		fwrite(&ll, 1, 1, pcm);

		if(channels==2)
		{
			short  r =(*right)[i];
			unsigned char rr=LOW__BYTE(r);
			fwrite(&rr, 1, 1, pcm);
			rr=HIGH_BYTE(r);
			fwrite(&rr, 1, 1, pcm);	

		}

	}
}
//字节集MP3解码
Windows::Foundation::IAsyncOperation<CompressedContent^>^ LameWrapper::DecodeMp3(IBuffer^ inMp3)
{
	IUnknown* pUnk = reinterpret_cast<IUnknown*>(inMp3);
	IBufferByteAccess* pAccess = NULL;
	byte* bytes = NULL;
	HRESULT hr = pUnk->QueryInterface(__uuidof(IBufferByteAccess), (void**)&pAccess);

	if (SUCCEEDED(hr))
	{
		hr = pAccess->Buffer(&bytes);
		if (SUCCEEDED(hr))
		{
			return Concurrency::create_async([=]()->CompressedContent^
			{
				CompressedContent^ result = ref new CompressedContent();


				lame_t lame=GetDecodeLameFlags();

				hip_t hip = hip_decode_init();

				mp3data_struct mp3data;
				memset(&mp3data, 0, sizeof(mp3data));


				const int mp3_len=inMp3->Length;



				std::vector<short>  pcm_l(mp3_len*4);//预先分配空间
				std::vector<short>  pcm_r(mp3_len*4);//预先分配空间

				pcm_l.clear();
				pcm_r.clear();

				//类型转换
				std::vector<unsigned char> mp3_buffer = std::vector<unsigned char>(mp3_len);

				for(int i=0; i<mp3_len; i++)
				{
					mp3_buffer[i]=(unsigned char)bytes[i];
				}

				decode1(hip,mp3_buffer.data(),mp3_len,&pcm_l,&pcm_r,&mp3data);
				//计算 wav 数据长度 和wav 总长度
				int totalsize=pcm_l.size();

				int i = (16 / 8) * mp3data.stereo;
				if (totalsize <= 0) 
				{
					totalsize = 0;
				}
				else if (totalsize > 0xFFFFFFD0 / i) 
				{
					totalsize = 0xFFFFFFD0;
				}
				else 
				{
					totalsize *= i;
				}

				std::vector<unsigned char> wav_data;
				int len= WriteWaveHeader(&wav_data,totalsize,&mp3data);
				WriteWave(&wav_data,&pcm_l,&pcm_r,&mp3data);
				result->Data=ref new Platform::Array<unsigned char>(wav_data.data(),wav_data.size());

				hip_decode_exit(hip);
				lame_close(lame);

				std::vector<short>().swap(pcm_l);
				std::vector<short>().swap(pcm_r);


				pAccess->Release();
				pUnk->Release();

				return result;
			});

		}
	}
}
//文件MP3解码
IAsyncAction^ LameWrapper::DecodeMp3(Platform::String^ p_InPath,Platform::String^ p_OutPath)
{
	return Concurrency::create_async([=]
	{
		const char* inpath=PlatformStringToCharArray(p_InPath);
		const char* outpath=PlatformStringToCharArray(p_OutPath);
		int read = 0, 
			write = 0,
			totalsize=0;
		long MP3_total_size = 0; 
		long cumulative_read = 0;

		mp3data_struct mp3data;
		memset(&mp3data, 0, sizeof(mp3data));


		FILE* pcm;
		FILE* mp3;

		lame_t lame=GetDecodeLameFlags();
		//初始化文件
		fopen_s(&pcm,outpath,"wb");
		fopen_s(&mp3,inpath, "rb");

		if( pcm==NULL||mp3==NULL)
		{
			return;
		}
		//跳过MP3的ID3
		int id3size= GetLenOfId3v2Tag(mp3);

		fseek(mp3, id3size, SEEK_SET);

		hip_t hip = hip_decode_init();

		//先写入头文件占位
		std::vector<unsigned char> wav_data;
		int len= WriteWaveHeader(&wav_data,0xFFFFFFD0,&mp3data);
		fwrite(wav_data.data(), len, sizeof(unsigned char), pcm);

		std::vector<short>  pcm_l(MP3_SIZE*4);//预先分配空间
		std::vector<short>  pcm_r(MP3_SIZE*4);//预先分配空间
		unsigned char mp3_buffer[MP3_SIZE];
		//解码
		do
		{
			//throw ref new Platform::Exception( 1000,L"change error");
			read = fread(mp3_buffer,1 , MP3_SIZE, mp3);

			len= decode1(hip,mp3_buffer,read,&pcm_l,&pcm_r,&mp3data);

			WriteWave(pcm,&pcm_l,&pcm_r,len,&mp3data);
			pcm_l.clear();
			pcm_r.clear();
			totalsize+=len;
		}while(read != 0);

		//计算 wav 数据长度 和wav 总长度
		int i = (16 / 8) * mp3data.stereo;
		if (totalsize <= 0) 
		{
			totalsize = 0;
		}
		else if (totalsize > 0xFFFFFFD0 / i) 
		{
			totalsize = 0xFFFFFFD0;
		}
		else 
		{
			totalsize *= i;
		}

		//重新写入wav头
		wav_data.clear();
		fseek(pcm, 0, SEEK_SET);
		len= WriteWaveHeader(&wav_data,totalsize,&mp3data);
		fwrite(wav_data.data(),1,len , pcm);

		std::vector<short>().swap(pcm_l);
		std::vector<short>().swap(pcm_r);
		std::vector<unsigned char>().swap(wav_data);

		fclose(pcm);
		fclose(mp3);
		hip_decode_exit(hip);
		lame_close(lame);

	});
};

//字节集MP3编码
Windows::Foundation::IAsyncOperation<CompressedContent^>^ LameWrapper::EncodeMp3(IBuffer^ inPcm)
{

	IUnknown* pUnk = reinterpret_cast<IUnknown*>(inPcm);
	IBufferByteAccess* pAccess = NULL;
	byte* bytes = NULL;
	HRESULT hr = pUnk->QueryInterface(__uuidof(IBufferByteAccess), (void**)&pAccess);
	if (SUCCEEDED(hr))
	{
		hr = pAccess->Buffer(&bytes);
		if (SUCCEEDED(hr))
		{
			return Concurrency::create_async([=]()->CompressedContent^
			{
				CompressedContent^ result = ref new CompressedContent();
				int pcmLength = inPcm->Length;
				//获取wav的头部信息 并初始化编码器
				unsigned char pcmheaddata[44];
				for (int i = 0; i < 44; i++)
				{
					pcmheaddata[i]=bytes[i];
				}

				lame_global_flags* lame = GetEncodeLameFlags(pcmheaddata,this);

				///TODO:此处直接获取了pcmLength的一半，在pcmLength为奇数的时候会丢掉最后一个字节~不过无所谓了......
				std::vector<short> inBuffer = std::vector<short>(pcmLength / 2);
				//载入数据
				for (std::vector<short>::size_type i=0; i<inBuffer.size(); i++)
				{
					inBuffer[i] = (((short)bytes[i*2+1]) << 8) + bytes[i*2];
				}

				std::vector<byte> outBuffer(inPcm->Length);
				//int size = lame_encode_buffer(lame, inBuffer.data(), inBuffer.data(), inBuffer.size(), outBuffer.data(), 0);
				int size =lame_encode_buffer_interleaved(lame, inBuffer.data(), inBuffer.size()/2,outBuffer.data(),inBuffer.size());

				if (size > 0)
				{
					result->Data = ref new Platform::Array<unsigned char>(size);

					for(int i=0; i<size; i++)
					{
						(result->Data)->get(i) = outBuffer[i];
					}
				}

				lame_close(lame);
				pAccess->Release();
				return result;
			});
		}
		else
		{
			pAccess->Release();
			throw ref new Platform::Exception(hr, L"Couldn't get bytes from the buffer");
		}
	}
	else
	{
		throw ref new Platform::Exception(hr, L"Couldn't access the buffer");
	}
};
//文件MP3解码
Windows::Foundation::IAsyncAction^ LameWrapper::EncodeMp3(Platform::String^ p_InPath,Platform::String^ p_OutPath)
{

	return Concurrency::create_async([=]
	{
		const char* inpath=PlatformStringToCharArray(p_InPath);
		const char* outpath=PlatformStringToCharArray(p_OutPath);
		int read = 0, 
			write = 0;
		long PCM_total_size = 0; 
		long cumulative_read = 0;
		FILE* pcm;
		FILE* mp3;


		//初始化文件
		fopen_s(&pcm,inpath,"rb");
		fopen_s(&mp3,outpath, "wb");

		if( pcm==NULL||mp3==NULL)
		{
			return;
		}

		fseek(pcm, 0, SEEK_SET);
		unsigned char  pcmheaddata[44];
		read = fread(pcmheaddata,  sizeof(unsigned char), 44, pcm);

		//初始化 编码器
		lame_global_flags* lame = GetEncodeLameFlags(pcmheaddata,this);



		lame_set_write_id3tag_automatic(lame, 0); //Dont write id3tag, will write it myself

		//读取文件 和 文件长度
		fseek(pcm, 0, SEEK_END);
		PCM_total_size = ftell(pcm);
		fseek(pcm, 0, SEEK_SET);


		short int pcm_buffer[PCM_SIZE*2];
		unsigned char mp3_buffer[MP3_SIZE];

		do
		{
			read = fread(pcm_buffer, 2 * sizeof(short int), PCM_SIZE, pcm);

			if(read == 0)
			{	
				write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
			}
			else
			{
				write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
			}
			fwrite(mp3_buffer, write, sizeof(char), mp3);

		}while(read != 0);


		fclose(mp3);
		fclose(pcm);

		lame_close(lame);
	});

}
