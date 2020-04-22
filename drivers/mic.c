
/**
 * @desc: MIC 音频驱动
 */

#include <alsa/asoundlib.h>
#include <elog.h>

#define DEFAULT_RECORD_DEVICE "default"

void record_init(void)
{
    int err;
    snd_pcm_t *handle;
    char *device = (char *)DEFAULT_RECORD_DEVICE;
    err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
        log_e("Unable to open PCM device: %s\n", snd_strerror(err));
}