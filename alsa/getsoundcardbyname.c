#include <alsa/asoundlib.h>

/* Compile:
 * gcc -o getsoundcardbyname getsoundcardbyname.c `pkg-config --cflags --libs alsa`
 *
 * Usage:
 * getsoundcardbyname "HDA NVidia" "HDMI 1"
 *
 * Outputs something like:
 * hw:1,7
 */

int main(int argc, char *argv[])
{
  int rc = 0;

  const char *search_name = "";
  const char *search_device_id = "";

  char       hwname[32];
  snd_ctl_t *handle = NULL;

  int   card = -1;
  char *name = NULL;
  const char          *card_name = NULL;
  snd_ctl_card_info_t *card_info;

  int                  device = -1;
  const char          *device_id = "";
  snd_pcm_info_t      *pcminfo = NULL;

  if (argc < 3) {
     printf("usage: %s <card-name> <device-id>\n", argv[0]);
     rc = -1;
     goto close;
     }

  search_name = argv[1];
  search_device_id = argv[2];

  snd_ctl_card_info_alloca(&card_info);
  do {
       if ((snd_card_next(&card) < 0) || (card < 0)) {
          fprintf(stderr, "card %s not found\n", search_name);
          rc = -1;
          goto close;
          }
       if (snd_card_get_name(card, &name) < 0) {
          fprintf(stderr, "can't get name of card %d\n", card);
          rc = -1;
          goto close;
          }
       if (strcmp(search_name, name) == 0)
          break;

       sprintf(hwname, "hw:%d", card);
       if (snd_ctl_open(&handle, hwname, 0) == 0) {
          if (snd_ctl_card_info(handle, card_info) == 0) {
             card_name = snd_ctl_card_info_get_id(card_info);
             if (card_name && (strcmp(search_name, card_name) == 0))
                break;
             }
          snd_ctl_close(handle);
          }
     } while (1);
  if (name)
     free(name);

  sprintf(hwname, "hw:%d", card);

  if (snd_ctl_open(&handle, hwname, 0) < 0) {
     fprintf(stderr, "can't open control for %s\n", hwname);
     rc = -1;
     goto close;
     }

  snd_pcm_info_alloca(&pcminfo);
  do {
       if ((snd_ctl_pcm_next_device(handle, &device) < 0) || (device < 0)) {
          fprintf(stderr, "device %s not found\n", search_device_id);
          rc = -1;
          goto close;
          }
       snd_pcm_info_set_device(pcminfo, device);
       snd_pcm_info_set_subdevice(pcminfo, 0);
       snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
       if (snd_ctl_pcm_info(handle, pcminfo) < 0) {
          fprintf(stderr, "can't get pcminfo of card %d, device %d\n", card, device);
          rc = -1;
          goto close;
          }
       device_id = snd_pcm_info_get_id(pcminfo);
       if (strcmp(search_device_id, device_id) == 0)
          break;
     } while (1);

  printf("hw:%d,%d", card, device);

close:
  if (handle)
     snd_ctl_close(handle);
  snd_config_update_free_global();
  return rc;
}
