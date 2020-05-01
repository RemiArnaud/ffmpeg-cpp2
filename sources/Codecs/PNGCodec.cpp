#include "PNGCodec.h"

namespace ffmpegcpp
{

	PNGCodec::PNGCodec()
		: VideoCodec(AV_CODEC_ID_PNG)
	{

		// we take the default image format of the codec
		m_codecContext->pix_fmt = m_codecContext->codec->pix_fmts[0];
	}

	void PNGCodec::SetCompressionLevel(int compressionLevel)
	{
		SetOption("compression_level", compressionLevel);
	}
}