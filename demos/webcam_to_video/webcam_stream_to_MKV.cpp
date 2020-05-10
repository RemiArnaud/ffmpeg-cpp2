
/*
 * File webcam_stream_to_MKV.cpp
 * Copyright Eric Bachard / 2020 05 08
 * This document is under GPL v3 license
 * see : http://www.gnu.org/licenses/gpl-3.0.html
 */

#include <iostream>

#include <ffmpegcpp.h>

using namespace ffmpegcpp;
using std::string;
using std::cerr;

void record_MKV()
{
    avdevice_register_all();

    // These are example video and audio sources used below.
    const char* videoDevice = "/dev/video0";

    //  OUTPUT CODEC, linked to the encoder ...
    H264Codec  * vcodec = new H264Codec();
    vcodec->SetGenericOption("b", "2M");
    vcodec->SetGenericOption("bit_rate", "2M");
    vcodec->SetGenericOption("movflags", "+faststart");
    vcodec->SetGenericOption("qmin", "12");
    vcodec->SetGenericOption("qmax", "34");

    // FIXME : needs more tests
    //vcodec->SetGenericOption("vbv-maxrate", "4M");
    //vcodec->SetGenericOption("vbv-bufsize", "2M");

    vcodec->SetProfile("high10"); // baseline, main, high, high10, high422
    vcodec->SetTune("film");  // film animation grain stillimage psnr ssim fastdecode zerolatency
    vcodec->SetPreset("veryslow"); // fast, medium, slow slower, veryslow placebo

    vcodec->SetCrf(23);

    // Create a muxer that will output the video as MKV.
    Muxer* muxer = new Muxer("../videos/output_H264.mkv");

    // create the different components that make this come together
    try
    {

        int width  = 1280;
        int height = 720;
        int fps = 24;

        // FIXME : timing is not precise, and probably wrong
        AVRational frameRate = { 24, 1 };

        // All seem to work
        //AVPixelFormat input_pix_fmt= AV_PIX_FMT_NV12;
        //AVPixelFormat outputPixFormat= AV_PIX_FMT_RGBA;
        AVPixelFormat outputPixFormat= AV_PIX_FMT_YUV420P;

        VideoEncoder * videoEncoder = new VideoEncoder(vcodec, muxer, frameRate, outputPixFormat);

        Demuxer * demuxer = new Demuxer(videoDevice, width, height, fps);
        demuxer->DecodeBestVideoStream(videoEncoder);
        demuxer->PreparePipeline();

        int frameNumber = 0;

        while (!demuxer->IsDone() && (frameNumber < 100))
        {
            frameNumber = demuxer->GetFrameCount(demuxer->getVideoStreamIndx());
            std::cerr << "frame number : " << frameNumber << "\n";

            if (frameNumber > 100)
                demuxer->Stop();

            demuxer->Step();
        }

        // close the muxer and save the file to disk
        muxer->Close();

        if (vcodec != nullptr)
            delete vcodec;

        if (videoEncoder != nullptr)
            delete videoEncoder;


        delete muxer;
    }
    catch (FFmpegException e)
    {
        cerr << e.what() << "\n";
        throw e;
    }
}

int main(void)
{
	record_MKV();

	std::cout << "Encoding complete!" << "\n";

	return 0;
}