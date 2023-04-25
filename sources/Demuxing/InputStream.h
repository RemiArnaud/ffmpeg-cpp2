#pragma once

#include "FrameSinks/FrameSink.h"
#include "Info/ContainerInfo.h"
#include "Demuxing/StreamData.h"

namespace ffmpegcpp
{
	class InputStream
	{

	public:
		InputStream();
		InputStream(AVFormatContext * format, AVStream * stream);
		~InputStream();

		void Init(AVFormatContext * format, AVStream * stream);

		void Open(FrameSink * frameSink);

		virtual void DecodePacket(AVPacket * pkt);
		void Close();

		bool IsPrimed();
		int GetFramesProcessed();

		virtual void WriteFrame(AVFrame * frame, StreamData * metadata)
		{
				(void)frame;
				(void)metadata;
		}

		virtual void AddStreamInfo(ContainerInfo * info) = 0;
		bool IsReady() { return m_bIsReady; }
		AVFrame * GetFrame()
		{
			return m_frame;
		}
		AVCodecContext * GetContext()
		{
			return m_codecContext;
		}
		StreamData * GetMetadata()
		{
			return m_metaData;
		}

	protected:

		AVCodecContext * m_codecContext = nullptr;


		virtual void ConfigureCodecContext();

		AVFormatContext * m_format;
		AVStream * m_stream;

		float CalculateBitRate(AVCodecContext * ctx);

	private:

		AVRational timeBaseCorrectedByTicksPerFrame;

		FrameSinkStream * m_output = nullptr;
		bool m_bIsReady;
		AVFrame * m_frame;

		StreamData * m_metaData = nullptr;

		StreamData * DiscoverMetaData();

		int m_nFramesProcessed = 0;

		void CleanUp();
	};
}
