#include "VideoSourceWithShaderInfo.h"
#include <ffmpegcpp.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VideoInputStreamWithShaderInfo::Init(Demuxer*pInput)
{
	ContainerInfo Info = pInput->GetInfo();
	if (Info.videoStreams.size() != 0)
	{

		m_nSrcWidth = Info.videoStreams[0].width;
		m_nSrcHeight = Info.videoStreams[0].height;
		m_Srcformat = Info.videoStreams[0].format;
		m_pOutput = new RawVideoDataSource(m_nSrcWidth, m_nSrcHeight + m_pInputSource->GetExtraHeight(), AV_PIX_FMT_RGBA, m_Srcformat, Info.videoStreams[0].frameRate.num, m_pInputSource->GetFrameSink());
		m_pConvContext = sws_getContext(m_nSrcWidth, m_nSrcHeight, m_Srcformat, m_nSrcWidth, m_nSrcHeight, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
		int size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_nSrcWidth, m_nSrcHeight + m_pInputSource->GetExtraHeight(), 1);

		rgb = (uint8_t *)malloc(size * sizeof(uint8_t));
		av_image_fill_arrays(m_pFrame->data, m_pFrame->linesize, rgb, AV_PIX_FMT_RGBA, m_nSrcWidth, m_nSrcHeight + m_pInputSource->GetExtraHeight(), 1);
	}

}

void VideoInputStreamWithShaderInfo::WriteFrame(AVFrame* frame, StreamData* metadata)
{
	AVFrame *pFrame = GetFrame();
	AVCodecContext* pContext = GetContext();

	sws_scale(m_pConvContext, pFrame->data, pFrame->linesize, 0, m_nSrcHeight, m_pFrame->data, m_pFrame->linesize);


	m_pOutput->WriteFrame(m_pFrame->data[0], 4 * m_pOutput->GetWidth());



	//rgb = generate_rgb(m_pOutput->GetWidth(), m_pOutput->GetHeight(), frameNumber, rgb);
	//m_pOutput->WriteFrame(rgb, 4 * m_pOutput->GetWidth());
	++frameNumber;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VideoSourceWithShaderInfo::VideoSourceWithShaderInfo(const char* fileName, int nExtraheight, FrameSink* frameSink)
{

	m_pInput = new Demuxer(fileName);

	m_pVideoInputStream = new VideoInputStreamWithShaderInfo(this);

	m_pInput->DecodeBestVideoStream(NULL, m_pVideoInputStream);
	m_pFrameSink = frameSink;
	m_nExtraheight = nExtraheight;
	
}

VideoSourceWithShaderInfo::~VideoSourceWithShaderInfo()
{
	delete m_pInput;
}

uint8_t* generate_rgb(int width, int height, int pts, uint8_t *rgb);

void VideoSourceWithShaderInfo::PreparePipeline()
{
	/*while (!m_pOutput->IsPrimed() && !IsDone())
	{
		Step();
	}*/
	m_pInput->PreparePipeline();

	m_pVideoInputStream->Init(m_pInput);
}




void VideoSourceWithShaderInfo::Step()
{
	m_pInput->Step();

	
	if (IsDone())
	{
		m_pVideoInputStream->Close();
	}
}

bool VideoSourceWithShaderInfo::IsDone()
{
	return m_pInput->IsDone();
}

/*
uint8_t* generate_rgb(int width, int height, int pts, uint8_t *rgb)
{
	int x, y, cur;
	rgb = (uint8_t*)realloc(rgb, 4 * sizeof(uint8_t) * height * width);
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			cur = 4 * (y * width + x);
			rgb[cur + 0] = 0;
			rgb[cur + 1] = 0;
			rgb[cur + 2] = 0;
			rgb[cur + 3] = (pts % 25) * (255 / 25);
			if ((pts / 25) % 2 == 0)
			{
				if (y < height / 2)
				{
					if (x < width / 2)
					{
						/ * Black. * /
					}
					else
					{
						rgb[cur + 0] = 255;
					}
				}
				else
				{
					if (x < width / 2)
					{
						rgb[cur + 1] = 255;
					}
					else
					{
						rgb[cur + 2] = 255;
					}
				}
			}
			else
			{
				if (y < height / 2)
				{
					rgb[cur + 0] = 255;
					if (x < width / 2)
					{
						rgb[cur + 1] = 255;
					}
					else
					{
						rgb[cur + 2] = 255;
					}
				}
				else
				{
					if (x < width / 2)
					{
						rgb[cur + 1] = 255;
						rgb[cur + 2] = 255;
					}
					else
					{
						rgb[cur + 0] = 255;
						rgb[cur + 1] = 255;
						rgb[cur + 2] = 255;
					}
				}
			}
		}
	}
	return rgb;
}

*/
