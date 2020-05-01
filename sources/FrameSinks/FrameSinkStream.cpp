#include "FrameSinkStream.h"
#include "FFmpegException.h"

namespace ffmpegcpp
{

	FrameSinkStream::FrameSinkStream(FrameWriter* p_frameSink, int p_streamIndex)
	{
		this->frameSink = p_frameSink;
		this->streamIndex = p_streamIndex;
	}

	void FrameSinkStream::WriteFrame(AVFrame* frame, StreamData* metaData)
	{
		frameSink->WriteFrame(streamIndex, frame, metaData);
	}

	void FrameSinkStream::Close()
	{
		frameSink->Close(streamIndex);
	}

	bool FrameSinkStream::IsPrimed()
	{
		return frameSink->IsPrimed();
	}
}
