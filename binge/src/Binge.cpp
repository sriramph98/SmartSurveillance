#include <binge/Binge.hpp>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#ifdef BINGE_LOG_ENABLED
#include <libavutil/log.h>
#endif
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

#include <clog/clog.h>
#include <ctime>

#include <binge/ContextParser.hpp>

using namespace binge;

static const char* TAG = "BINGE";

static int av_err = 0;

static void handle_av_err(Binge* b, int av_err) {
  const char *errMsg = av_err2str(av_err);
  clog_e(TAG, errMsg);
  b->fireEvent(FrameReaderStates::ERROR, Frame());
}

Binge::Binge() {
  av_register_all();
}

void Binge::read(const char* input) {
#ifdef BINGE_LOG_ENABLED
  av_log_set_level(AV_LOG_DEBUG);
#endif

  AVFormatContext* inctx = nullptr;

  av_err = avformat_open_input(&inctx, input, nullptr, nullptr);
  if (av_err < 0) {
    return handle_av_err(this, av_err);
  }

  av_err = avformat_find_stream_info(inctx, nullptr);
  if (av_err < 0) {
    handle_av_err(this, av_err);
    fireEvent(FrameReaderStates::ERROR, Frame());
  }
    // find primary video stream
    AVCodec* vcodec = nullptr;
    av_err = av_find_best_stream(inctx, AVMEDIA_TYPE_VIDEO, -1, -1, &vcodec, 0);
    if (av_err < 0) {   
       return handle_av_err(this, av_err);
    }
    const int vstrm_idx = av_err;
    AVStream* vstrm = inctx->streams[vstrm_idx];

    // open video decoder context
    av_err = avcodec_open2(vstrm->codec, vcodec, nullptr);
    if (av_err < 0) {
      return handle_av_err(this, av_err);
    }

    // initialize sample scaler
    const int dst_width = vstrm->codec->width;
    const int dst_height = vstrm->codec->height;
    const AVPixelFormat dst_pix_fmt = AV_PIX_FMT_BGR24;
    SwsContext* swsctx = sws_getCachedContext(
        nullptr, vstrm->codec->width, vstrm->codec->height, vstrm->codec->pix_fmt,
        dst_width, dst_height, dst_pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsctx) {
      return handle_av_err(this, av_err);
    }
  //  std::cout << "output: " << dst_width << 'x' << dst_height << ',' << av_get_pix_fmt_n ame(dst_pix_fmt) << std::endl;

    // allocate frame buffer for output
    AVFrame* frame = av_frame_alloc();
    std::vector<uint8_t> framebuf(avpicture_get_size(dst_pix_fmt, dst_width, dst_height));
    avpicture_fill(reinterpret_cast<AVPicture*>(frame), framebuf.data(), dst_pix_fmt, dst_width, dst_height);

    // decoding loop`
    AVFrame* decframe = av_frame_alloc();
    unsigned nb_frames = 0;
    bool end_of_stream = false;
    int got_pic = 0;
    AVPacket pkt;
    do {
        if (!end_of_stream) {
            // read packet from input file
            av_err = av_read_frame(inctx, &pkt);
            if (av_err < 0 && av_err != AVERROR_EOF) {
              return handle_av_err(this, av_err); 
            }
            if (av_err == 0 && pkt.stream_index != vstrm_idx)
                goto next_packet;
            end_of_stream = (av_err == AVERROR_EOF);
        }
        if (end_of_stream) {
            // null packet for bumping process
            av_init_packet(&pkt);
            pkt.data = nullptr;
            pkt.size = 0;
        }
        // decode video frame
        avcodec_decode_video2(vstrm->codec, decframe, &got_pic, &pkt);
        if (!got_pic)
            goto next_packet;
        // frames!!!
        sws_scale(swsctx, decframe->data, decframe->linesize, 0, decframe->height, frame->data, frame->linesize);
        {
          Frame *f = new Frame();
          f->timestamp = ((double)decframe->best_effort_timestamp) / 1000 ; //Returns timestamp 
          f->frameData = new cv::Mat (dst_height, dst_width, CV_8UC3, framebuf.data(), frame->linesize[0]);
          std::cout<<"Converted to mat"<<std::endl; 
          std::cout<<f<<std::endl; // Mat values are printed
          f->frameSize = (FrameSize){dst_width, dst_height};
          bool status = ContextParser::getInstance()->parse(f);
          if(status){
            fireEvent(FrameReaderStates::READ, *f);
          }
        }
        ++nb_frames;
next_packet:
        av_free_packet(&pkt);
    } while (!end_of_stream || got_pic);

    av_frame_free(&decframe);
    av_frame_free(&frame);
    avcodec_close(vstrm->codec);
    avformat_close_input(&inctx);

  fireEvent(FrameReaderStates::COMPLETE, Frame());
}

void Binge::read(std::string input) {
  return this->read(input.c_str());
}

Binge::~Binge() {
  // std::cout
  //     << "waitFor not called, please use waitFor to suppress this error message"
  //     << std::endl;
}