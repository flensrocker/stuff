#include <alsa/asoundlib.h>

/* Compile:
 * gcc -o listsoundcardids listsoundcardids.c `pkg-config --cflags --libs alsa`
 *
 * Usage:
 * listsoundcardids
 *
 * Outputs something like:
 * Intel:VT1708S Analog
 * Intel:VT1708S Digital
 * Intel:VT1708S HP
 * NVidia:HDMI 0
 * NVidia:HDMI 1
 */

static int count_strings(char **Array)
{
  int count = 0;
  if (Array == NULL)
     return -1;
  while ((*(Array + count) != NULL))
        count++;
  return count;
}

static void free_array(char **Array)
{
  int i = 0;
  if (Array == NULL)
     return;
  while ((*(Array + i) != NULL)) {
        free(Array[i]);
        i++;
        }
  free(Array);
}

static void add_string(char ***Array, const char *String)
{
  int count = 0;
  if ((Array == NULL) || (String == NULL))
     return;
  if (*Array == NULL)
     *Array = malloc(2 * sizeof(char*));
  else {
     count = count_strings(*Array);
     *Array = realloc(*Array, (count + 2) * sizeof(char*));
     }
  (*Array)[count] = strdup(String);
  (*Array)[count + 1] = NULL;
}

static int list_soundcard_ids(char ***CardId, char ***DeviceId)
{
  int count = 0;
  char       hwname[32];
  snd_ctl_t *handle = NULL;

  int                  card_index = -1;
  const char          *card_id = NULL;
  snd_ctl_card_info_t *card_info;

  int                  device_index = -1;
  const char          *device_id = "";
  snd_pcm_info_t      *pcm_info = NULL;

  *CardId = NULL;
  *DeviceId = NULL;

  if (snd_ctl_card_info_malloc(&card_info) < 0) {
     count = -1;
     goto close;
     }
  if (snd_pcm_info_malloc(&pcm_info) < 0) {
     count = -1;
     goto close;
     }

  do {
       if ((snd_card_next(&card_index) < 0) || (card_index < 0))
          break;

       sprintf(hwname, "hw:%d", card_index);
       if (snd_ctl_open(&handle, hwname, 0) == 0) {
          if (snd_ctl_card_info(handle, card_info) == 0) {
             card_id = snd_ctl_card_info_get_id(card_info);

             device_index = -1;
             do {
                  if ((snd_ctl_pcm_next_device(handle, &device_index) < 0) || (device_index < 0))
                     break;

                  memset(pcm_info, 0, snd_pcm_info_sizeof());
                  snd_pcm_info_set_device(pcm_info, device_index);
                  snd_pcm_info_set_subdevice(pcm_info, 0);
                  snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);
                  if (snd_ctl_pcm_info(handle, pcm_info) == 0) {
                     device_id = snd_pcm_info_get_id(pcm_info);
                     add_string(CardId, card_id);
                     add_string(DeviceId, device_id);
                     count++;
                     }
                } while (1);
             }
          snd_ctl_close(handle);
          }
     } while (1);

close:
  if (pcm_info)
     snd_pcm_info_free(pcm_info);
  if (card_info)
     snd_ctl_card_info_free(card_info);
  snd_config_update_free_global();
  return count;
}

int main(int argc, char *argv[])
{
  char **CardId = NULL;
  char **DeviceId = NULL;
  int count = 0;
  int i = 0;
  
  count = list_soundcard_ids(&CardId, &DeviceId);
  for (i = 0; i < count; i++)
      printf("%s:%s\n", CardId[i], DeviceId[i]);

  free_array(CardId);
  free_array(DeviceId);
  return 0;
}
