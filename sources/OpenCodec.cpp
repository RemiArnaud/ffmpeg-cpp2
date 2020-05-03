#ifndef __linux__
#pragma once
#endif

#include "OpenCodec.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
	OpenCodec::OpenCodec(AVCodecContext* context)
	{
		if (!avcodec_is_open(context))
		{
			throw FFmpegException(std::string("Codec context for " + string(context->codec->name) + " hasn't been opened yet").c_str());
		}

		this->context = context;
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
