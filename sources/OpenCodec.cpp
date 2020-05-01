#ifndef __linux__
#pragma once
#endif

#include "OpenCodec.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
	OpenCodec::OpenCodec(AVCodecContext* p_context)
	{
		if (!avcodec_is_open(p_context))
		{
			throw FFmpegException(std::string("Codec context for " + string(p_context->codec->name) + " hasn't been opened yet").c_str());
		}

		this->context = p_context;
	}

	OpenCodec::~OpenCodec()
	{
		avcodec_free_context(&context);
	}

	AVCodecContext* OpenCodec::GetContext()
	{
		return context;
	}
}
