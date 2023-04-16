#include "OpenCodec.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
    OpenCodec::OpenCodec(AVCodecContext * context)
	{
		if (!avcodec_is_open(context))
		{
            throw FFmpegException(string("Codec context for " + string(context->codec->name) + " hasn't been opened yet").c_str());
		}

        m_context = context;
	}

	OpenCodec::~OpenCodec()
	{
        avcodec_free_context(& m_context);
	}

	AVCodecContext* OpenCodec::GetContext()
	{
        return m_context;
	}
}
