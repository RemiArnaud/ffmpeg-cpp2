
/*
 * File webcam_stream_to_MPEG2.cpp
 * Copyright Eric Bachard / 2020 05 08
 * This document is under GPL v3 license
 * see : http://www.gnu.org/licenses/gpl-3.0.html
 */


#include <iostream>
#include <ffmpegcpp.h>
#include "video_device.h"

using namespace ffmpegcpp;
using namespace std;

void PlayDemo(int argc, char* argv[])
{
    (void)argc; (void)argv;
    avdevice_register_all();

    // These are example video and audio sources used below.
    const char videoDevice[] = VIDEO_DEVICE;

    Muxer* muxer = new Muxer("../videos/output_MPEG2.mpg");

    // Create a MPEG2 codec that will encode the raw data.
    VideoCodec * jcodec = new VideoCodec("mpeg2video");
    // parameter -qscale and must be within range [0,31].
    jcodec->SetQualityScale(23);
    // Set the bit rate option -b:v 2M
    jcodec->SetGenericOption("b", "2M");

    // create the different components that make this come together
    try
    {

        //AVPixelFormat pix_format = AV_PIX_FMT_YUVJ420P; // = V4L2_PIX_FMT_MJPEG
        int width  = 1280;
        int height = 720;
        int fps = 30;
        // FIXME : unprecise, and probably wrong somewhere
        AVRational frameRate = { 30, 1 };
        AVPixelFormat outputPixFormat= AV_PIX_FMT_YUV420P;

        VideoEncoder * videoEncoder = new VideoEncoder(jcodec, muxer, frameRate, outputPixFormat);

        Demuxer * demuxer = new Demuxer(videoDevice, width, height, fps);
        demuxer->setMaxFrameCount(30*5);
        demuxer->DecodeBestVideoStream(videoEncoder);
        demuxer->PreparePipeline();

        int frameCount = 0;
#if false
        int oldFrameCount = -30;
#endif
        while (!demuxer->IsDone() && (frameCount < 5*30))
        {
            frameCount = demuxer->GetFrameCount(demuxer->getVideoStreamIndx());
            //frameNumber += fn;
#if false
            if((frameCount - oldFrameCount) > 29) {
                std::cout << "frame count : " << frameCount << "\n";
                oldFrameCount = frameCount;
            }
#else
            std::cout << "frame count : " << frameCount << "\n";
#endif
            demuxer->Step();
        }
        demuxer->Stop();

        // close the muxer and save the file to disk
        muxer->Close();

        if (jcodec != nullptr)
            delete jcodec;

        if (videoEncoder != nullptr)
            delete videoEncoder;


        delete muxer;
    }
    catch (const FFmpegException & e)
    {
        cerr << e.what() << "\n";
        throw e;
    }
}

int main(int argc, char **argv)
{
	PlayDemo(argc, argv);

	std::cout << "Encoding complete!" << "\n";

	return 0;
}
