
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

//#ifdef ALSA_BUFFER_SIZE_MAX
#undef ALSA_BUFFER_SIZE_MAX
#define ALSA_BUFFER_SIZE_MAX  524288


void record_MKV()
{
    // These are example video and audio sources used below.
#ifndef VIDEO_MIX
    const char* videoDevice = "/dev/video0";
#endif
    //  OUTPUT CODEC, linked to the encoder ...
    H264Codec  * vcodec = new H264Codec();
    vcodec->SetGenericOption("b", "2M");
    vcodec->SetGenericOption("bit_rate", "2M");
    vcodec->SetGenericOption("movflags", "+faststart");
    vcodec->SetGenericOption("qmin", "12");
    vcodec->SetGenericOption("qmax", "34");
    vcodec->SetGenericOption("async", "1");
    vcodec->SetGenericOption("vsync", "1");
    vcodec->SetGenericOption("threads", "4");

    // ffmpeg -i input -c:v libx264 -b:v 1M -maxrate 1M -bufsize 2M -pass 2 output.mp4

    // FIXME : needs more tests
    //vcodec->SetGenericOption("pass", "2");
    vcodec->SetGenericOption("maxrate", "8M"); // or 4M
    vcodec->SetGenericOption("bufsize", "4M"); //    2M

    vcodec->SetProfile("high10"); // baseline, main, high, high10, high422
    vcodec->SetTune("film");  // film animation grain stillimage psnr ssim fastdecode zerolatency
    vcodec->SetPreset("medium"); // fast, medium, slow slower, veryslow placebo

    vcodec->SetCrf(23);

    //const char* audioDevice = "hw:1,0"; // first webcam   1 is DEV alsa parameter, 0 is 
    const char* audioDevice = "default";

    const char * audioDeviceFormat = "alsa";
    int audioSampleRate = 44100;
    int audioChannels   = 2;

    AudioCodec * audioCodec = new AudioCodec(AV_CODEC_ID_AAC);

    //audioCodec->Open(64000, AV_SAMPLE_FMT_FLTP, audioSampleRate);

    // Create a muxer that will output the video as MKV.
    Muxer* muxer = new Muxer("../videos/output_H264.mp4");  // good result
    // create the different components that make this come together
    try
    {

        int width  = 1280;
        int height = 720;
        int fps = 24;

        // FIXME : timing is not precise, and probably wrong
        AVRational frameRate = { 24, 1 };

        // All seem to work
        //AVPixelFormat outputPixFormat= AV_PIX_FMT_RGBA;
        //AVPixelFormat outputPixFormat= AV_PIX_FMT_YUV420P;
        AVPixelFormat outputPixFormat= AV_PIX_FMT_NV12;

        VideoEncoder * videoEncoder = new VideoEncoder(vcodec, muxer, frameRate, outputPixFormat);
        AudioEncoder * audioEncoder = new AudioEncoder(audioCodec, muxer);

        Demuxer * audioFile = new Demuxer(audioDevice, audioDeviceFormat, audioSampleRate, audioChannels, audioEncoder);
        Demuxer * demuxer  = new Demuxer(videoDevice, width, height, fps);

        audioFile->DecodeBestAudioStream(audioEncoder);
        demuxer->DecodeBestVideoStream(videoEncoder);

        audioFile->PreparePipeline();
        demuxer->PreparePipeline();

        while (!demuxer->IsDone() && !audioFile->IsDone())
        {
            //int frameNumber = demuxer->GetFrameCount(demuxer->getVideoStreamIndx());
            demuxer->Step();
            audioFile->Step();
        }

        // close the muxer and save the file to disk
        muxer->Close();

        if (vcodec != nullptr)
            delete vcodec;

        if (videoEncoder != nullptr)
            delete videoEncoder;

        if (audioEncoder != nullptr)
            delete audioEncoder;

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
    avdevice_register_all();
//    avdevice_register_all();
//    avformat_network_init();

    record_MKV();

    std::cout << "Encoding complete!" << "\n";

	return 0;
}