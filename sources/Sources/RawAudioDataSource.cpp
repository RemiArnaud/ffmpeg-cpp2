#include "RawAudioDataSource.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{

	RawAudioDataSource::RawAudioDataSource(AVSampleFormat sampleFormat, int sampleRate, int channels, FrameSink* output)
    : RawAudioDataSource(sampleFormat, sampleRate, channels, av_get_default_channel_layout(channels), output)
	{
	}

    RawAudioDataSource::RawAudioDataSource(AVSampleFormat sampleFormat
        , int sampleRate, int channels, int64_t channelLayout, FrameSink* output)
	{
        m_output = output->CreateStream();

		// create the frame
		int ret;
        
        m_frame = av_frame_alloc();
        if (!m_frame)
		{
			CleanUp();
			throw FFmpegException("Could not allocate video frame");
		}
        
        m_frame->format = sampleFormat;
        m_frame->sample_rate = sampleRate;
        m_frame->channels = channels;
        m_frame->channel_layout = channelLayout;
        m_frame->nb_samples = 735;

		// allocate the buffers for the frame data
        ret = av_frame_get_buffer(m_frame, 0);
		if (ret < 0)
		{
			CleanUp();
			throw FFmpegException("Could not allocate the video frame data", ret);
		}

	}

	RawAudioDataSource::~RawAudioDataSource()
	{
		CleanUp();
	}

	void RawAudioDataSource::CleanUp()
	{
        if (m_frame != nullptr)
		{
            av_frame_free(&m_frame);
            m_frame = nullptr;
		}
        if (m_metaData != nullptr)
		{
            delete m_metaData;
            m_metaData = nullptr;
		}
	}

	void RawAudioDataSource::WriteData(void* data, int sampleCount)
	{
		// resize the frame to the input
        m_frame->nb_samples = sampleCount;
        
        int ret = av_frame_make_writable(m_frame);
		if (ret < 0)
		{
			throw FFmpegException("Failed to make audio frame writable", ret);
		}

		// copy the data to the frame buffer
        int bytesPerSample = av_get_bytes_per_sample((AVSampleFormat)m_frame->format);
        memcpy(*m_frame->data, data, m_frame->nb_samples * m_frame->channels * bytesPerSample);

		// fill in the meta data
        if (m_metaData == nullptr)
		{
            m_metaData = new StreamData();
            m_metaData->type = AVMEDIA_TYPE_AUDIO;
		}

		// pass on to the sink
		// we don't have a time_base so we pass NULL and hope that it gets handled later...
        m_output->WriteFrame(m_frame, m_metaData);
	}

	void RawAudioDataSource::Close()
	{
        m_output->Close();
	}

	bool RawAudioDataSource::IsPrimed()
	{
        return m_output->IsPrimed();
	}
}
