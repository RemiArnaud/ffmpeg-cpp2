
/*
 * File remux_webcam.cpp
 * renamed in remux_webcam_to_MKV.cpp
 * Copyright Eric Bachard / 2020 05 08
 * This document is under GPL v3 license
 * see : http://www.gnu.org/licenses/gpl-3.0.html
 */

#include <iostream>
#include <chrono>
#include <ffmpegcpp.h>
#include <thread>         // std::thread
#include <fstream>        // std::remove
#include <mutex>
#include "cyan-semaphore.h"

#include "video_device.h"

#include "getch.h"

static bool bRecording = false;
static bool b_quit = false;

using namespace ffmpegcpp;
using namespace std;

#define MAX_DEVICENAME_SIZE 16

//#ifdef ALSA_BUFFER_SIZE_MAX
#undef ALSA_BUFFER_SIZE_MAX
#define ALSA_BUFFER_SIZE_MAX  524288

const char * audio_file = "../videos/audio.mp4";  // aac (s32le ?)
const char * video_file = "../videos/video_H264.mp4";  // h264

/* The audio and video threads take different amounts of time to get started,
 * so they must be synchronized.
 * Only when both are ready can they both start.
*/
cyan::counting_semaphore<3> threads_ready(-1), threads_go(0), threads_done(-1);

void record_Audio()
{
    //const char * audioDevice = "pulse";
    const char audioDevice[MAX_DEVICENAME_SIZE] = "default";
    //const char * audioDevice = "hw:1,0";
    const char audioDeviceFormat[MAX_DEVICENAME_SIZE] = "alsa";

    Muxer* Amuxer  = new Muxer(audio_file);
    int audioSampleRate = 44100;
    int audioChannels   = 2;

    AudioCodec         *   audioCodec = new AudioCodec(AV_CODEC_ID_AAC);
    AudioEncoder       *   audioEncoder = new AudioEncoder(audioCodec, Amuxer);

    RawAudioFileSource *   audioFile = new RawAudioFileSource( audioDevice,
                                                            audioDeviceFormat,
                                                            audioSampleRate,
                                                            audioChannels,
                                                            audioEncoder);
    audioFile->PreparePipeline();

    threads_ready.release();
    cout << "Audio ready." << endl;
    threads_go.acquire();
    cout << "Audio GO!" << endl;

    while (!audioFile->IsDone())
    {
        audioFile->Step();

        if (bRecording == false)
            audioFile->Stop();
    }

    Amuxer->Close();

    if (audioEncoder != nullptr)
        delete audioEncoder;

    delete Amuxer;

    cout << "Audio done." << endl;
    threads_done.release();
}

#define FPS 30

void record_Video()
{
    int width  = 1280; // 1920;
    int height = 720;  // 1080;
    int fps = FPS;
    AVRational frameRate = { FPS, 1 };

    // These are example video and audio sources used below.
    const char * videoDevice = VIDEO_DEVICE;
    AVPixelFormat outputPixFormat= AV_PIX_FMT_NV12;

    //  OUTPUT CODEC, linked to the encoder ...
    Muxer* Vmuxer = new Muxer(video_file);
    H264Codec  * vcodec = new H264Codec();

    vcodec->SetGenericOption("b", "2M");
    vcodec->SetGenericOption("bit_rate", "2M");
    vcodec->SetGenericOption("movflags", "+faststart");
    vcodec->SetGenericOption("qmin", "12");
    vcodec->SetGenericOption("qmax", "34");
    vcodec->SetGenericOption("async", "1");
    vcodec->SetGenericOption("vsync", "1");
    vcodec->SetGenericOption("threads", "4");

    // FIXME : needs more tests
    //vcodec->SetGenericOption("pass", "2");
    vcodec->SetGenericOption("maxrate", "8M"); // or 4M
    vcodec->SetGenericOption("bufsize", "4M"); //    2M

    vcodec->SetGenericOption("stream_name", "Webcam C922"); // pb : 


    vcodec->SetProfile("high10"); // baseline, main, high, high10, high422
    vcodec->SetTune("film");  // film animation grain stillimage psnr ssim fastdecode zerolatency
    vcodec->SetPreset("medium"); // fast, medium, slow slower, veryslow placebo

    vcodec->SetCrf(23);

    VideoEncoder * videoEncoder = new VideoEncoder(vcodec, Vmuxer, frameRate, outputPixFormat);

    Demuxer * demuxer  = new Demuxer(videoDevice, width, height, fps);
    demuxer->DecodeBestVideoStream(videoEncoder);
    demuxer->PreparePipeline();

    threads_ready.release();
    cout << "Video ready." << endl;
    threads_go.acquire();
    cout << "Video GO!" << endl;

    while (!demuxer->IsDone())
    {
        demuxer->Step();

        if (bRecording == false)
        {
            demuxer->Stop();
        }
    }

    // close the first muxers and save separately audio and video files to disk
    Vmuxer->Close();

    if (videoEncoder != nullptr)
        delete videoEncoder;

    delete Vmuxer;

    cout << "Video done." << endl;
    threads_done.release();
}

