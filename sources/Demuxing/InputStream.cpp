#include "InputStream.h"
#include "CodecDeducer.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
	InputStream::InputStream()
	{
	}
	InputStream::InputStream(AVFormatContext * p_format, AVStream * p_stream)
	{
		m_stream = p_stream;
		m_format = p_format;

		// find decoder for the stream
		const AVCodec* codec = CodecDeducer::DeduceDecoder(m_stream->codecpar->codec_id);
		if (!codec)
		{
			CleanUp();
			throw FFmpegException(std::string("Failed to find codec for stream " + to_string(m_stream->index)).c_str());
		}

		// Allocate a codec context for the decoder
		m_codecContext = avcodec_alloc_context3(codec);
		if (!m_codecContext)
		{
			CleanUp();
			throw FFmpegException(std::string("Failed to allocate the codec context for " + string(codec->name)).c_str());
		}

		m_codecContext->framerate = m_stream->avg_frame_rate;

		// Copy codec parameters from input stream to output codec context
		int ret;
        if ((ret = avcodec_parameters_to_context(m_codecContext, m_stream->codecpar)) < 0)
		{
			throw FFmpegException(std::string("Failed to copy " + string(codec->name) + " codec parameters to decoder context").c_str(), ret);
		}

		// before we open it, we let our subclasses configure the codec context as well
		ConfigureCodecContext();

		// Init the decoders
        if ((ret = avcodec_open2(m_codecContext, codec, NULL)) < 0)
		{
			throw FFmpegException(std::string("Failed to open codec " + string(codec->name)).c_str(), ret);
		}

		// calculate the "correct" time_base
		// TODO this is definitely an ugly hack but right now I have no idea on how to fix this properly.
		timeBaseCorrectedByTicksPerFrame.num = m_codecContext->time_base.num;
		timeBaseCorrectedByTicksPerFrame.den = m_codecContext->time_base.den;
		timeBaseCorrectedByTicksPerFrame.num *= m_codecContext->ticks_per_frame;

		// assign the frame that will be read from the container
		m_frame = av_frame_alloc();
		if (!m_frame)
		{
			throw FFmpegException(std::string("Could not allocate frame").c_str());
		}
    }
    void InputStream::Init(AVFormatContext* p_format, AVStream* p_stream)
    {
        m_stream = p_stream;
        m_format = p_format;

        // find decoder for the stream
        const AVCodec* codec = CodecDeducer::DeduceDecoder(m_stream->codecpar->codec_id);
        if (!codec)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to find codec for stream " + to_string(m_stream->index)).c_str());
        }

        // Allocate a codec context for the decoder
        m_codecContext = avcodec_alloc_context3(codec);
        if (!m_codecContext)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to allocate the codec context for " + string(codec->name)).c_str());
        }

        m_codecContext->framerate = m_stream->avg_frame_rate;

        // Copy codec parameters from input stream to output codec context
        int ret;
        if ((ret = avcodec_parameters_to_context(m_codecContext, m_stream->codecpar)) < 0)
        {
            throw FFmpegException(std::string("Failed to copy " + string(codec->name) + " codec parameters to decoder context").c_str(), ret);
        }

        // before we open it, we let our subclasses configure the codec context as well
        ConfigureCodecContext();

        // Init the decoders
        if ((ret = avcodec_open2(m_codecContext, codec, NULL)) < 0)
        {
            throw FFmpegException(std::string("Failed to open codec " + string(codec->name)).c_str(), ret);
        }

        // calculate the "correct" time_base
        // TODO this is definitely an ugly hack but right now I have no idea on how to fix this properly.
        timeBaseCorrectedByTicksPerFrame.num = m_codecContext->time_base.num;
        timeBaseCorrectedByTicksPerFrame.den = m_codecContext->time_base.den;
        timeBaseCorrectedByTicksPerFrame.num *= m_codecContext->ticks_per_frame;

        // assign the frame that will be read from the container
        m_frame = av_frame_alloc();
        if (!m_frame)
        {
            throw FFmpegException(std::string("Could not allocate frame").c_str());
        }
    }

	InputStream::~InputStream()
	{
		CleanUp();
	}

	void InputStream::ConfigureCodecContext()
	{
		// does nothing by default
	}

	void InputStream::Open(FrameSink* frameSink)
	{
        if(frameSink)
            m_output = frameSink->CreateStream();
	}

	void InputStream::CleanUp()
	{
		if (m_codecContext != nullptr)
		{
			avcodec_free_context(&m_codecContext);
			m_codecContext = nullptr;
		}
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

	StreamData* InputStream::DiscoverMetaData()
	{
		/*metaData = new StreamData();

		AVRational* time_base = &timeBaseCorrectedByTicksPerFrame;
		if (!timeBaseCorrectedByTicksPerFrame.num)
		{
			time_base = &stream->time_base;
		}

		metaData->timeBase.num = time_base->num;
		metaData->timeBase.den = time_base->den;*/

		AVRational overrideFrameRate;
		overrideFrameRate.num = 0;

		AVRational tb = overrideFrameRate.num ? av_inv_q(overrideFrameRate) : m_stream->time_base;
		AVRational fr = overrideFrameRate;
		if (!fr.num) fr = av_guess_frame_rate(m_format, m_stream, NULL);

		StreamData* metaData = new StreamData();
		metaData->timeBase = tb;
		metaData->frameRate = fr;

		metaData->type = m_codecContext->codec->type;

		return metaData;
	}

	void InputStream::DecodePacket(AVPacket *pkt)
	{
		int ret;

		/* send the packet with the compressed data to the decoder */
		ret = avcodec_send_packet(m_codecContext, pkt);
		if (ret < 0)
		{
			throw FFmpegException(std::string("Error submitting the packet to the decoder").c_str(), ret);
		}
        m_bIsReady = false;

		/* read all the output frames (in general there may be any number of them */
		while (ret >= 0)
		{
			ret = avcodec_receive_frame(m_codecContext, m_frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
			{
				throw FFmpegException(std::string("Error during decoding").c_str(), ret);
            }
            m_bIsReady = true;

			// put default settings from the stream into the frame
			if (!m_frame->sample_aspect_ratio.num)
			{
				m_frame->sample_aspect_ratio = m_stream->sample_aspect_ratio;
			}

			// the meta data does not exist yet - we figure it out!
			if (m_metaData == nullptr)
			{
				m_metaData = DiscoverMetaData();
			}

			// push the frame to the next stage.
			// The time_base is filled in in the codecContext after the first frame is decoded
			// so we can fetch it from there.
			if (m_output == nullptr)
			{
				// No frame sink specified - just release the frame again.
                WriteFrame(m_frame, m_metaData);
			}
			else
			{
				m_output->WriteFrame(m_frame, m_metaData);
			}
			++m_nFramesProcessed;
		}
	}

	int InputStream::GetFramesProcessed()
	{
		return m_nFramesProcessed;
	}

	void InputStream::Close()
	{
		if (m_output != nullptr) m_output->Close();
	}

	bool InputStream::IsPrimed()
    {
        if (m_output)
            return m_output->IsPrimed();
        else
            return true;
    }

	float InputStream::CalculateBitRate(AVCodecContext* ctx)
	{
		int64_t bit_rate;
		int bits_per_sample;

		switch (ctx->codec_type)
		{
		case AVMEDIA_TYPE_VIDEO:
		case AVMEDIA_TYPE_DATA:
		case AVMEDIA_TYPE_SUBTITLE:
		case AVMEDIA_TYPE_ATTACHMENT:
			bit_rate = ctx->bit_rate;
			break;
		case AVMEDIA_TYPE_AUDIO:
			bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
			bit_rate = bits_per_sample ? ctx->sample_rate * (int64_t)ctx->channels * bits_per_sample : ctx->bit_rate;
			break;
		default:
			bit_rate = 0;
			break;
		}
		return bit_rate / 1000.0f;
	}
}

