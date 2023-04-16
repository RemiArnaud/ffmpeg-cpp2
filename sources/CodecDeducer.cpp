#include "CodecDeducer.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
	const AVCodec* CodecDeducer::DeduceEncoder(const char * codecName)
	{
        const AVCodec* codec = avcodec_find_encoder_by_name(codecName);
		if (!codec)
		{
			throw FFmpegException(std::string("Codec " + string(codecName) + " not found").c_str());
		}
		return codec;
	}

	const AVCodec* CodecDeducer::DeduceEncoder(AVCodecID codecId)
	{
		const AVCodec* codec = avcodec_find_encoder(codecId);
		if (!codec)
		{
			throw FFmpegException(std::string("Codec with id " + to_string((int)codecId) + " not found").c_str());
		}
		return codec;
	}

	const AVCodec* CodecDeducer::DeduceDecoder(const char* codecName)
	{
		const AVCodec* codec = avcodec_find_decoder_by_name(codecName);
		if (!codec)
		{
			throw FFmpegException(std::string("Codec " + string(codecName) + " not found").c_str());
		}
		return codec;
	}

	const AVCodec* CodecDeducer::DeduceDecoder(AVCodecID codecId)
	{
		if (codecId == AV_CODEC_ID_NONE) return nullptr;
		const AVCodec* codec = avcodec_find_decoder(codecId);
		if (!codec)
		{
			throw FFmpegException(std::string("Codec with id " + to_string((int)codecId) + " not found").c_str());
		}
		return codec;
	}

	const AVCodec* CodecDeducer::DeduceEncoderFromFilename(const char* /* fileName */)
	{
		throw FFmpegException(std::string("Not implemented yet").c_str());
	}
}
