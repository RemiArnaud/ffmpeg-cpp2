#include "VideoFormatConverter.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{
    VideoFormatConverter::VideoFormatConverter(AVCodecContext * codecContextIn)
	{
        m_codecContext = codecContextIn;
        m_converted_frame = av_frame_alloc();
        int ret;
        if (!m_converted_frame)
		{
			CleanUp();
			throw FFmpegException("Error allocating a video frame");
		}

		// configure the frame and get the buffer
        m_converted_frame->format = codecContextIn->pix_fmt;
        m_converted_frame->width = codecContextIn->width;
        m_converted_frame->height = codecContextIn->height;

        /* allocate the buffers for the frame data */
        ret = av_frame_get_buffer(m_converted_frame, 32);
		if (ret < 0)
		{
			CleanUp();
			throw FFmpegException("Failed to allocate buffer for frame", ret);
		}
	}

	VideoFormatConverter::~VideoFormatConverter()
	{
		CleanUp();
	}

	void VideoFormatConverter::CleanUp()
    {
        if (m_converted_frame != nullptr)
        {
            av_frame_free(&m_converted_frame);
            m_converted_frame = nullptr;
        }
        if (m_swsContext != nullptr)
        {
            sws_freeContext(m_swsContext);
            m_swsContext = nullptr;
		}
	}

    void VideoFormatConverter::InitDelayed(AVFrame * frame)
	{
        // configure the conversion context based in the source and target data
        m_swsContext = sws_getCachedContext(m_swsContext
            , frame->width, frame->height, (AVPixelFormat)frame->format
            , m_converted_frame->width, m_converted_frame->height
            , (AVPixelFormat)m_converted_frame->format
            , 0, 0, 0, 0);
	}

    AVFrame * VideoFormatConverter::ConvertFrame(AVFrame * frame)
	{
        // initialize the resampler
        if (!m_initialized)
		{
            InitDelayed(frame);
            m_initialized = true;
		}

		// convert the frame
        sws_scale(m_swsContext, frame->data, frame->linesize, 0,
                  frame->height, m_converted_frame->data, m_converted_frame->linesize);

        av_frame_copy_props(m_converted_frame, frame); // remember all the other data

        return m_converted_frame;
	}
}

