#include "VoiceDecode.h"
#include "GtooLogger.h"


extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
//#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

VoiceDecode::VoiceDecode() {

}

VoiceDecode::~VoiceDecode() {

}

#define MAX_AUDIO_FRAME_SIZE 19200

AVCodecContext* pCodecCtx = nullptr;
AVPacket* packet = nullptr;
AVFrame* pFrame = nullptr;
uint8_t *out_buffer;
struct SwrContext *auConvertCtx;

static uint8_t *audio_chunk;
static uint32_t audio_len;
static uint8_t *audio_pos;

//typedef void (SDLCALL * SDL_AudioCallback) (void *userdata, Uint8 * stream,
//    int len);
void read_audio_data(void *udata, Uint8 *stream, int len) {
    fprintf(stderr, "stream addr:%p, audio_len:%d, len:%d\n",
        stream,
        audio_len,
        len);
    //����ʹ��SDL_memset()��stream�е���������Ϊ0
    SDL_memset(stream, 0, len);
    if (audio_len == 0)
        return;
    len = (len > audio_len ? audio_len : len);

    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}


bool VoiceDecode::open(const QString& url) {
    //QString newUrl("E:\\Desktop\\languguetest\\Cplusplustest\\3-VisualStudio2017\\0-GtooPlayer\\test_video\\123.mp3");
    if (url.isNull())
        return false;

    AVDictionary* dict = nullptr;
    av_dict_set(&dict, "rtsp_transport", "tcp", 0);      // ����rtsp��ʹ��tcp�򿪣������ʧ�ܴ�����ϢΪ��Error number -135 occurred�������л���UDP��tcp��udp_multicast��http��������vlc��������Ҫʹ��udp��
    av_dict_set(&dict, "max_delay", "3", 0);             // ��������û�⸴���ӳ٣���΢��Ϊ��λ������ͨ����UDP�� ��������ʱ���⸴������������������յ������ݰ�����Ϊ���ǿ������򵽴�������ݰ�������ȫ��ʧ���������ͨ�������⸴���ӳ�����Ϊ�㣨ͨ��max_delayAVFormatContext �ֶΣ������á�
    av_dict_set(&dict, "timeout", "1000000", 0);         // ��΢��Ϊ��λ�����׽��� TCP I/O ��ʱ������ȴ�ʱ����̣�Ҳ���ܻỹû���Ӿͷ����ˡ�

    // �������������ؽ��װ������
    int ret = avformat_open_input(&pFormatCtx, url.toStdString().data(), nullptr, &dict);
    if (ret < 0){
        LOG_ERROR("Failed to open video file! avformat_open_input {:d}\n", ret);
        return false;
    }
    if (dict)// �ͷŲ����ֵ�
    {
        av_dict_free(&dict);
    }

    ret = avformat_find_stream_info(pFormatCtx, nullptr);// ��ȡý���ļ������ݰ��Ի�ȡ����Ϣ
    // ���Բ�����
    // ��ȡ����ʱ������Ϣ
    qint64 totalTime = pFormatCtx->duration / (AV_TIME_BASE / 1000);// ������Ƶ��ʱ�������룩


    mVoiceIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (mVoiceIndex < 0) {
        LOG_ERROR("Din't find a video stream! av_find_best_stream {:d}\n", mVoiceIndex);
        return false;
    }

    pCodecPara = pFormatCtx->streams[mVoiceIndex]->codecpar;

    const AVCodec* pCodec = avcodec_find_decoder(pCodecPara->codec_id);
    if (pCodec == nullptr) {
        LOG_ERROR("Unsupported codec! avcodec_find_decoder {:d}\n", mVoiceIndex);
        return false;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    ret = avcodec_parameters_to_context(pCodecCtx, pCodecPara);
    if (ret < 0) {
        LOG_ERROR("Couldn't copy codec context! avcodec_parameters_to_context {:d}\n", ret);
        return false;
    }
    ret = avcodec_open2(pCodecCtx, pCodec, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to open decoder! avcodec_open2 {:d}\n", ret);
        return false;
    }
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    pFrame = av_frame_alloc();


    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;  //���������

    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;//�����ʽS16

    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    int out_sample_rate = 44100;
    int out_nb_samples = 1024;

    int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
    out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG_ERROR("Could not initialize SDL {:s}\n", SDL_GetError());
        return false;
    }


    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = out_channels;
    spec.silence = 0;
    spec.samples = 1024;
    spec.callback = read_audio_data;
    spec.userdata = pCodecCtx;

    //SDL_AudioSpec spec;
    //spec.freq = 44100;//������¼�Ƶ�PCM�����ʾ���
    //spec.format = AUDIO_S16SYS;
    //spec.channels = 1; //������
    //spec.silence = 0;
    //spec.samples = 1024;
    //spec.callback = read_audio_data2;
    //spec.userdata = NULL;

    int64_t in_channel_layout;
    in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    printf("in_channel_layout --->%d\n", in_channel_layout);
    auConvertCtx = swr_alloc();
    auConvertCtx = swr_alloc_set_opts(auConvertCtx, out_channel_layout, out_sample_fmt, out_sample_rate,
        in_channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0, NULL);
    swr_init(auConvertCtx);


    if (SDL_OpenAudio(&spec, NULL) < 0) {
        printf("can't open audio.\n");
        return false;
    }


    SDL_PauseAudio(0);
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        if (packet->stream_index == mVoiceIndex) {
            avcodec_send_packet(pCodecCtx, packet);
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                swr_convert(auConvertCtx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data,
                    pFrame->nb_samples); // ת����Ƶ
            }

            audio_chunk = (Uint8 *)out_buffer;
            audio_len = out_buffer_size;
            audio_pos = audio_chunk;
            LOG_DEBUG("audio_len:{:d}", audio_len);
            while (audio_len > 0) {
                SDL_Delay(1);//�ӳٲ���
            }
        }
        av_packet_unref(packet);
    }
    swr_free(&auConvertCtx);
    SDL_Quit();

}