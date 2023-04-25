#include "RawAudioFileSource.h"
#include "FFmpegException.h"


using namespace std;

namespace ffmpegcpp
{
    RawAudioFileSource::RawAudioFileSource(const char * fileName
        , const char * inputFormat, int sampleRate, int channels
        , FrameSink * frameSink)
	{
        // fileName = hw::1,0 input format = alsa, sampleRate = 48000 (default)  channels= 2 (default)  frameSink = ? AudioDecoder() ?

		// try to deduce the input format from the input format name
        const AVInputFormat *file_iformat;
        if (!(file_iformat = av_find_input_format(inputFormat)))
		{
			CleanUp();
			throw FFmpegException(std::string("Unknown input format: " + string(inputFormat)).c_str());
		}

		AVDictionary* format_opts = NULL;

		av_dict_set_int(&format_opts, "sample_rate", sampleRate, 0);
		av_dict_set_int(&format_opts, "channels", channels, 0);
		av_dict_set_int(&format_opts, "b", 256000, 0);
		av_dict_set_int(&format_opts, "minrate", 400000, 0);
		av_dict_set_int(&format_opts, "maxrate", 400000, 0);

		// create the demuxer
		try
        {
            m_demuxer = new Demuxer(fileName, file_iformat, format_opts);
            m_demuxer->DecodeBestAudioStream(frameSink);
		}
        catch (const FFmpegException & e)
		{
			CleanUp();
			throw e;
		}
	}

	RawAudioFileSource::~RawAudioFileSource()
	{
		CleanUp();
	}

	void RawAudioFileSource::CleanUp()
    {
        if (m_demuxer != nullptr)
        {
            delete m_demuxer;
            m_demuxer = nullptr;
		}
	}

	void RawAudioFileSource::PreparePipeline()
    {
        m_demuxer->PreparePipeline();
	}

	bool RawAudioFileSource::IsDone()
    {
        return m_demuxer->IsDone();
	}

	void RawAudioFileSource::Step()
    {
        m_demuxer->Step();
	}
}

