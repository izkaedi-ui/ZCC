#define SNDCTL_DSP_SETFMT 0
#define SNDCTL_DSP_CHANNELS 0
#define SNDCTL_DSP_SPEED 0
#define SNDCTL_DSP_SETFRAGMENT 0
#define SNDCTL_DSP_GETOSPACE 0
#define AFMT_U8 0
#define AFMT_S16_LE 0
typedef struct audio_buf_info { int bytes; int fragments; int fragstotal; int fragsize; } audio_buf_info;