void create_final_Video();
static void keyboard_loop();
int main(void)
{
    avdevice_register_all();
    //avformat_network_init(); // future use
    bRecording = true;

    //cout << "[main] Acquiring threads_go." << endl;
    //threads_go.acquire();
    //cout << "[main] threads_go acquired." << endl;

    thread loop  (keyboard_loop);
    thread first (record_Audio);
    thread second (record_Video);

    auto start = chrono::steady_clock::now();
    auto current_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_seconds = current_time - start;

    threads_ready.acquire();
    cout << "main: threads are ready." << endl;
    threads_go.release(2);

    cout << "Hit q key to stop recording" << "\n";

    do
    {
        current_time = std::chrono::steady_clock::now();
        elapsed_seconds = current_time - start;

        //    } while ((elapsed_seconds.count()) < (20));

    } while(b_quit == false);

    bRecording = false;

    threads_done.acquire();
    cout << "main: threads are done." << endl;

    if (loop.joinable())
        loop.join();

    if (first.joinable())
        first.join();

    if (second.joinable())
        second.join();


    create_final_Video();

    std::cout << "Encoding complete!" << "\n";

//#define TEST
#ifdef TEST
    remove(audio_file.c_str());
    remove(video_file.c_str());

    bool failed = (ifstream(audio_file) || ifstream(video_file));

    if(failed)
    {
        perror("Error opening deleted file");
        return 1;
    }
#endif

    return 0;
}

void create_final_Video()
{
    const char final_file[] = "../videos/final_video.mkv";  // h264 + aac (or vp9 + aac)

    Muxer* AVmuxer = new Muxer(final_file);

    AudioCodec * faudioCodec = new AudioCodec(AV_CODEC_ID_AAC);
    H264Codec  * fvcodec = new H264Codec();

    try
    {
        // Create encoders for both
        VideoEncoder* fvideoEncoder = new VideoEncoder(fvcodec, AVmuxer);
        AudioEncoder* faudioEncoder = new AudioEncoder(faudioCodec, AVmuxer);

        // Load both audio and video from a container
        Demuxer* videoContainer = new Demuxer(video_file);
        Demuxer* audioContainer = new Demuxer(audio_file);

        // Tie the best stream from each container to the output
        videoContainer->DecodeBestVideoStream(fvideoEncoder);
        audioContainer->DecodeBestAudioStream(faudioEncoder);

        // Prepare the pipeline. We want to call this before the rest of the loop
        // to ensure that the muxer will be fully ready to receive data from
        // multiple sources.
        videoContainer->PreparePipeline();
        audioContainer->PreparePipeline();

        // Pump the audio and video fully through.
        // To avoid big buffers, we interleave these calls so that the container
        // can be written to disk efficiently.
        while ( (!videoContainer->IsDone()) || (!audioContainer->IsDone()))
        {
            if (!videoContainer->IsDone())
                videoContainer->Step();

            if (!audioContainer->IsDone())
                audioContainer->Step();
        }

        // Save everything to disk by closing the muxer.
        AVmuxer->Close();
    }
    catch (const FFmpegException & e)
    {
        cerr << e.what() << "\n";
        throw e;
    }

    delete AVmuxer;
}
static void keyboard_loop()
{
    char c = 0;
    while ((c != 27) && (c != 'q') && (c != EXIT_FAILURE)) // esc
    {
        c = getch();
    }
    if ((c == 27) || (c == 'q') || (c == EXIT_FAILURE)) // esc
        b_quit = true;
}
