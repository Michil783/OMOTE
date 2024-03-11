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

SamsungTV *SamsungTV::mInstance;

void SamsungTV::virtualKeypad_event_cb(lv_event_t *e)
{
  LV_LOG_TRACE(">>> SamsungTV::virtualKeypad_event_cb");
  lv_obj_t *target = lv_event_get_target(e);
  lv_obj_t *cont = lv_event_get_current_target(e);
  // if (target == cont)
  // {
  //     LV_LOG_TRACE("virtualKeypad_event_cb - container was clicked");
  //     return; // stop if container was clicked
  // }
  LV_LOG_TRACE("button pressed %c", *((char *)target->user_data));
  // Send IR command based on the button user data
  mInstance->handleCustomKeypad(-1, *((char *)target->user_data));
  LV_LOG_TRACE("<<< SamsungTV::viurtualKeypad_event_cb");
}

SamsungTV::SamsungTV(std::shared_ptr<DisplayAbstract> display)
{
  LV_LOG_TRACE(">>> SamsungTV::SamsungTV()");
  mDisplay = display;
  UI::Basic::OmoteUI::getInstance()->addDevice(this);
  mInstance = this;
  LV_LOG_TRACE("<<< SamsungTV::SamsungTV()");
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
  if (keyChar == 'p')
    powerOn();
#ifdef OMOTE_ESP32
  else
  {
    if (code != -1)
      IrSender.sendSAMSUNG(code);
  }
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
  // lv_obj_set_user_data(button, (void *)&(samsungKeys[4].key));
  // lv_obj_add_event_cb(button, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);

  mControlPage = createControlPage(menu);
  lv_menu_set_load_page_event(menu, button, mControlPage);

  LV_LOG_TRACE("<<< SamsungTV::displaySettings()");
}

lv_obj_t *SamsungTV::mControlPage;

lv_obj_t *SamsungTV::createControlPage(lv_obj_t *menu)
{
  LV_LOG_TRACE(">>> SamsungTV::createControlPage()");
  lv_obj_t *menuLabel;

  lv_obj_t *ret_val = lv_menu_page_create(menu, NULL);
  lv_obj_t *cont = lv_menu_cont_create(ret_val);
  lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);

  static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST}; // equal x distribution
  static lv_coord_t row_dsc[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, LV_GRID_TEMPLATE_LAST};      // manual y distribution to compress the grid a bit

  // Create a container with grid for tab2
  lv_obj_set_style_pad_all(ret_val, 0, LV_PART_MAIN);
  lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, LV_STATE_DEFAULT);
  lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, LV_STATE_DEFAULT);
  lv_obj_set_size(cont, lv_pct(98), 10 * 60);
  lv_obj_center(cont);
  lv_obj_set_layout(cont, LV_LAYOUT_GRID);

  lv_obj_t *buttonLabel;
  lv_obj_t *obj;
  static lv_style_t style;

  lv_style_init(&style);
  lv_style_set_bg_color(&style, mPrimaryColor);

  static lv_style_t btn_style;
  lv_style_init(&btn_style);
  lv_style_set_text_font(&btn_style, lv_font_default());
  lv_style_set_text_color(&btn_style, lv_color_black());
  lv_style_set_text_align(&btn_style, LV_ALIGN_CENTER);
  lv_style_set_align(&btn_style, LV_ALIGN_CENTER);
  // lv_style_set_bg_color(&btn_style, lv_color_make(120, 120, 120));

  for (int i = 0; i < SAMSUNGKEYS; i++)
  {
    LV_LOG_TRACE("key: %d col: %d, row: %d", i, keyInfo[i].col, keyInfo[i].row);
    obj = lv_btn_create(cont);
    lv_obj_set_style_bg_color(obj, lv_color_make(120, 120, 120), LV_PART_MAIN);
    // Create Labels for each button
    buttonLabel = lv_label_create(obj);
    lv_label_set_text(buttonLabel, keyInfo[i].keySymbol.c_str());
    lv_obj_add_style(buttonLabel, &btn_style, 0);

    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, keyInfo[i].col, keyInfo[i].colSpan, LV_GRID_ALIGN_STRETCH, keyInfo[i].row, keyInfo[i].rowSpan);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE); // Clicking a button causes a event in its container
    lv_obj_set_user_data(obj, (void *)&(keyInfo[i].key));
  }
  // Create a shared event for all button inside container
  lv_obj_add_event_cb(cont, virtualKeypad_event_cb, LV_EVENT_CLICKED, NULL);

  LV_LOG_TRACE("<<< SamsungTV::createControlPage()");
  return ret_val;
}

