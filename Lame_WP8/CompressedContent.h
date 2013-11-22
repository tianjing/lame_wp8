#pragma once
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;


namespace Lame_WP8{

	public ref class CompressedContent sealed
	{
	public:
		CompressedContent(void);
		property Platform::Array<unsigned char>^ Data;
	};
}