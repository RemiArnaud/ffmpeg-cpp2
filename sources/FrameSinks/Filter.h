#pragma once

#include "ffmpeg.h"


#include "VideoFrameSink.h"
#include "VideoFilterInput.h"
#include <vector>
#include <string>

namespace ffmpegcpp
{
	class Filter : public FrameSink, public FrameWriter
	{

	public:

        Filter(const char * filterString, FrameSink * target);
		virtual ~Filter();

        FrameSinkStream * CreateStream();

        void WriteFrame(int streamIndex, AVFrame * frame, StreamData * metaData);
		void Close(int streamIndex);

		bool IsPrimed();

		virtual AVMediaType GetMediaType();

	private:

		void CleanUp();

		void ConfigureFilterGraph();
		void DrainInputQueues();
		void PollFilterGraphForFrames();
		void FillArguments(char* args, int argsLength, AVFrame* frame, StreamData* metaData);

        const char * GetBufferName(AVMediaType mediaType);
        const char * GetBufferSinkName(AVMediaType mediaType);

        std::vector<VideoFilterInput*> m_inputs;
        std::vector<AVFilterContext*> m_bufferSources;

        AVMediaType m_targetMediaType;
        FrameSinkStream * m_target;

        const char * m_filterString;

        AVFilterGraph * m_filter_graph = nullptr;
        AVFilterContext * m_buffersink_ctx = nullptr;
        AVFrame * m_filt_frame = nullptr;

        bool m_initialized = false;

        StreamData m_outputMetaData;
	};
}
