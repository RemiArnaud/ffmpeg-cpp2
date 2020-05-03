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
        Demuxer(const char* fileName, AVInputFormat* inputFormat, AVDictionary *inputFormatOptions, AVFormatContext * aContainerContext);
        ~Demuxer();

        void DecodeBestAudioStream(FrameSink* frameSink);
        void DecodeBestVideoStream(FrameSink* frameSink);

        void DecodeAudioStream(int streamId, FrameSink* frameSink);
        void DecodeVideoStream(int streamId, FrameSink* frameSink);

        virtual void PreparePipeline();
        virtual bool IsDone();
        virtual void Step();

        ContainerInfo GetInfo();
        int GetFrameCount(int streamId);

        bool convertToRGB(int VideoStreamIndx, AVFormatContext *bAVFormatContext, AVCodecContext *bAVCodecContext, AVCodec *bAVCodec );

        void setVideoStreamDevice();
        const char* GetFileName();

    private:

        bool done = false;

        const char* m_fileName;

        int m_width;
        int m_height;
        int m_framerate;


        AVFormatContext* pAVFormatContextIn;
        AVDictionary * options;
        AVCodec * pAVCodec;
        AVInputFormat * m_inputFormat;

        InputStream* GetInputStream(int index);
        InputStream* GetInputStreamById(int streamId);

        //std::vector<StreamInfo> GetStreamInfo(AVMediaType mediaType);
        //StreamInfo CreateInfo(int streamIndex, AVStream* stream, AVCodec* codec);

        InputStream** inputStreams = nullptr;

        AVFormatContext* containerContext = nullptr;
        AVPacket* pkt = nullptr;

        void DecodePacket();

        void CleanUp();
    };
}
