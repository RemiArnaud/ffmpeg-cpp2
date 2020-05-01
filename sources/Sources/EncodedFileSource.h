#pragma once

#include "ffmpeg.h"
#include "FrameSinks/FrameSink.h"
#include "InputSource.h"

namespace ffmpegcpp
{
	// EncodedFileSource takes a file that is already encoded but not in a container (ie .mp3, .h264)
	// and feeds it to the system.
	class EncodedFileSource : public InputSource
	{

	public:
		EncodedFileSource(const char* inFileName, AVCodecID codecId, FrameSink* output);
		EncodedFileSource(const char* inFileName, const char* codecName, FrameSink* output);
		virtual ~EncodedFileSource();

		virtual void PreparePipeline();
		virtual bool IsDone();
		virtual void Step();

	private:

		void CleanUp();

		bool done = false;

		FrameSinkStream* m_output;
		
		AVCodecParserContext* parser = nullptr;

		AVCodec* m_codec;
		AVCodecContext* codecContext = nullptr;

		int bufferSize;
                int refillThreshold;


		AVFrame* decoded_frame = nullptr;
		AVPacket* m_pkt = nullptr;
		uint8_t* buffer = nullptr;

		FILE* file;

		void Init(const char* inFileName, AVCodec* codec, FrameSink* output);

		void Decode(AVPacket *packet, AVFrame* targetFrame);

		AVRational timeBaseCorrectedByTicksPerFrame;

		StreamData* metaData = nullptr;
	};
}
