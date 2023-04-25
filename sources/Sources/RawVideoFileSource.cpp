
#include "RawVideoFileSource.h"
#include "FFmpegException.h"
#include <iostream>

#include <string>

using namespace std;

namespace ffmpegcpp
{
    RawVideoFileSource::RawVideoFileSource(const char * fileName
        , FrameSink * p_frameSink)
    {
        setFrameSink(p_frameSink);
        // create the demuxer - it can handle figuring out the video type on its own apparently
        try
        {
            m_demuxer = new Demuxer(fileName, NULL, NULL);
            m_demuxer->DecodeBestVideoStream(m_frameSink);

        }
        catch (FFmpegException const & e)
        {
            CleanUp();
            throw e;
        }
    }
    // only testng on Linux for the moment, but Windows should work. MacOS X = don't care
#ifdef __linux__

    RawVideoFileSource::RawVideoFileSource(const char * fileName
        , int d_width, int d_height, int d_framerate)
    {
        // mandatory under Linux
        avdevice_register_all();

        try
        {
            m_demuxer = new Demuxer(fileName, d_width, d_height, d_framerate);
        }

        catch (const FFmpegException & e)
        {
            CleanUp();
            throw e;
        }
    }

#endif  /*  __linux__ */

        // Doesn't work for now. See the header for more info.
        /*RawVideoFileSource::RawVideoFileSource(const char* fileName, int width, int height, const char* frameRate, AVPixelFormat format, VideoFrameSink* frameSink)
        {
            // try to deduce the input format from the input format name
            AVInputFormat *file_iformat;
            if (!(file_iformat = av_find_input_format("yuv4mpegpipe")))
            {
                CleanUp();
                throw FFmpegException("Unknown input format 'rawvideo'");
            }

            AVDictionary* format_opts = NULL;

            // only set the frame rate if the format allows it!
            if (file_iformat && file_iformat->priv_class &&	av_opt_find(&file_iformat->priv_class, "framerate", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ))
            {
                av_dict_set(&format_opts, "framerate", frameRate, 0);
            }
            char videoSize[200];
            sprintf(videoSize, "%dx%d", width, height);
            av_dict_set(&format_opts, "video_size", videoSize, 0);
            const char* pixelFormatName = av_get_pix_fmt_name(format);
            av_dict_set(&format_opts, "pixel_format", pixelFormatName, 0);

            // create the demuxer
            try
            {
                demuxer = new Demuxer(fileName, file_iformat, format_opts);
                demuxer->DecodeBestVideoStream(frameSink);
            }
            catch (FFmpegException e)
            {
                CleanUp();
                throw e;
            }
        }*/

    RawVideoFileSource::~RawVideoFileSource()
    {
        CleanUp();
    }

    void RawVideoFileSource::CleanUp()
    {
        if (m_demuxer != nullptr)
        {
            avformat_close_input(&m_pAVFormatContextIn);
            avformat_free_context(m_pAVFormatContextIn);

            delete m_demuxer;
            m_demuxer = nullptr;
        }
    }

    void RawVideoFileSource::PreparePipeline()
    {
        m_demuxer->PreparePipeline();
    }

    bool RawVideoFileSource::IsDone()
    {
        return m_demuxer->IsDone();
    }

    void RawVideoFileSource::Step()
    {
        m_demuxer->Step();
    }

    void RawVideoFileSource::setFrameSink(FrameSink * aFrameSink)
    {
        m_frameSink = aFrameSink;
    }
}


