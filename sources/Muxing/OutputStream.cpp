#include "OutputStream.h"
#include "FFmpegException.h"

using namespace std;

namespace ffmpegcpp
{
    OutputStream::OutputStream(Muxer * muxer, Codec * codec)
    : m_codec(codec)
    , m_muxer(muxer)
    {
	}

    void OutputStream::SendPacketToMuxer(AVPacket * pkt)
	{
		// if the muxer is primed, we submit the packet for real
		if (m_muxer->IsPrimed())
		{
			// drain the queue
			DrainPacketQueue();

			// send this packet
			PreparePacketForMuxer(pkt);
			m_muxer->WritePacket(pkt);
		}

		// otherwise, we queue the packet
		else
		{
			AVPacket* tmp_pkt = av_packet_alloc();
			if (!tmp_pkt)
			{
				throw FFmpegException("Failed to allocate packet");
			}
            av_packet_ref(tmp_pkt, pkt);
            m_packetQueue.push_back(tmp_pkt);
		}
	}

	void OutputStream::DrainPacketQueue()
    {
        if (m_packetQueue.size() > 0) printf("Drain %lu packets from the packet queue...", m_packetQueue.size());
        for (unsigned int i = 0; i < m_packetQueue.size(); ++i)
        {
            AVPacket* tmp_pkt = m_packetQueue[i];

			// Write the compressed frame to the media file
			PreparePacketForMuxer(tmp_pkt);
			m_muxer->WritePacket(tmp_pkt);

			// Release the packet
			av_packet_unref(tmp_pkt);
			av_packet_free(&tmp_pkt);
		}

        m_packetQueue.clear();
	}
}
