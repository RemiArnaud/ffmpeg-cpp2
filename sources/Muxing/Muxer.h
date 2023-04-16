#pragma once

#include "ffmpeg.h"
#include <vector>
#include <string>

namespace ffmpegcpp {

	class OutputStream;

	class Muxer
	{
	public:
		Muxer(const std::string & fileName);
		~Muxer();
		void AddOutputStream(OutputStream* stream);
		void WritePacket(AVPacket* pkt);
		void Close();
		bool IsPrimed();
        const AVCodec* GetDefaultVideoFormat();
        const AVCodec* GetDefaultAudioFormat();
	private:
        void Open();
        void CleanUp();
        std::vector<OutputStream*> m_outputStreams;
        std::vector<AVPacket*> m_packetQueue;
        const AVOutputFormat* m_containerFormat;
        AVFormatContext* m_containerContext = nullptr;
        std::string m_fileName;
        bool m_opened = false;
	};
}
