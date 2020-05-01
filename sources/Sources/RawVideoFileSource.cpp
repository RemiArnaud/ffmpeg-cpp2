
#include "RawVideoFileSource.h"
#include "FFmpegException.h"
#include <iostream>

using namespace std;

namespace ffmpegcpp
{
    RawVideoFileSource::RawVideoFileSource(const char* fileName, FrameSink* p_frameSink)
    {
        setFrameSink(p_frameSink);
        // create the demuxer - it can handle figuring out the video type on its own apparently
        try
        {
            demuxer = new Demuxer(fileName, NULL, NULL);
            demuxer->DecodeBestVideoStream(m_frameSink);

        }
        catch (FFmpegException e)
        {
            CleanUp();
            throw e;
        }
    }
    // only testng on Linux for the moment, but Windows should work. MacOS X = don't care
#ifdef __linux__

    RawVideoFileSource::RawVideoFileSource(const char* fileName, int d_width, int d_height, int d_framerate, AVPixelFormat format)
    {
        // mandatory under Linux
        avdevice_register_all();

        width  = d_width;
        height = d_height;
        m_framerate = d_framerate;
        //frameSink = aFrameSink;
#ifdef _WIN32
        // Fixed by the operating system
        const char * input_device = "dshow"; // I'm using dshow when cross compiling :-)
#elif defined(__linux__)
        // libavutil, pixdesc.h
        const char * pix_fmt_name = av_get_pix_fmt_name(format);
        const char * pix_fmt_name2 = av_get_pix_fmt_name(AV_PIX_FMT_YUVJ420P); // = "mjpeg"
        enum AVPixelFormat pix_name = av_get_pix_fmt("mjpeg");
        enum AVPixelFormat pix_name2 = av_get_pix_fmt("rawvideo");

        cout<<"AVPixelFormat name of AV_PIX_FMT_YUV420P : " << pix_fmt_name << "\n";
        cout<<"AVPixelFormat name of AV_PIX_FMT_YUVJ420P : " << pix_fmt_name2 << "\n";
        cout<<"PixelFormat value for \"mjpeg\" : " << pix_name << "\n";
        cout<<"PixelFormat value for \"rawvideo\" : " << pix_name2 << "\n";

        // Fixed by the operating system
        const char * input_device = "v4l2";
//        const char * device_name = "/dev/video0";
#endif
        //  /!\ v4l2  is a DEMUXER for ffmpeg !!!  (not a device or format or whatever else !! )
        // important: AVCodecContext can be freed on failure (easy with mjpeg ...)
        pAVCodec = NULL;
        pAVFormatContextIn = NULL;
        options = NULL;
        pAVFormatContextIn = avformat_alloc_context();
        pAVFormatContextIn->video_codec_id = AV_CODEC_ID_MJPEG;

        inputFormat = av_find_input_format(input_device);

        // WORKS OK TOO
        char videoSize[32];
        sprintf(videoSize, "%dx%d", width, height);
        av_dict_set(&options, "video_size", videoSize, 0);
        // Other (fixed) way :
        // av_dict_set(&options, "video_size", "1280x720", 0);
        //av_dict_set(&options, "video_size", "1920x1080", 0);

#ifdef DEBUG
        const char * framerate_option_name = "frameRate";
        char frameRateValue[10];
        sprintf(frameRateValue, "%d", m_framerate);

        std::cerr << "framerate_option_name :  " << framerate_option_name  << "\n";
        std::cerr << "frameRateValue        :  " << frameRateValue  << "\n";
#else
        av_dict_set(&options, "framerate", "30", 0);
#endif

//        av_dict_set(&options, framerate_option_name, frameRateValue, 0);
        av_dict_set(&options, "pixel_format", pix_fmt_name2, 0);  //  "mjpeg" "yuvj420p"
        //av_dict_set(&options, "pixel_format", pix_fmt_name2, 0);  //  "mjpeg" "yuvj420p"
        av_dict_set(&options, "use_wallclock_as_timestamps", "1", 0);

        try
        {
            demuxer = new Demuxer(fileName, inputFormat, options, pAVFormatContextIn);
        }
        catch (FFmpegException e)
        {
            CleanUp();
            throw e;
        }


    }
#endif  /*  __linux__ */

        // Doesn't work for now. See the header for more info.
        /*RawVideoFileSource::RawVideoFileSource(const char* fileName, int width, int height, const char* frameRate, AVPixelFormat format, VideoFrameSink* frameSink)
        {
            // try to deduce the input format from the input format name
            AVInputFormat *file_iformat;
            if (!(file_iformat = av_find_input_format("yuv4mpegpipe")))
            {
                CleanUp();
                throw FFmpegException("Unknown input format 'rawvideo'");
            }

            AVDictionary* format_opts = NULL;

            // only set the frame rate if the format allows it!
            if (file_iformat && file_iformat->priv_class &&	av_opt_find(&file_iformat->priv_class, "framerate", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ))
            {
                av_dict_set(&format_opts, "framerate", frameRate, 0);
            }
            char videoSize[200];
            sprintf(videoSize, "%dx%d", width, height);
            av_dict_set(&format_opts, "video_size", videoSize, 0);
            const char* pixelFormatName = av_get_pix_fmt_name(format);
            av_dict_set(&format_opts, "pixel_format", pixelFormatName, 0);

            // create the demuxer
            try
            {
                demuxer = new Demuxer(fileName, file_iformat, format_opts);
                demuxer->DecodeBestVideoStream(frameSink);
            }
            catch (FFmpegException e)
            {
                CleanUp();
                throw e;
            }
        }*/

    RawVideoFileSource::~RawVideoFileSource()
    {
        CleanUp();
    }

    void RawVideoFileSource::CleanUp()
    {
        if (demuxer != nullptr)
        {
            avformat_close_input(&pAVFormatContextIn);
            avformat_free_context(pAVFormatContextIn);
            av_dict_free(&options);

            delete demuxer;
            demuxer = nullptr;
        }
    }

    void RawVideoFileSource::setFrameSink(FrameSink * aFrameSink)
    {
        m_frameSink = aFrameSink;
    }

    void RawVideoFileSource::PreparePipeline()
    {
        demuxer->PreparePipeline();
    }

    bool RawVideoFileSource::IsDone()
    {
        return demuxer->IsDone();
    }

    void RawVideoFileSource::Step()
    {
        demuxer->Step();
    }
}

