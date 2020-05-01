#pragma once

#include "ffmpeg-cpp/ffmpeg.h"
#include "VideoStreamInfo.h"
#include "AudioStreamInfo.h"

#ifdef __linux__
#include <vector>
#endif

namespace ffmpegcpp
{
	struct ContainerInfo
	{
		long durationInMicroSeconds;
		float durationInSeconds;
		float start;
		float bitRate;
		const AVInputFormat* format;

		std::vector<VideoStreamInfo> videoStreams;
		std::vector<AudioStreamInfo> audioStreams;
	};


}
