#include <sstream>
#include <alsa/asoundlib.h>
#include <jsoncpp/json.hpp>

/* Compile:
 * g++ -o listsoundcardids listsoundcardids.cpp `pkg-config --cflags --libs alsa jsoncpp`
 *
 * Usage:
 * listsoundcardids
 *
 * Outputs something like:
 * {
 *    "cards" : [
 *       {
 *          "alsa_address" : "hw:CARD=Intel,DEV=0",
 *          "card_id" : "Intel",
 *          "card_name" : "HDA Intel",
 *          "device_id" : "VT1708S Analog"
 *       },
 *       {
 *          "alsa_address" : "hw:CARD=Intel,DEV=1",
 *          "card_id" : "Intel",
 *          "card_name" : "HDA Intel",
 *          "device_id" : "VT1708S Digital"
 *       },
 *       {
 *          "alsa_address" : "hw:CARD=Intel,DEV=2",
 *          "card_id" : "Intel",
 *          "card_name" : "HDA Intel",
 *          "device_id" : "VT1708S HP"
 *       },
 *       {
 *          "alsa_address" : "hw:CARD=NVidia,DEV=3",
 *          "card_id" : "NVidia",
 *          "card_name" : "HDA NVidia",
 *          "device_id" : "HDMI 0"
 *       },
 *       {
 *          "alsa_address" : "hw:CARD=NVidia,DEV=7",
 *          "card_id" : "NVidia",
 *          "card_name" : "HDA NVidia",
 *          "device_id" : "HDMI 1"
 *       }
 *    ]
 * }
 */

static int list_soundcard_ids(Json::Value &Cards)
{
  int count = 0;
  snd_ctl_card_info_t *card_info;
  snd_pcm_info_t      *pcm_info = NULL;

  if (snd_ctl_card_info_malloc(&card_info) < 0)
     return -1;

  if (snd_pcm_info_malloc(&pcm_info) < 0) {
     snd_ctl_card_info_free(card_info);
     return -1;
     }

  int card_index = -1;
  do {
       if ((snd_card_next(&card_index) < 0) || (card_index < 0))
          break;

       std::ostringstream hw_name;
       hw_name << "hw:" << card_index;
       snd_ctl_t *handle = NULL;
       if (snd_ctl_open(&handle, hw_name.str().c_str(), 0) == 0) {
          if (snd_ctl_card_info(handle, card_info) == 0) {
             char *tmp = NULL;
             snd_card_get_name(card_index, &tmp);
             std::string card_name = tmp;
             free(tmp);
             std::string card_id = snd_ctl_card_info_get_id(card_info);

             int device_index = -1;
             do {
                  if ((snd_ctl_pcm_next_device(handle, &device_index) < 0) || (device_index < 0))
                     break;

                  memset(pcm_info, 0, snd_pcm_info_sizeof());
                  snd_pcm_info_set_device(pcm_info, device_index);
                  snd_pcm_info_set_subdevice(pcm_info, 0);
                  snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);
                  if (snd_ctl_pcm_info(handle, pcm_info) == 0) {
                     std::string device_id = snd_pcm_info_get_id(pcm_info);
                     std::ostringstream alsa_address;
                     alsa_address << "hw:CARD=" << card_id << ",DEV=" << device_index;

                     Json::Value card;
                     card["card_name"] = card_name;
                     card["card_id"] = card_id;
                     card["device_id"] = device_id;
                     card["alsa_address"] = alsa_address.str();
                     Cards.append(card);
                     count++;
                     }
                } while (true);
             }
          snd_ctl_close(handle);
          }
     } while (true);

  if (pcm_info)
     snd_pcm_info_free(pcm_info);
  if (card_info)
     snd_ctl_card_info_free(card_info);
  snd_config_update_free_global();
  return count;
}

int main(int argc, char *argv[])
{
  Json::Value cards;
  int count = list_soundcard_ids(cards);
  Json::Value alsacards;
  alsacards["cards"] = cards;
  Json::StyledWriter styledWriter;
  std::cout << styledWriter.write(alsacards);
  return 0;
}
