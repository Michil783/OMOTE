// #include <Arduino.h>
#include <SamsungTV.hpp>
#include <DisplayAbstract.h>
#include <Settings.hpp>

// #include <omote.hpp>

#ifdef OMOTE_ESP32
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Preferences.h>
extern Preferences preferences;
extern IRrecv IrReceiver;
extern IRsend IrSender;
#endif

SamsungTV::SamsungTV(std::shared_ptr<DisplayAbstract> display)
{
  LV_LOG_USER(">>> SamsungTV::SamsungTV()");
  mDisplay = display;
  UI::Basic::OmoteUI::getInstance()->addDevice(this);

  MessageSend("/api/v2/");
  LV_LOG_USER("<<< SamsungTV::SamsungTV()");
}

u_int64_t SamsungTV::getValue(char keyChar)
{
  for (size_t i = 0; i < sizeof(this->samsungKeys) / sizeof(SamsungKeys); i++)
  {
    if (samsungKeys[i].key == keyChar)
    {
      LV_LOG_TRACE("key %c found: data is %X", samsungKeys[i].key, samsungKeys[i].code);
      return this->samsungKeys[i].code;
    }
  }
  return -1;
}

void SamsungTV::handleCustomKeypad(int keyCode, char keyChar)
{
  LV_LOG_TRACE(">>> SamsungTV::handleCustomKeypad(%x, %c)", keyCode, keyChar);
  LV_LOG_TRACE("handleCustomKeypad(%d, %c)", keyCode, keyChar);
  uint64_t code = getValue(keyChar);
#ifdef OMOTE_ESP32
  if (code != -1)
    IrSender.sendSAMSUNG(code);
#endif
  LV_LOG_TRACE("<<< SamsungTV::handleCustomKeypad()");
}

lv_obj_t *SamsungTV::mSubPage;
lv_obj_t *SamsungTV::mContent;

void SamsungTV::displaySettings(lv_obj_t *menu, lv_obj_t *parent)
{
  LV_LOG_TRACE(">>> SamsungTV::displaySettings()");
  lv_color_t primary_color = lv_color_hex(0x303030); // mDisplay->getPrimaryColor();
  lv_obj_t *menuLabel;

  menuLabel = lv_label_create(parent);
  lv_label_set_text(menuLabel, getName().c_str());

  lv_obj_t *menuBox = lv_obj_create(parent);
  lv_obj_set_size(menuBox, lv_pct(100), 80);
  lv_obj_set_style_bg_color(menuBox, primary_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(menuBox, 0, LV_PART_MAIN);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, "IP:");
  lv_obj_align(menuLabel, LV_ALIGN_TOP_LEFT, 0, 3);

  menuLabel = lv_label_create(menuBox);
  lv_label_set_text(menuLabel, this->ip.c_str());
  lv_obj_align(menuLabel, LV_ALIGN_TOP_RIGHT, 0, 3);

  static lv_style_t btn_style;
  lv_style_init(&btn_style);
  lv_style_set_text_font(&btn_style, lv_font_default());
  lv_style_set_text_color(&btn_style, lv_color_black());
  lv_style_set_text_align(&btn_style, LV_ALIGN_CENTER);
  lv_style_set_align(&btn_style, LV_ALIGN_CENTER);

  lv_obj_t *button = lv_btn_create(menuBox);
  lv_obj_set_width(button, lv_pct(100));
  lv_obj_t *buttonLabel = lv_label_create(button);
  lv_obj_set_style_bg_color(button, lv_color_make(128, 128, 128), LV_PART_MAIN);
  lv_label_set_text(buttonLabel, getName().c_str());
  lv_obj_add_style(buttonLabel, &btn_style, 0);
  lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, 0);

  LV_LOG_TRACE("<<< SamsungTV::displaySettings()");
}

// Network related includes:
//#include <WebSocketsClient.h>
#include <iostream>
#include <ctype.h>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <fstream>

// Target host details:
#define PORT 8001
#define HOST "192.168.178.43"

void SamsungTV::MessageSend(std::string message)
{
  LV_LOG_USER(">>> SamsungTV::MessageSend(%s)", message.c_str());
int sock;
struct sockaddr_in client;

    struct hostent * host = gethostbyname("samsung.fritz.box");
    //struct hostent * host = gethostbyaddr("192.168.178.43");
    
    if ( (host == NULL) || (host->h_addr == NULL) ) {
        LV_LOG_USER("Error retrieving DNS information.");
        exit(1);
    }

    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons( PORT );
    memcpy(&client.sin_addr, host->h_addr, host->h_length);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        LV_LOG_USER("Error creating socket.");
        exit(1);
    }

    if ( connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0 ) {
        close(sock);
        LV_LOG_USER("Could not connect");
        exit(1);
    }

    std::stringstream ss;
    ss << "GET " << message << " HTTP/1.1\r\n"
       << "Host: 192.168.178.43\r\n"
       << "Accept: application/json\r\n"
       << "\r\n\r\n";
    std::string request = ss.str();
    LV_LOG_USER("send request: %s", request.c_str());
    if (send(sock, request.c_str(), request.length(), 0) != (int)request.length()) {
        LV_LOG_USER("Error sending request.");
        return;
    }

    char cur;
    char buffer[2048]; int i = 0;
    while ( read(sock, &cur, 1) > 0 ) {
        //printf("%c", cur);
        buffer[i++] = cur;
        if( i == 2048 ){
          LV_LOG_ERROR("buffer full");
          break;
        }
    }
    //LV_LOG_USER("buffer:\n>>>\n%s\n<<<", buffer);
    LV_LOG_USER("%s", buffer);
  LV_LOG_USER("<<< SamsungTV::MessageSend()");
}