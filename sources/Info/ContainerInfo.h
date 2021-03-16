#pragma once

#include "ffmpeg.h"
#include "VideoStreamInfo.h"
#include "AudioStreamInfo.h"

#include <vector>

namespace ffmpegcpp
{
	struct ContainerInfo
	{
		int64_t durationInMicroSeconds;
		float durationInSeconds;
		float start;
		double bitRate;
		const AVInputFormat* format;

		std::vector<VideoStreamInfo> videoStreams;
		std::vector<AudioStreamInfo> audioStreams;
	};


}
