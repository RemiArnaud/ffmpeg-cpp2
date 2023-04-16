#pragma once

#include "ffmpeg.h"

#include "InputSource.h"
#include "Demuxer.h"


namespace ffmpegcpp
{
	class RawVideoFileSource : public InputSource
	{
	public:
        RawVideoFileSource(const char * fileName, FrameSink * a_frameSink);
        RawVideoFileSource(const char * fileName
            , int d_width, int d_height, int d_framerate);
        /* I couldn't get this to work. The thing is that it also crashes
         * weirdly when I run ffmpeg directly,
         * so I think it's more an issue of ffmpeg than one of my library.
        * RawVideoFileSource(const char* fileName, int width, int height
        * , const char* frameRate, AVPixelFormat format
        * , VideoFrameSink * frameSink);
        */
		virtual ~RawVideoFileSource();
		virtual void PreparePipeline();
		virtual bool IsDone();
        virtual void Step();
        Demuxer * m_demuxer;
        void setFrameSink(FrameSink * aframeSink);
    private:
        void CleanUp();
        AVFormatContext * m_pAVFormatContextIn;
//		AVDictionary * options;
//		enum AVPixelFormat m_format;
//		AVCodec * pAVCodec;
//		AVInputFormat * inputFormat;
        int m_width;
        int m_height;
//		int m_framerate;
        FrameSink * m_frameSink;
	};
}
