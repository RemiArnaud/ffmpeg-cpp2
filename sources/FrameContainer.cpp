#include "FrameContainer.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
    FrameContainer::FrameContainer(AVFrame * frame, AVRational * timeBase)
	{
        AVFrame * tmp = av_frame_clone(frame);
		if (!tmp) throw new FFmpegException("Failed to clone frame");
		av_frame_unref(frame);
        m_frame = tmp;
        m_timeBase = timeBase;
	}

	FrameContainer::~FrameContainer()
	{
        av_frame_free(&m_frame);
	}

    AVFrame * FrameContainer::GetFrame()
	{
        return m_frame;
	}

    AVRational * FrameContainer::GetTimeBase()
	{
        return m_timeBase;
	}
}
