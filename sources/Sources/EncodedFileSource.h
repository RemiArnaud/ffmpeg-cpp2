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
        EncodedFileSource(const char * inFileName, AVCodecID codecId, FrameSink * output);
        EncodedFileSource(const char * inFileName, const char * codecName, FrameSink * output);
		virtual ~EncodedFileSource();

		virtual void PreparePipeline();
		virtual bool IsDone();
		virtual void Step();

	private:
        void Init(const std::string & inFileName, const AVCodec * codec, FrameSink * output);
        void Decode(AVPacket * packet, AVFrame * targetFrame);
        void CleanUp();

        bool m_done = false;

        FrameSinkStream * m_output;
		
        AVCodecParserContext * m_parser = nullptr;

        const AVCodec * m_codec;
        AVCodecContext * m_codecContext = nullptr;

        int bufferSize;
        int refillThreshold;


        AVFrame * m_decoded_frame = nullptr;
        AVPacket * m_pkt = nullptr;
        uint8_t * m_buffer = nullptr;
        FILE * m_file;
        AVRational m_timeBaseCorrectedByTicksPerFrame;
        StreamData * m_metaData = nullptr;
	};
}