// Network related includes:
// #include <WebSocketsClient.h>
#ifdef OMOTE_SIM
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
#endif

// Target host details:
#define PORT 8001
// #define HOST "192.168.178.43"
// #define MAC "68:72:c3:62:53:ab"
#define BROADCAST_IP = "255.255.255.255"

void SamsungTV::createMagicPacket(std::string macaddress)
{
  /*
  Create a magic packet.

  A magic packet is a packet that can be used with the for wake on lan
  protocol to wake up a computer. The packet is constructed from the
  mac address given as a parameter.

  Args:
      macaddress: the mac address that should be parsed into a magic packet.

  */
  LV_LOG_TRACE(">>> SamsungTV::createMagicPacket(%s)", macaddress.c_str());

  while (macaddress.find(":") != std::string::npos)
  {
    macaddress = macaddress.replace(macaddress.find(":"), 1, "");
  }
  bzero(magicPacket, 102);
  memcpy(magicPacket, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
  for (int i = 0; i < 16; i++)
  {
    for (int pos = 0; pos < macaddress.length(); pos = pos + 2)
    {
      unsigned char byte = (unsigned char)strtol(macaddress.substr(pos, 2).c_str(), NULL, 16);
      memcpy(&magicPacket[6 + (i * 6) + (pos / 2)], &byte, 1);
    }
  }
  LV_LOG_TRACE("magicPacket");
  for (int i = 0; i < 102; i++)
  {
    printf("0x%02x ", magicPacket[i]);
  }
  printf("\n");
  LV_LOG_TRACE("<<< SamsungTV::createMagicPacket()");
}

bool SamsungTV::powerOn()
{
  LV_LOG_TRACE(">>> SamsungTV::powerOn()");

  bool result = false;
  #ifdef OMOTE_SIM
  int sock;
  struct sockaddr_in client;
  struct sockaddr_in encoder_addr;

  bzero(&client, sizeof(client));
  client.sin_family = AF_INET;
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    LV_LOG_TRACE("Error creating socket.");
    result = false;
  }
  else
  {
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = INADDR_ANY;
    bzero(&client.sin_zero, sizeof client.sin_zero);
    bind(sock, (struct sockaddr *)&client, sizeof client);
    // mark the socket for broadcasting
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1)
    {
      LV_LOG_ERROR("setsockopt (SO_BROADCAST)");
      result = false;
    }
    else
    {
      encoder_addr.sin_family = AF_INET;
      encoder_addr.sin_port = htons(PORT);
      encoder_addr.sin_addr.s_addr = INADDR_BROADCAST; // inet_addr(IP_address);
      memset(encoder_addr.sin_zero, '\0', sizeof encoder_addr.sin_zero);

      /* Send the magic packet */
      createMagicPacket(mac);
      int totalBytesSent = 0;
      int bytesSent;
      int packetSize = sizeof(magicPacket);
      LV_LOG_TRACE("Total packetSize = %d", packetSize);
      int totalBytesLeft = packetSize;
      while (totalBytesSent < packetSize)
      {
        if ((bytesSent = sendto(sock, magicPacket + totalBytesSent, totalBytesLeft, 0,
                                (struct sockaddr *)&encoder_addr, sizeof encoder_addr)) == -1)
        {
          perror("sendto");
          result = false;
        }
        else
        {
          totalBytesSent += bytesSent;
          totalBytesLeft -= bytesSent;
          printf("sent %d bytes to %s\n", bytesSent, inet_ntoa(encoder_addr.sin_addr));
        }
        close(sock);
        result = true;
      }
    }
  }
  #endif
  LV_LOG_TRACE("<<< SamsungTV::powerOn()");
  return result;
}

