
#include <iostream>
#include "VideoSourceWithShaderInfo.h"
#include <ffmpegcpp.h>
#include <conio.h>

using namespace ffmpegcpp;
using namespace std;

int main(int argc, char **argv)
{
	string inputContainerName = "D:\\Downloads\\yd\\Video ok\\004ARCHITECT.mp4"; // container format is deduced from extension so use a known one
	string outputContainerName = "out.mp4"; // container format is deduced from extension so use a known one

	// you can use any filter string that you can use in the ffmpeg command-line here
	// set the filter to NULL to disable filtering.
	// See https://trac.ffmpeg.org/wiki/FilteringGuide for more info
	// This example rotates the entire video and then puts a vignette on top of it.
	//const char* videoFilterConfig = "transpose=cclock[middle];[middle]vignette";
	const char* videoFilterConfig = NULL;


	// create the different components that make this come together
	try
	{

		/**
			* CREATE THE OUTPUT CONTAINER
		*/

		// create the output muxer - we'll be adding encoders to it later
		Muxer* muxer = new Muxer(outputContainerName.c_str());

		/**
			* CONFIGURE AUDIO OUTPUT
		*/

		// create the output encoder based on our setting above
		AudioCodec* audioCodec = nullptr;
		printf("Encoding audio as AAC...\n");
		audioCodec = new AudioCodec(AV_CODEC_ID_AAC);

		// create an encoder - this encoder will receive raw data from any source (filter, file, container, memory, etc),
		// encode it and send it to the muxer (the output container).
		AudioEncoder* audioEncoder = nullptr;
		if (audioCodec != nullptr)
		{
			audioEncoder = new AudioEncoder(audioCodec, muxer);
		}

		/**
			* CONFIGURE VIDEO OUTPUT
		*/

		// create the output encoder based on our setting above
		VideoCodec* videoCodec = nullptr;
		printf("Encoding video as H265 on Nvidia GPU...\n");
		H265NVEncCodec* h265Codec = new H265NVEncCodec();
		h265Codec->SetPreset("hq");
		videoCodec = h265Codec;

		// create an encoder for the codec and tie it to the muxer
		// this encoder will receive data from an input source (file, raw, filter, etc), encode it and send it to the output container (muxer)
		VideoEncoder* videoEncoder = nullptr;
		if (videoCodec != nullptr)
		{
			videoEncoder = new VideoEncoder(videoCodec, muxer);
		}

		/**
			* CONFIGURE AUDIO INPUT
		*/

		// only do this when there is an output - otherwise there is no point in reading audio
		InputSource* audioInputSource = nullptr;
		if (audioEncoder != nullptr)
		{
			// if the input comes from a container, we use the demuxer class - it is just an input source like any other
			printf("Pulling audio from %s...\n", inputContainerName.c_str());
			Demuxer* demuxer = new Demuxer(inputContainerName.c_str());
			demuxer->DecodeBestAudioStream(audioEncoder);
			audioInputSource = demuxer;
		}

		/**
			* CONFIGURE VIDEO FILTER IF IT IS USED
		*/

		FrameSink* videoFrameSink = videoEncoder;

		// If a video filter was specified, we inject it into the pipeline here.
		// Instead of feeding the video source directly to the encoder, we feed it to
		// the video filter instead, which will pass it on to the encoder.
		Filter* videoFilter = nullptr;
		if (videoFilterConfig != NULL && videoEncoder != nullptr)
		{
			printf("Applying filter %s to video...\n", videoFilterConfig);
			videoFilter = new Filter(videoFilterConfig, videoEncoder);
			videoFrameSink = videoFilter; // used to feed the source below
		}

		/**
			* CONFIGURE VIDEO INPUT
		*/

		// only do this when there is video output
		InputSource* videoInputSource = nullptr;
		if (videoEncoder != nullptr)
		{
			printf("Pulling video from %s...\n", inputContainerName.c_str());
			videoInputSource = new VideoSourceWithShaderInfo(inputContainerName.c_str(),64, videoFrameSink);
		}


		/**
		* PROCESS THE DATA
		*/


		// As an awkward but necessary first step, we need to prime each input source.
		// This step decodes a part of the input data and from this, generates info
		// about the stream and propagates it all the way to the output container,
		// which needs to write this data to its header.
		if (videoInputSource != nullptr) videoInputSource->PreparePipeline();
		if (audioInputSource != nullptr) audioInputSource->PreparePipeline();



		// finally, we can start writing data to our pipelines. Open the floodgates
		// to start reading frames from the input, decoding them, optionally filtering them,
		// encoding them and writing them to the final container.
		// This can be interweaved if you want to.
		if (audioInputSource != nullptr)
		{
			while (!audioInputSource->IsDone()) audioInputSource->Step();
		}
		if (videoInputSource != nullptr)
		{
			while (!videoInputSource->IsDone()) videoInputSource->Step();
		}

		// close the muxer and save the file to disk
		muxer->Close();

		// all done
		if (audioCodec != nullptr)
		{
			delete audioCodec;
			delete audioEncoder;
		}
		if (videoCodec != nullptr)
		{
			delete videoCodec;
			delete videoEncoder;
			if (videoFilter != nullptr) delete videoFilter;
		}

		if (audioInputSource != nullptr)
		{
			delete audioInputSource;
		}

		if (videoInputSource != nullptr)
		{
			delete videoInputSource;
		}


		delete muxer;
	}
	catch (FFmpegException e)
	{
		cerr << e.what() << endl;
		throw e;
	}


	cout << "Encoding complete!" << endl;
	_getch();
	return 0;
}