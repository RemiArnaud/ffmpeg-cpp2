#pragma once

#include "ffmpegcpp.h"

using namespace ffmpegcpp;

class VideoInputStreamWithShaderInfo : public VideoInputStream
{
public:
	VideoInputStreamWithShaderInfo(class VideoSourceWithShaderInfo *pInputSource) : VideoInputStream()
	{
		m_pInputSource = pInputSource;
		m_pFrame = av_frame_alloc();

	}
	~VideoInputStreamWithShaderInfo()
	{
		delete m_pOutput;
		delete rgb;
	}


	void Init(Demuxer*pInput);

	void WriteFrame(AVFrame* frame, StreamData* metadata);

	void Close() { m_pOutput->Close(); }


	class VideoSourceWithShaderInfo *m_pInputSource;

	SwsContext *m_pConvContext;
	RawVideoDataSource* m_pOutput;
	int m_nSrcWidth;
	int m_nSrcHeight;
	AVPixelFormat m_Srcformat;
	AVFrame *m_pFrame;
	uint8_t *rgb = NULL;
	int frameNumber = 0;

};

class VideoSourceWithShaderInfo : public InputSource
{
public:

	VideoSourceWithShaderInfo(const char* fileName, int nExtraheight, FrameSink* frameSink);
	~VideoSourceWithShaderInfo();

	virtual void PreparePipeline();
	virtual bool IsDone();
	virtual void Step();

	int GetExtraHeight() { return m_nExtraheight; }
	FrameSink* GetFrameSink() {		return m_pFrameSink;	}

	
private:

	VideoInputStreamWithShaderInfo *m_pVideoInputStream;
	Demuxer* m_pInput;
	FrameSink* m_pFrameSink;
	int m_nExtraheight;
};