bool SamsungTV::powerOff()
{
  bool result = false;
  LV_LOG_TRACE(">>> SamsungTV::powerOff()");
  LV_LOG_TRACE("<<< SamsungTV::powerOff()");
  return result;
}

bool SamsungTV::isPowerOn()
{
  LV_LOG_TRACE(">>> SamsungTV::isPowerOn()");
  bool result;
  int sock;
  #ifdef OMOTE_SIM
  struct sockaddr_in client;
  struct hostent *host = gethostbyname("samsung.fritz.box");
  // struct hostent * host = gethostbyaddr("192.168.178.43");

  if ((host == NULL) || (host->h_addr == NULL))
  {
    LV_LOG_TRACE("Error retrieving DNS information.");
    exit(1);
  }

  bzero(&client, sizeof(client));
  client.sin_family = AF_INET;
  client.sin_port = htons(PORT);
  memcpy(&client.sin_addr, host->h_addr, host->h_length);

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0)
  {
    LV_LOG_TRACE("Error creating socket.");
    result = false;
  }
  else
  {
    if (connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0)
    {
      close(sock);
      LV_LOG_TRACE("Could not connect");
      result = false;
    }
    LV_LOG_TRACE("SamsungTV reachable = power on");
    close(sock);
    result = true;
  }
  #endif
  LV_LOG_TRACE("<<< SamsungTV::isPowerOn() = %d", result);
  return result;
}

void SamsungTV::sendMagicPacket(std::string mac)
{
  LV_LOG_TRACE(">>> SamsungTV::sendMagicPacket(%s)", mac.c_str());
  LV_LOG_TRACE("<<< SamsungTV::sendMagicPacket()");
}

void SamsungTV::MessageSend(std::string message)
{
  LV_LOG_TRACE(">>> SamsungTV::MessageSend(%s)", message.c_str());
  int sock;

  #ifdef OMOTE_SIM
  struct sockaddr_in client;

  struct hostent *host = gethostbyname("samsung.fritz.box");
  // struct hostent * host = gethostbyaddr("192.168.178.43");

  if ((host == NULL) || (host->h_addr == NULL))
  {
    LV_LOG_TRACE("Error retrieving DNS information.");
    exit(1);
  }

  bzero(&client, sizeof(client));
  client.sin_family = AF_INET;
  client.sin_port = htons(PORT);
  memcpy(&client.sin_addr, host->h_addr, host->h_length);

  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock < 0)
  {
    LV_LOG_TRACE("Error creating socket.");
    exit(1);
  }

  if (connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0)
  {
    close(sock);
    LV_LOG_TRACE("Could not connect");
    exit(1);
  }

  std::stringstream ss;
  ss << "GET " << message << " HTTP/1.1\r\n"
     << "Host: 192.168.178.43\r\n"
     << "Accept: application/json\r\n"
     << "\r\n\r\n";
  std::string request = ss.str();
  LV_LOG_TRACE("send request: %s", request.c_str());
  if (send(sock, request.c_str(), request.length(), 0) != (int)request.length())
  {
    LV_LOG_TRACE("Error sending request.");
    return;
  }

  char cur;
  char buffer[2048];
  int i = 0;
  while (read(sock, &cur, 1) > 0)
  {
    // printf("%c", cur);
    buffer[i++] = cur;
    if (i == 2048)
    {
      LV_LOG_ERROR("buffer full");
      break;
    }
  }
  // LV_LOG_TRACE("buffer:\n>>>\n%s\n<<<", buffer);
  LV_LOG_TRACE("%s", buffer);
  #endif
  LV_LOG_TRACE("<<< SamsungTV::MessageSend()");
}