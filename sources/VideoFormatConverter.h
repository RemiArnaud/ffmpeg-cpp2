#pragma once

#include "ffmpeg.h"
#include "ConvertedAudioProcessor.h"

namespace ffmpegcpp
{
	class VideoFormatConverter
	{
	public:
        VideoFormatConverter(AVCodecContext * codecContext);
		~VideoFormatConverter();
        AVFrame * ConvertFrame(AVFrame * frame);
	private:
		void CleanUp();
        void InitDelayed(AVFrame * frame);
        AVCodecContext * m_codecContext;
        bool m_initialized = false;
        AVFrame * m_converted_frame = nullptr;
        struct SwsContext * m_swsContext = nullptr;
	};
}
