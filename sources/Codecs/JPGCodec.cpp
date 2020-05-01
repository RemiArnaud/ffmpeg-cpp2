#include "JPGCodec.h"

namespace ffmpegcpp
{

	JPGCodec::JPGCodec()
		: VideoCodec(AV_CODEC_ID_MPEG2VIDEO)
	{

		// we take the default image format of the codec
		m_codecContext->pix_fmt = m_codecContext->codec->pix_fmts[0];
	}

	void JPGCodec::SetCompressionLevel(int compressionLevel)
	{
		SetOption("compression_level", compressionLevel);
	}
}