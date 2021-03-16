#pragma once

#include "ffmpeg.h"

#include "Demuxing/AudioInputStream.h"
#include "Demuxing/VideoInputStream.h"
#include "Demuxing/InputStream.h"
#include "Sources/InputSource.h"
#include "Info/ContainerInfo.h"

namespace ffmpegcpp
{
    class Demuxer : public InputSource
    {
    public:

        Demuxer(const char* fileName);
        Demuxer(const char* fileName, int width, int height, int framerate);
        Demuxer(const char* fileName, AVInputFormat* inputFormat, AVDictionary *inputFormatOptions);
        Demuxer(const char* fileName, const char* inputFormat, int sampleRate, int channels, AudioFrameSink * p_audio_frameSink);
        Demuxer(const char* fileName, AVInputFormat* inputFormat, AVDictionary *inputFormatOptions, AVFormatContext * aContainerContext);
        ~Demuxer();

        void DecodeBestAudioStream(FrameSink* frameSink, InputStream* pInputStream = nullptr);
        void DecodeBestVideoStream(FrameSink* frameSink, InputStream* pInputStream = nullptr);

        void DecodeAudioStream(int streamId, FrameSink* frameSink, InputStream* pInputStream = nullptr);
        void DecodeVideoStream(int streamId, FrameSink* frameSink, InputStream* pInputStream = nullptr);

        virtual void PreparePipeline();
        virtual bool IsDone();
        virtual void Step();

        void Stop() { done = true;}

        ContainerInfo GetInfo();
        int GetFrameCount(int streamId);

        int  getVideoStreamIndx() { return m_VideoStreamIndx ;}

        void setAudioStreamDevice();
        void setVideoStreamDevice();

        const char* GetFileName();

		InputStream* GetInputStream(int index, bool bDontCreate=true, InputStream* pVideoInputStream = nullptr, InputStream* pAudioInputStream = nullptr);
		InputStream* GetInputStreamById(int streamId);		
		int GetInputStreamCount() 
		{
			if (containerContext)
				return containerContext->nb_streams;
			else
				return 0;
		}


    private:

        bool done = false;

        const char* m_fileName;

        int m_width;
        int m_height;
        int m_framerate;

        int m_VideoStreamIndx = 0;

        int frameCount = 0;

        AVFormatContext* pAVFormatContextIn;

        AVDictionary * options;
        AVCodec * pAVCodec;
        AVInputFormat * m_inputFormat;

        int setStreamIndexAndCopyParameters(AVMediaType aMediaType);

        const char* m_audio_device;

        AVDictionary   *  m_audio_opts;
        AVInputFormat  *  m_file_iformat;
        AudioFrameSink *  m_audio_frameSink;
        int               m_sampleRate;
        int               m_channels;
        int               m_AudioStreamIndx = 0;


        //std::vector<StreamInfo> GetStreamInfo(AVMediaType mediaType);
        //StreamInfo CreateInfo(int streamIndex, AVStream* stream, AVCodec* codec);

        InputStream** inputStreams = nullptr;

        AVFormatContext* containerContext = nullptr;
        AVPacket* pkt = nullptr;

        void DecodePacket();

        void CleanUp();
    };
}
