#pragma once
#include "AppInterface.hpp"
#include "DisplayAbstract.h"
#include "SDL2/SDL.h"
#include <stdint.h>

#define TAB_ARRAY_SIZE 10

class SDLDisplay : public DisplayAbstract {

public:
  static std::shared_ptr<SDLDisplay> getInstance();

  virtual void setBrightness(uint8_t brightness) override;
  virtual uint8_t getBrightness() override;
  //virtual lv_color_t getPrimaryColor() override;
  virtual void turnOff() override;

  void setTitle(std::string aNewTitle);

  //lv_obj_t* addTab(AppInterface* app);
  //virtual lv_obj_t* getTabView() override;

protected:
  virtual void flushDisplay(lv_disp_drv_t *disp, const lv_area_t *area,
                            lv_color_t *color_p) override;
  virtual void screenInput(lv_indev_drv_t *indev_driver,
                           lv_indev_data_t *data) override;

private:
  SDLDisplay();
  uint8_t mBrightness;
  SDL_Window *mSimWindow;

  //lv_color_t mPrimaryColor;
  
  /**
   * @brief Function to create the tab view buttons
   * 
   */
  //void createTabviewButtons();

  //lv_obj_t* tabview;
  
  /**
   * @brief Array of tab names
   * 
  */
  //const char *tabNames[TAB_ARRAY_SIZE];
  AppInterface* apps[TAB_ARRAY_SIZE];

};