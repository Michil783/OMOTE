/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "lvgl.h"
#include "app_hal.h"

#include <omote.hpp>
#include <Display.hpp>
//#include <WifiHandler.hpp>
//#include <Battery.hpp>
//#include <IRHandler.hpp>
#include <Settings.hpp>
//#include <MagentaTV.hpp>

//#include "demos/lv_demos.h"

Display display(LCD_BL, LCD_EN, screenWidth, screenHeight);
//WifiHandler wifihandler;
//Battery battery(ADC_BAT, CRG_STAT);
//IRHandler irhandler;

/* UI instances */
Settings settings(&display);

// App instances
MagentaTV magentaTV(&display);

int main(void)
{
	lv_init();

	hal_setup();

  //lv_demo_widgets();

	hal_loop();
}
