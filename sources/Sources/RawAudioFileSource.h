#pragma once

#include "ffmpeg.h"

#include "InputSource.h"
#include "Demuxer.h"

namespace ffmpegcpp
{
	class RawAudioFileSource : public InputSource
	{
	public:
        RawAudioFileSource(const char * fileName, const char * inputFormat
            , int sampleRate, int channels, FrameSink * frameSink);
		virtual ~RawAudioFileSource();
		virtual void PreparePipeline();
		virtual bool IsDone();
		virtual void Step();
        void Stop() { m_demuxer->Stop(); }
	private:
        void CleanUp();
        Demuxer * m_demuxer = nullptr;
	};
}
