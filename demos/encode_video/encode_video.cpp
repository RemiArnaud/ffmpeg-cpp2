#include <iostream>
#include "ffmpegcpp.h"

using std::cerr;
using std::cout;

using namespace ffmpegcpp;

//TODO : use existing interface instead ?

class PGMFileSink : public VideoFrameSink, public FrameWriter
{
public:

    PGMFileSink()
    {
    }

    FrameSinkStream* CreateStream()
    {
        stream = new FrameSinkStream(this, 0);
        return stream;
    }

    virtual void WriteFrame(int /* streamIndex */, AVFrame* frame, StreamData*  /* streamData */)
    {
        ++frameNumber;
        printf("saving frame %3d\n", frameNumber);
        fflush(stdout);

        // write the first channel's color data to a PGM file.
        // This raw image file can be opened with most image editing programs.
        snprintf(fileNameBuffer, sizeof(fileNameBuffer), "frames/frame-%d.pgm", frameNumber);
        pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, fileNameBuffer);
    }

    void pgm_save(unsigned char *bufy, int wrap, int xsize, int ysize, char *filename)
    {
        FILE *f;
        int i;
        f = fopen(filename, "w");

        fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

        for (i = 0; i < ysize; i++)
        {
            fwrite(bufy + i * wrap, 1, xsize, f);
        }
        fclose(f);
    }

    virtual void Close(int /* streamIndex */)
    {
        delete stream;
    }

    virtual bool IsPrimed()
    {
        // Return whether we have all information we need to start writing out data.
        // Since we don't really need any data in this use case, we are always ready.
        // A container might only be primed once it received at least one frame from each source
        // it will be muxing together (see Muxer.cpp for how this would work then).
        return true;
    }

private:
    char fileNameBuffer[1024];
    int frameNumber = 0;
    FrameSinkStream* stream;

};



int main()
{
    avdevice_register_all();

    // This example is a work in progress, means unfinished.
    // DONE:
    // - detect the webcam
    // - set the right parameters (for instance, 1920x1080@30fps, Logitech C920 / C922 / Brio  (other webcams untested)
    // - activate the webcam
    // - create the stream
    // - create the demuxer
    // - create the frames
    // - convert them into grey (type 5) .ppm
    //
    // - TODO:
    // - convert YUV frames into RGB frames
    // - initialize  the encoder and build the video at the end.
    // - create the video (.mpg or whatever)
    // - improve, and use better the implementation
    //
    // GOAL : 
    // This program aims to take the stream from a webcam  (mjpeg stream),
    // turn it into raw frames, and encode it into as VP9 video (HEVC codec).


    try
    {
        // Create a muxer that will output the video as MKV.
        Muxer* muxer = new Muxer("webcam_to_VP9.mpg");

        std::cerr << "Using MJPEG input Codec" << "\n";

        MJPEGCodec * codec = new MJPEGCodec();

        //  OUTPUT CODEC, linked to the encoder ...
        H264Codec  * vcodec = new H264Codec();
        vcodec->SetGenericOption("b", "2M");
        vcodec->SetGenericOption("bit_rate", "2M");
        vcodec->SetProfile("high10"); // baseline, main, high, high10, high422
        vcodec->SetTune("film");  // film animation grain stillimage psnr ssim fastdecode zerolatency
        vcodec->SetPreset("veryslow"); // fast, medium, slow slower, veryslow placebo
        vcodec->SetCrf(23);

        int width = 1280;
        int height = 720;
        AVRational frameRate = {1, 30};
        AVPixelFormat pix_format = AV_PIX_FMT_YUVJ420P; // = V4L2_PIX_FMT_MJPEG

        codec->Open(width, height, &frameRate, pix_format);

        //VideoEncoder * vEncoder = new VideoEncoder(codec, muxer);

        PGMFileSink* fileSink = new PGMFileSink();
        RawVideoFileSource* videoFile = new RawVideoFileSource("/dev/video0", 1280, 720, frameRate.den, pix_format);
        videoFile->setFrameSink(fileSink);;
        videoFile->demuxer->DecodeBestVideoStream(fileSink);

        // Prepare the output pipeline. This will push a small amount of frames to the file sink until it IsPrimed returns true.
        videoFile->PreparePipeline();
        std::cerr << "videoFile->PreparePipeline() done ..." << "\n";

        // Push all the remaining frames through.
        while (!videoFile->IsDone())
        {
            videoFile->Step();
        }
        // Save everything to disk by closing the muxer.
        muxer->Close();
    }

    catch (FFmpegException e)
    {
        cerr << "Exception caught!" << "\n";
        cerr << e.what() << "\n";
        throw e;
    }
    cout << "Encoding complete!" << "\n";
}
