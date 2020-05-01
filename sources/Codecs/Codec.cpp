#include "Codec.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"
#include <iostream>

using namespace std;

namespace ffmpegcpp
{
	Codec::Codec(const char* codecName)
	{
		
		AVCodec* codec = CodecDeducer::DeduceEncoder(codecName);
		m_codecContext = LoadContext(codec);
	}


	Codec::Codec(AVCodecID codecId)
	{
		AVCodec* codec = CodecDeducer::DeduceEncoder(codecId);
		m_codecContext = LoadContext(codec);
	}

	void Codec::SetOption(const char* name, const char* value)
	{
		av_opt_set(m_codecContext->priv_data, name, value, 0);
	}

	void Codec::SetOption(const char* name, int value)
	{
		av_opt_set_int(m_codecContext->priv_data, name, value, 0);
	}

	void Codec::SetOption(const char* name, double value)
	{
		av_opt_set_double(m_codecContext->priv_data, name, value, 0);
	}

	void Codec::SetGenericOption(const char* name, const char* value)
	{
		av_opt_set(m_codecContext, name, value, 0);
	}

	AVCodecContext* Codec::LoadContext(AVCodec* p_codec)
	{
		m_codecContext = avcodec_alloc_context3(p_codec);
		if (!m_codecContext)
		{
			CleanUp();
			throw FFmpegException(std::string("Could not allocate video codec context for codec " + string(p_codec->name)).c_str());
		}

		// copy the type
		m_codecContext->codec_type = p_codec->type;

		return m_codecContext;
	}

	void Codec::CleanUp()
	{
		if (m_codecContext != nullptr && !opened)
		{
			avcodec_free_context(&m_codecContext);
		}
	}

	OpenCodec* Codec::Open()
	{
		if (opened)
		{
			throw FFmpegException(std::string("You can only open a codec once").c_str());
		}

		int ret = avcodec_open2(m_codecContext, m_codecContext->codec, NULL);
		if (ret < 0)
		{
			throw FFmpegException(std::string("Could not open codecContext for codec").c_str(), ret);
		}

		opened = true;
#ifdef DEBUG
                std::cerr << "codec Context is open (ouf)"  << "\n";
#endif

		return new OpenCodec(m_codecContext);
	}

	Codec::~Codec()
	{
		CleanUp();
	}

	void Codec::SetGlobalContainerHeader()
	{
		if (opened) throw FFmpegException("This flag should be set before opening the codec");
		m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
}
