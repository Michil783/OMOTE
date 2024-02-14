#include "SDLDisplay.hpp"
#include "sdl/sdl.h"
#include <string>

std::shared_ptr<SDLDisplay> SDLDisplay::getInstance() {
  if (!DisplayAbstract::mInstance) {
    DisplayAbstract::mInstance = std::shared_ptr<SDLDisplay>(new SDLDisplay());
  }
  return std::static_pointer_cast<SDLDisplay>(mInstance);
}

void SDLDisplay::setBrightness(uint8_t brightness) { mBrightness = brightness; }

uint8_t SDLDisplay::getBrightness() { return mBrightness; }

//lv_color_t SDLDisplay::getPrimaryColor() { return mPrimaryColor; }

void SDLDisplay::turnOff() {}

void SDLDisplay::flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area,
                              lv_color_t *color_p) {
  sdl_display_flush(disp, area, color_p);
}

void SDLDisplay::screenInput(lv_indev_drv_t *indev_driver,
                             lv_indev_data_t *data) {
  sdl_mouse_read(indev_driver, data);
}

// lv_obj_t *SDLDisplay::getTabView()
// {
//   return this->tabview;
// }

// lv_obj_t *SDLDisplay::addTab(AppInterface* app)
// {
//   lv_obj_t *tab = nullptr;
//   /* search free slot in tab array */
//   for (int i = 0; i < TAB_ARRAY_SIZE; i++)
//   {
//     if (apps[i] == nullptr)
//     {
//       //  Add tab (name is irrelevant since the labels are hidden and hidden buttons are used (below))
//       tab = lv_tabview_add_tab(this->tabview, app->getName().c_str());
//       apps[i] = app;
//       createTabviewButtons();

//       // Initialize scroll position for the indicator
//       //lv_event_send(lv_tabview_get_content(this->tabview), LV_EVENT_SCROLL, NULL);
//       break;
//     }
//   }

//   return tab;
// }

// void SDLDisplay::createTabviewButtons()
// {
// }

void SDLDisplay::setTitle(std::string aNewTitle) {
  SDL_SetWindowTitle(mSimWindow, aNewTitle.c_str());
}

SDLDisplay::SDLDisplay() : DisplayAbstract() {
  //sdl_init();

  // Get the SDL window via an event
  SDL_Event aWindowIdFinder;
  SDL_PollEvent(&aWindowIdFinder);
  mSimWindow = SDL_GetWindowFromID(aWindowIdFinder.window.windowID);
}