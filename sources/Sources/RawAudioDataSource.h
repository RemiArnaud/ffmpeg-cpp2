#pragma once

#include "ffmpeg.h"
#include "FrameSinks/AudioFrameSink.h"

namespace ffmpegcpp
{
    /* RawVideoDataSource feeds raw memory to the system for processing.
     * You can use this if the video data comes from a source other than the
     * file system (ie rendering).
    */
    class RawAudioDataSource
	{
	public:
        RawAudioDataSource(AVSampleFormat sampleFormat
                    , int sampleRate, int channels, FrameSink * output);
        RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate
                    , int channels, int64_t channelLayout, FrameSink * output);
		virtual ~RawAudioDataSource();
        void WriteData(void * data, int sampleCount);
		void Close();
		bool IsPrimed();
	private:
		void CleanUp();
        FrameSinkStream * m_output;
        AVFrame * m_frame = nullptr;
        StreamData * m_metaData = nullptr;
	};
}
