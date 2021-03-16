#pragma once

#include "ffmpeg.h"
#include "FrameSinks/FrameSink.h"
#include "Info/ContainerInfo.h"
#include "Demuxing/StreamData.h"

namespace ffmpegcpp
{
	class InputStream
	{

	public:
		InputStream();
		InputStream(AVFormatContext* format, AVStream* stream);
		~InputStream();

		void Init(AVFormatContext* format, AVStream* stream);

		void Open(FrameSink* frameSink);

		virtual void DecodePacket(AVPacket* pkt);
		void Close();

		bool IsPrimed();
		int GetFramesProcessed();
		
		virtual void WriteFrame(AVFrame* frame, StreamData* metadata) {}

		virtual void AddStreamInfo(ContainerInfo* info) = 0;
		bool IsReady() { return m_bIsReady; }
		AVFrame*GetFrame() 
		{
			return frame;
		}
		AVCodecContext* GetContext()
		{
			return codecContext;
		}
		StreamData* GetMetadata()
		{
			return metaData;
		}

	protected:

		AVCodecContext* codecContext = nullptr;


		virtual void ConfigureCodecContext();

		AVFormatContext* format;
		AVStream* stream;

		float CalculateBitRate(AVCodecContext* ctx);

	private:

		AVRational timeBaseCorrectedByTicksPerFrame;

		FrameSinkStream* output = nullptr;
		bool m_bIsReady;
		AVFrame* frame;

		StreamData* metaData = nullptr;

		StreamData* DiscoverMetaData();

		int nFramesProcessed = 0;
		
		void CleanUp();

	};


}
