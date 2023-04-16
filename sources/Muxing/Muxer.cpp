#include "Muxer.h"
#include "FFmpegException.h"
#include "OutputStream.h"
#include "CodecDeducer.h"

#include <string>

using namespace std;

namespace ffmpegcpp
{

	Muxer::Muxer(const std::string & fileName)
	{
        m_fileName = fileName;
#ifdef DBBUG
                std::cerr  <<  "Currently in : "  << __func__ << "   creating new muxer " <<  "\n";
#endif
        /* allocate the output media context */
                avformat_alloc_output_context2(&m_containerContext, NULL, NULL, fileName.c_str());
#ifdef DBBUG
                std::cerr  <<  "avformat_alloc_output_context2 done "  <<  "\n";
                std::cerr  <<  "containerContext =  "  << containerContext << "\n";
#endif
                if (!m_containerContext)
		{
            printf("WARNING: Could not deduce output format from file extension: using MP4. as default\n");
                    avformat_alloc_output_context2(&m_containerContext, NULL, "mp4", fileName.c_str());
		}

                if (!m_containerContext)
		{
            throw FFmpegException(std::string("Could not allocate container context for " + this->m_fileName).c_str());
		}

        // the format of the container - not necessarily the same as the fileName suggests, see above
        m_containerFormat = m_containerContext->oformat;
	}

	Muxer::~Muxer()
	{
		CleanUp();
	}

	void Muxer::CleanUp()
	{
		// see if we were primed - we will use this info below
		bool wasPrimed = IsPrimed();

        // clean up all the output streams that were added to this muxer.
        for (unsigned int i = 0; i < m_outputStreams.size(); ++i)
               {
            m_outputStreams[i] = nullptr;
               }
        m_outputStreams.clear();

        // clean up the container context - this will also finalize the content of the container!
        if (m_containerContext != nullptr && wasPrimed)
		{
            // If we don't ALWAYS do this, we leak memory!
            av_write_trailer(m_containerContext);

            if (!(m_containerFormat->flags & AVFMT_NOFILE))
                /* Close the output file. */
                avio_closep(&m_containerContext->pb);


            avformat_free_context(m_containerContext);
            m_containerContext = nullptr;
		}

        // clean up the queue
        for (unsigned int i = 0; i < m_packetQueue.size(); ++i)
        {
            AVPacket* tmp_pkt = m_packetQueue[i];
			av_packet_free(&tmp_pkt);
        }
        m_packetQueue.clear();
	}

    const AVCodec* Muxer::GetDefaultVideoFormat()
    {
        return CodecDeducer::DeduceEncoder(m_containerFormat->video_codec);
	}

    const AVCodec* Muxer::GetDefaultAudioFormat()
    {
        return CodecDeducer::DeduceEncoder(m_containerFormat->audio_codec);
	}

	void Muxer::AddOutputStream(OutputStream* outputStream)
    {
        if (m_opened) throw FFmpegException(std::string("You cannot open a new stream after something was written to the muxer").c_str());

        // create an internal stream and pass it on
        AVStream* stream = avformat_new_stream(m_containerContext, NULL);
		if (!stream)
        {
            throw FFmpegException(std::string("Could not allocate stream for container " + std::string(m_containerContext->oformat->name)).c_str());
		}

        stream->id = m_containerContext->nb_streams - 1;

        outputStream->OpenStream(stream, m_containerContext->oformat->flags);

        m_outputStreams.push_back(outputStream);
	}

	bool Muxer::IsPrimed()
    {
        if (m_opened) return true; // we were already opened before - always primed from now on!
        bool allPrimed = true;
        for (unsigned int i = 0; i < m_outputStreams.size(); ++i)
        {
            if (!m_outputStreams[i]->IsPrimed()) allPrimed = false;
		}

		// we are finally primed - open ourselves before we continue.
		if (allPrimed)
		{
			// if we are all primed
            Open();
            m_opened = true;
			//printf("After %d cached packets, we can finally open the container\n", packetQueue.size());
		}
		return allPrimed;
	}

	void Muxer::WritePacket(AVPacket* pkt)
	{
		// The muxer needs to buffer all the packets UNTIL all streams are primed
		// at that moment, we can actually open ourselves!
		// Because of this, we need to call PreparePipeline on all input sources BEFORE
		// we start running the actual data through. This pipeline preparation step
        // pushes one frame down the pipeline so that the output can be configured properly.
        if (!m_opened)
		{
			throw FFmpegException(std::string("You cannot submit a packet to the muxer until all output streams are fully primed!").c_str());
		}

        // submit this packet
        int ret = av_interleaved_write_frame(m_containerContext, pkt);
		if (ret < 0)
		{
			throw FFmpegException(std::string("Error while writing frame to output container").c_str(), ret);
		}

		return;

        if (!m_opened)
		{
			// we CAN open now - all streams are primed and ready to go!
			if (IsPrimed())
			{
                Open();
                m_opened = true;
                printf("After %lu cached packets, we can finally open the container\n", m_packetQueue.size());

                // flush the queue
                for (unsigned int i = 0; i < m_packetQueue.size(); ++i)
                {
                    AVPacket* tmp_pkt = m_packetQueue[i];

					// Write the compressed frame to the media file.
                    ret = av_interleaved_write_frame(m_containerContext, tmp_pkt);
					if (ret < 0)
					{
						throw FFmpegException(std::string("Error while writing frame to output container").c_str(), ret);
					}

					av_packet_unref(tmp_pkt);
					av_packet_free(&tmp_pkt);
                }
                m_packetQueue.clear();
			}

			// not ready - buffer the packet
			else
			{
				AVPacket* tmp_pkt = av_packet_alloc();
				if (!tmp_pkt)
				{
					throw FFmpegException(std::string("Failed to allocate packet").c_str());
				}
                av_packet_ref(tmp_pkt, pkt);
                m_packetQueue.push_back(tmp_pkt);
			}
		}

        // we are opened now - write this packet!
        if (m_opened)
		{
            ret = av_interleaved_write_frame(m_containerContext, pkt);
			if (ret < 0)
			{
				throw FFmpegException(std::string("Error while writing frame to output container").c_str(), ret);
			}
		}
	}

	void Muxer::Open()
	{
        // open the output file, if needed
        if (!(m_containerFormat->flags & AVFMT_NOFILE))
		{
            int ret = avio_open(&m_containerContext->pb, m_fileName.c_str(), AVIO_FLAG_WRITE);
			if (ret < 0)
			{
                throw FFmpegException(std::string("Could not open file for container " + m_fileName).c_str(), ret);
			}
		}

        // Write the stream header, if any.
        int ret = avformat_write_header(m_containerContext, NULL);
		if (ret < 0)
		{
            throw FFmpegException(std::string("Error when writing header to output file " + m_fileName).c_str(), ret);
		}
	}

	void Muxer::Close()
	{
		// if some of the output streams weren't primed, we cannot finish this process
		if (!IsPrimed())
		{
			throw FFmpegException(std::string("You cannot close a muxer when one of the streams wasn't primed. You need to make sure all streams are primed before closing the muxer.").c_str());
		}

		// Make sure we drain all the output streams before we write the first packet.
		// We must be sure to do this because in an extreme case, one entire stream
		// might be queueing all its packets before we are opened, so it might not
        // be draining them at all.
        for (unsigned int i = 0; i < m_outputStreams.size(); ++i)
        {
            m_outputStreams[i]->DrainPacketQueue();
		}

		// free the stream
		CleanUp();
	}
}
