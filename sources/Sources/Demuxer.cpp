#define _CRT_SECURE_NO_WARNINGS
#include "Demuxer.h"
#include "FFmpegException.h"
#include "CodecDeducer.h"
#include <string>
#include <iostream>

using namespace std;

namespace ffmpegcpp
{
    Demuxer::Demuxer(const char * fileName)
        : Demuxer(fileName, NULL, NULL)
    {
    }

    Demuxer::Demuxer(const std::string & p_fileName, const AVInputFormat * p_inputFormat, AVDictionary * p_format_opts)
    {
        m_fileName    = p_fileName.c_str();

        int ret = 0;// open input file, and allocate format context

        if ((ret = avformat_open_input(&m_containerContext, p_fileName.c_str(), p_inputFormat, &p_format_opts)) < 0)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to open input container " + string(p_fileName)).c_str(), ret);
        }

        this->m_options       = p_format_opts;
        this->m_inputFormat = p_inputFormat;

        // retrieve stream information
        if ( (ret = (avformat_find_stream_info(m_containerContext, NULL))) < 0)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to read streams from " + string(m_fileName)).c_str(), ret);
        }

        m_inputStreams = new InputStream*[m_containerContext->nb_streams];
        for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
        {
            m_inputStreams[i] = nullptr;
        }

        // initialize packet, set data to NULL, let the demuxer fill it
        m_pkt = av_packet_alloc();
        if (!m_pkt)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to create packet for input stream").c_str());
        }
        m_pkt = av_packet_alloc();
        m_pkt->data = NULL;
        m_pkt->size = 0;
    }

    Demuxer::Demuxer(const std::string & p_fileName, int d_width, int d_height, int d_framerate)
    {
        m_fileName = p_fileName.c_str();
        m_width  = d_width;
        m_height = d_height;
        m_framerate = d_framerate;
        setVideoStreamDevice();
    }

    Demuxer::Demuxer(const std::string & deviceName, const char * inputFormat, int sampleRate, int channels, AudioFrameSink * p_frameSink)
    {
        // LINUX : fileName = hw::1,0; input format = alsa; sampleRate = 48000 (default); channels= 2 (default); frameSink = audioDecoder
        // WINDOWS : fileName = ?? ; input format = alsa; sampleRate = 48000 (default); channels= 2 (default); frameSink = audioDecoder

/*
         ~ $ arecord --dump-hw-params -D hw:1,0
        Capture WAVE 'stdin' : Unsigned 8 bit, Fréquence 8000 Hz, Mono
        HW Params of device "hw:1,0":
        --------------------
        ACCESS:  MMAP_INTERLEAVED RW_INTERLEAVED
        FORMAT:  S16_LE
        SUBFORMAT:  STD
        SAMPLE_BITS: 16
        FRAME_BITS: 32
        CHANNELS: 2
        RATE: [16000 32000]
        PERIOD_TIME: [1000 8192000]
        PERIOD_SIZE: [16 131072]
        PERIOD_BYTES: [64 524288]
        PERIODS: [2 1024]
        BUFFER_TIME: [1000 16384000]
        BUFFER_SIZE: [32 262144]
        BUFFER_BYTES: [128 1048576]
        TICK_TIME: ALL
        --------------------
        arecord: set_params:1299: Ce format d'échantillonage n'est pas disponible
        Available formats:
        - S16_LE
*/
        m_audio_device = deviceName.c_str();
        m_sampleRate = sampleRate;
        m_channels = channels;
        m_audio_frameSink = p_frameSink;
        m_audio_opts = nullptr;

        m_containerContext = avformat_alloc_context();

        //containerContext->audio_codec_id = AV_CODEC_ID_AAC;

        if (!(m_file_iformat = av_find_input_format(inputFormat)))
        {
            CleanUp();
            throw FFmpegException(std::string("Unknown input format: " + string(inputFormat)).c_str());
        }

        setAudioStreamDevice ();
    }

    void Demuxer::setAudioStreamDevice ()
    {

        // FIXME : remove unnecessary parameters
        av_dict_set_int(&m_audio_opts, "sample_rate", m_sampleRate, 0);
        av_dict_set_int(&m_audio_opts, "minrate", 20000, 0);
        av_dict_set_int(&m_audio_opts, "maxrate", 40000, 0);

// !!        av_dict_set_int(&m_audio_opts, "b", 128000, 0);
        av_dict_set_int(&m_audio_opts, "bit_rate", 32000, 0);
        av_dict_set_int(&m_audio_opts, "profile", FF_PROFILE_AAC_LOW, 0);
        av_dict_set_int(&m_audio_opts, "time_base.num", 1 , 0);
        av_dict_set_int(&m_audio_opts, "time_base.den", m_sampleRate, 0);

        av_dict_set_int(&m_audio_opts, "duration", 3600, 0);
        // av_dict_set_int(&m_audio_opts, "thread_queue_size", 1024 , 0);
        av_dict_set_int(&m_audio_opts, "threads", 0 , 0);
        av_dict_set_int(&m_audio_opts, "frame_size", 1024, 0);
        av_dict_set_int(&m_audio_opts, "frames", 32, 0);
//!!        av_dict_set_int(&m_audio_opts, "format", AV_SAMPLE_FMT_S16, 0);
        av_dict_set_int(&m_audio_opts, "format", AV_SAMPLE_FMT_S32P, 0);
//        av_dict_set    (&m_audio_opts, "movflags", "faststart", 0);

        av_dict_set    (&m_audio_opts, "use_wallclock_as_timestamps", "1", 0);
//        av_dict_set_int(&m_audio_opts, "async", 1000, 0);
//        av_dict_set_int(&m_audio_opts, "aresample", 1000, 0);
        av_dict_set_int(&m_audio_opts, "channels", m_channels, 0);
        av_dict_set    (&m_audio_opts, "stream_name", "webcam C922", 0);

        int ret = 0;

        if ((ret = avformat_open_input(&m_containerContext, m_audio_device, m_file_iformat, &m_audio_opts)) < 0)
        {
            std::cerr << "Failed to open input container. ret =  " <<  ret  <<  "\n";
            CleanUp();
            throw FFmpegException(std::string("Failed to open input container " + string(m_audio_device)).c_str(), ret);
        }

        if ( (ret = (avformat_find_stream_info(m_containerContext, NULL))) < 0)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to read streams from " + string(m_fileName)).c_str(), ret);
        }
        av_dump_format(m_containerContext , 0 , m_audio_device , 0 );

        m_AudioStreamIndx = setStreamIndexAndCopyParameters(AVMEDIA_TYPE_AUDIO);
    }

    void Demuxer::setVideoStreamDevice ()
    {
        int ret = 0;
#ifdef _WIN32
        const char * input_device = "dshow"; // Fixed by the operating system
#elif defined(__linux__)
        // libavutil, pixdesc.h
        const char * input_device = "v4l2";
#endif
        const char * pix_fmt_name = av_get_pix_fmt_name(AV_PIX_FMT_YUVJ420P); // = "mjpeg"

        if (m_containerContext == nullptr)
            m_containerContext = avformat_alloc_context();

        m_containerContext->video_codec_id = AV_CODEC_ID_MJPEG;

        m_inputFormat = av_find_input_format(input_device);

        m_options= nullptr;

        char videoSize[32];
        sprintf(videoSize, "%dx%d", this->m_width, this->m_height);
        av_dict_set(&m_options, "video_size", videoSize, 0);

        const char * framerate_option_name = "framerate";
        char frameRateValue[10];
        sprintf(frameRateValue, "%d", this->m_framerate);

        av_dict_set(&m_options, framerate_option_name, frameRateValue, 0);
        av_dict_set(&m_options, "pixel_format", pix_fmt_name, 0);  //  "mjpeg" "yuvj420p"
        av_dict_set(&m_options, "use_wallclock_as_timestamps", "1", 0);
        av_dict_set(&m_options, "movflags", "faststart", 0 ); // FIXME : only for H264 (moov atom not found)
        ///ret = avformat_write_header( ofmt_ctx, &dict );

        if ((ret = avformat_open_input(&m_containerContext, m_fileName, m_inputFormat, &m_options)) < 0)
        {
            std::cerr << "Failed to open input container. ret =  " <<  ret  <<  "\n";
            CleanUp();
            throw FFmpegException(std::string("Failed to open input container " + string(m_fileName)).c_str(), ret);
        }

        // retrieve stream information
        if ( (ret = (avformat_find_stream_info(m_containerContext, NULL))) < 0)
        {
            CleanUp();
            throw FFmpegException(std::string("Failed to read streams from " + string(m_fileName)).c_str(), ret);
        }
        av_dump_format(m_containerContext , 0 , m_fileName , 0 );

        m_VideoStreamIndx = setStreamIndexAndCopyParameters(AVMEDIA_TYPE_VIDEO);
    }


    int Demuxer::setStreamIndexAndCopyParameters(AVMediaType aMediaType)
    {
        // retrieve stream information
        int index = -1;

        for(unsigned int i = 0; i < m_containerContext->nb_streams ;i++ )
        {
            if( m_containerContext->streams[i]->codecpar->codec_type == aMediaType ) {
                index = i;
                break;
            }
        }

        if((index) == -1) {
            cout<<"Error : video streams not found in demuxer ctor";
            CleanUp();
            throw FFmpegException(std::string("Failed to create packet for input stream").c_str());
        }

        // inspired from https://code.mythtv.org/trac/ticket/13186?cversion=0&cnum_hist=2
        AVCodecContext *pAVCodecContext = NULL;
        m_pAVCodec = NULL;
        m_pAVCodec = avcodec_find_decoder(m_containerContext->streams[index]->codecpar->codec_id);
        pAVCodecContext = avcodec_alloc_context3(m_pAVCodec);
        avcodec_parameters_to_context(pAVCodecContext, m_containerContext->streams[index]->codecpar);

        m_inputStreams = new InputStream*[m_containerContext->nb_streams];

        for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
        {
            m_inputStreams[i] = nullptr;
        }

        // initialize packet, set data to NULL, let the demuxer fill it
        m_pkt = av_packet_alloc();

        if (!m_pkt) {

            CleanUp();
            throw FFmpegException(std::string("Failed to create packet for input stream").c_str());
        }

        m_pkt = av_packet_alloc();
        m_pkt->data = NULL;
        m_pkt->size = 0;
        avcodec_close(pAVCodecContext);

        return index;
    }


    Demuxer::~Demuxer()
    {
        CleanUp();
    }

    void Demuxer::CleanUp()
    {
        if (m_inputStreams != nullptr)
        {
            for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
            {
                m_inputStreams[i] = nullptr;
            }
            delete m_inputStreams;
            m_inputStreams = nullptr;
        }

        if (m_containerContext != nullptr)
        {
            avformat_close_input(&m_containerContext);
            m_containerContext = nullptr;
        }

        if (m_pkt != nullptr)
        {
            av_packet_free(&m_pkt);
            m_pkt = nullptr;
        }
    }

    void Demuxer::DecodeBestAudioStream(FrameSink* frameSink)
    {
        int ret = av_find_best_stream(m_containerContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

        if (ret < 0)
        {
            throw FFmpegException(std::string("Could not find " + string(av_get_media_type_string(AVMEDIA_TYPE_AUDIO)) + " stream in input file " + m_fileName).c_str(), ret);
        }

        int streamIndex = ret;
        return DecodeAudioStream(streamIndex, frameSink);
    }

    void Demuxer::DecodeBestVideoStream(FrameSink* frameSink)
    {
        int ret = av_find_best_stream(m_containerContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

        if (ret < 0)
        {
            throw FFmpegException(std::string("Could not find " + string(av_get_media_type_string(AVMEDIA_TYPE_VIDEO)) + " stream in input file " + m_fileName).c_str(), ret);
        }

        int streamIndex = ret;
        return DecodeVideoStream(streamIndex, frameSink);
    }

    void Demuxer::DecodeAudioStream(int streamIndex, FrameSink* frameSink)
    {
        std::cerr << "Im in : " << __func__ << " line : " << __LINE__ << "\n";

        // each input stream can only be used once
        if (m_inputStreams[streamIndex] != nullptr)
        {
            throw FFmpegException(std::string("That stream is already tied to a frame sink, you cannot process the same stream multiple times").c_str());
        }

        // create the stream
        InputStream* inputStream = GetInputStream(streamIndex);
        inputStream->Open(frameSink);

        // remember and return
        m_inputStreams[streamIndex] = inputStream;
    }

    void Demuxer::DecodeVideoStream(int streamIndex, FrameSink* frameSink)
    {
        // each input stream can only be used once
        if (m_inputStreams[streamIndex] != nullptr)
        {
            throw FFmpegException(std::string("That stream is already tied to a frame sink, you cannot process the same stream multiple times").c_str());
        }

        // create the stream
        InputStream* inputStream = GetInputStream(streamIndex);
        inputStream->Open(frameSink);

        // remember and return
        m_inputStreams[streamIndex] = inputStream;
    }

    InputStream* Demuxer::GetInputStream(int streamIndex)
    {
        // already exists
        if (m_inputStreams[streamIndex] != nullptr) return m_inputStreams[streamIndex];

        // The stream doesn't exist but we already processed all our frames, so it makes no sense
        // to add it anymore.
        if (IsDone())
            return nullptr;

        AVStream* stream = m_containerContext->streams[streamIndex];
        m_pAVCodec = CodecDeducer::DeduceDecoder(stream->codecpar->codec_id);

        if (m_pAVCodec == nullptr)
            return nullptr; // no codec found - we can't really do anything with this stream!

        switch (m_pAVCodec->type)
        {
            case AVMEDIA_TYPE_VIDEO:
            m_inputStreams[streamIndex] = new VideoInputStream(m_containerContext, stream);
            break;

            case AVMEDIA_TYPE_AUDIO:
            m_inputStreams[streamIndex] = new AudioInputStream(m_containerContext, stream);
            break;

            default:
            break;
        }

        // return the created stream
        return m_inputStreams[streamIndex];
    }

    InputStream* Demuxer::GetInputStreamById(int streamId)
    {
        // map the stream id to an index by going over all the streams and comparing the id
        for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
        {
            AVStream* stream = m_containerContext->streams[i];

            if (stream->id == streamId)
                return GetInputStream(i);
        }

        // no match found
        return nullptr;
    }

    void Demuxer::PreparePipeline()
    {
        bool allPrimed = false;

        do
        {
            Step();

            // see if all input streams are primed
            allPrimed = true;

            for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
            {
                InputStream* stream = m_inputStreams[i];

                if (stream != nullptr)
                {
                    if (!stream->IsPrimed())
                        allPrimed = false;
                }
            }

        } while (!allPrimed && !IsDone());

    }

    bool Demuxer::IsDone()
    {
        return m_done;
    }


    void Demuxer::Step()
    {
        // read frames from the file
        int ret = av_read_frame(m_containerContext, m_pkt);

        // EOF
        if (ret == AVERROR_EOF)
        {
            m_pkt->data = NULL;
            m_pkt->size = 0;

            for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
            {
                InputStream* stream = m_inputStreams[i];


                if (stream != nullptr)
                {
                    m_pkt->stream_index = i;
                    DecodePacket();
                    stream->Close();
                }
            }

            m_done = true;
            return;
        }

        // not ready yet
        if (ret == AVERROR(EAGAIN)) return;

        // error
        if (ret < 0)
        {
            throw FFmpegException(std::string("Error during demuxing").c_str(), ret);
        }

        // decode the finished packet
        DecodePacket();
    }

    void Demuxer::DecodePacket()
    {
        //std::cerr  << "frame : " <<  frameCount << "\n";

        int streamIndex = m_pkt->stream_index;
        InputStream* inputStream = m_inputStreams[streamIndex];

        if (inputStream != nullptr)
        {
            inputStream->DecodePacket(m_pkt);
        }

        // We need to unref the packet here because packets might pass by here
        // that don't have a stream attached to them. We want to dismiss them!
        av_packet_unref(m_pkt);

        m_frameCount++;
    }

    ContainerInfo Demuxer::GetInfo()
    {
        ContainerInfo info;

        // general data
        // the duration is calculated like this... why?
        if(m_containerContext->duration != AV_NOPTS_VALUE) {
            int64_t duration = m_containerContext->duration + (m_containerContext->duration <= INT64_MAX - 5000 ? 5000 : 0);
            info.durationInMicroSeconds = duration;
            info.durationInSeconds = (float)info.durationInMicroSeconds / AV_TIME_BASE;
        }else{
            info.durationInMicroSeconds = 0;
            info.durationInSeconds = 0;
        }
        info.start = (float)m_containerContext->start_time / AV_TIME_BASE;
        info.bitRate = m_containerContext->bit_rate;
        info.format = m_containerContext->iformat;

        // go over all streams and get their info
        for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
        {
            InputStream* stream = GetInputStream(i);

            if (stream == nullptr) continue; // no valid stream
                stream->AddStreamInfo(&info);
        }

        return info;
    }

    int Demuxer::GetFrameCount(int streamId)
    {
        // Make sure all streams exist, so we can query them later.
        for (unsigned int i = 0; i < m_containerContext->nb_streams; ++i)
        {
            GetInputStream(i);
        }
        // Process the entire container so we can know how many frames are in each
        if(hasMaxFrameCount()) {
            if(!IsDone())
                Step();
            if(m_frameCount == m_manualMaxFrameCount)
                m_done = true;
        }else{
            while (!IsDone()) {
                Step();
            }
        }
        // Return the right stream's frame count.
        return GetInputStreamById(streamId)->GetFramesProcessed();
    }

    const char* Demuxer::GetFileName()
    {
        return m_fileName;
    }
    bool Demuxer::hasMaxFrameCount() const
    {
        return m_manualMaxFrameCount != -1;
    }

    void Demuxer::setMaxFrameCount(int fc)
    {
        m_manualMaxFrameCount = fc;
    }
    int Demuxer::maxFrameCount()
    {
        return m_manualMaxFrameCount;
    }
}

