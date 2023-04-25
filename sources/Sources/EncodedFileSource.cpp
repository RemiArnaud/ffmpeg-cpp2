#include "EncodedFileSource.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"

using namespace std;

namespace ffmpegcpp
{
    EncodedFileSource::EncodedFileSource(const char * inFileName, const char * codecName, FrameSink* output)
	{
		try
		{
            const AVCodec* codec = CodecDeducer::DeduceDecoder(codecName);
			Init(inFileName, codec, output);
		}
        catch (const FFmpegException & e)
		{
			CleanUp();
			throw e;
		}
	}

    EncodedFileSource::EncodedFileSource(const char * inFileName, AVCodecID codecId, FrameSink* output)
	{
		try
		{
            const AVCodec* codec = CodecDeducer::DeduceDecoder(codecId);
			Init(inFileName, codec, output);
        }
        catch (const FFmpegException & e)
        {
			CleanUp();
			throw e;
		}
	}

	EncodedFileSource::~EncodedFileSource()
	{
		CleanUp();
	}

	void EncodedFileSource::CleanUp()
	{
        if (m_decoded_frame != nullptr)
		{
            av_frame_free(&m_decoded_frame);
            m_decoded_frame = nullptr;
		}
        if (m_pkt != nullptr)
		{
            av_packet_free(&m_pkt);
            m_pkt = nullptr;
		}
        if (m_buffer != nullptr)
		{
            delete m_buffer;
            m_buffer = nullptr;
		}
        if (m_codecContext != nullptr)
		{
            avcodec_free_context(&m_codecContext);
            m_codecContext = nullptr;
		}
        if (m_parser != nullptr)
		{
            av_parser_close(m_parser);
            m_parser = nullptr;
		}
        if (m_metaData != nullptr)
		{
            delete m_metaData;
            m_metaData = nullptr;
		}

        fclose(m_file);
	}

    void EncodedFileSource::Init(const std::string & inFileName, const AVCodec* codec, FrameSink* output)
	{
        m_output = output->CreateStream();
        m_codec = codec;

        m_parser = av_parser_init(codec->id);
        if (!m_parser)
		{
			throw FFmpegException(std::string("Parser for codec not found " + string(codec->name)).c_str());
		}

        m_codecContext = avcodec_alloc_context3(codec);
        if (!m_codecContext)
		{
			throw FFmpegException(std::string("Failed to allocate context for codec " + string(codec->name)).c_str());
		}

		/* open it */
        if (int ret = avcodec_open2(m_codecContext, codec, NULL) < 0)
		{
			throw FFmpegException(std::string("Failed to open context for codec " + string(codec->name)).c_str(), ret);
		}

        m_file = fopen(inFileName.c_str(), "rb");
        if (!m_file)
		{
			throw FFmpegException(std::string("Could not open file " + string(inFileName)).c_str());
		}

        m_decoded_frame = av_frame_alloc();
        if (!m_decoded_frame)
		{
			throw FFmpegException(std::string("Could not allocate video frame").c_str());
		}

        m_pkt = av_packet_alloc();
        if (!m_pkt)
		{
			throw FFmpegException(std::string("Failed to allocate packet").c_str());
		}

		// based on the codec, we use different buffer sizes
        if (m_codecContext->codec->type == AVMEDIA_TYPE_VIDEO)
		{
			bufferSize = 4096;
			refillThreshold = 0;
		}
        else if (m_codecContext->codec->type == AVMEDIA_TYPE_AUDIO)
		{
			bufferSize = 20480;
			refillThreshold = 4096;
		}
		else
		{
            throw FFmpegException(std::string("Codec " + string(m_codecContext->codec->name) + " is not supported as a RawFileSource").c_str());
		}

        m_buffer = new uint8_t[bufferSize + AV_INPUT_BUFFER_PADDING_SIZE];

		/* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
        memset(m_buffer + (int)bufferSize, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	}

	void EncodedFileSource::PreparePipeline()
	{
        while (!m_output->IsPrimed() && !IsDone())
		{
			Step();
		}
	}

	bool EncodedFileSource::IsDone()
	{
        return m_done;
	}

	void EncodedFileSource::Step()
	{
		// one step is one part of a buffer read, this might contain no, one or multiple packets

        uint8_t * data;
		size_t   data_size;
		int ret;

		/* read raw data from the input file */
        data_size = fread(m_buffer, 1, bufferSize, m_file);
		if (!data_size)	return;

		/* use the parser to split the data into frames */
        data = m_buffer;
		while (data_size > 0)
		{
            ret = av_parser_parse2(m_parser, m_codecContext, &m_pkt->data, &m_pkt->size,
                data, (int)data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (ret < 0)
			{
				throw FFmpegException(std::string("Error while parsing file").c_str(), ret);
			}
			data += ret;
			data_size -= ret;

            if (m_pkt->size)
			{
                Decode(m_pkt, m_decoded_frame);
			}
		}

		// reached the end of the file - flush everything
        if (feof(m_file))
		{

			/* flush the decoder */
            m_pkt->data = NULL;
            m_pkt->size = 0;
            Decode(m_pkt, m_decoded_frame);

            m_output->Close();

            m_done = true;
		}
	}

	void EncodedFileSource::Decode(AVPacket *pkt, AVFrame *frame)
	{
		int ret;

		/* send the packet with the compressed data to the decoder */
        ret = avcodec_send_packet(m_codecContext, pkt);
		if (ret < 0)
		{
			throw FFmpegException(std::string("Error submitting the packet to the decoder").c_str(), ret);
		}

		/* read all the output frames (in general there may be any number of them */
		while (ret >= 0)
		{
            ret = avcodec_receive_frame(m_codecContext, frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return;
			else if (ret < 0)
			{
				throw FFmpegException(std::string("Error during decoding").c_str(), ret);
			}

            if (m_metaData == nullptr)
			{
				// calculate the "correct" time_base
				// TODO this is definitely an ugly hack but right now I have no idea on how to fix this properly.
                m_timeBaseCorrectedByTicksPerFrame.num = m_codecContext->time_base.num;
                m_timeBaseCorrectedByTicksPerFrame.den = m_codecContext->time_base.den;
                m_timeBaseCorrectedByTicksPerFrame.num *= m_codecContext->ticks_per_frame;


                m_metaData = new StreamData();
                m_metaData->timeBase.num = m_timeBaseCorrectedByTicksPerFrame.num;
                m_metaData->timeBase.den = m_timeBaseCorrectedByTicksPerFrame.den;
                m_metaData->frameRate.den = m_timeBaseCorrectedByTicksPerFrame.num;
                m_metaData->frameRate.num = m_timeBaseCorrectedByTicksPerFrame.den;

                m_metaData->type = m_codecContext->codec->type;
			}


			// push the frame to the next stage.
			// The time_base is filled in in the codecContext after the first frame is decoded
			// so we can fetch it from there.
            m_output->WriteFrame(frame, m_metaData);
		}
	}
}
