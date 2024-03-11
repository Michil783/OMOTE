//#include "BasicUI.hpp"
#include "OmoteUI.hpp"
#include "HardwareFactory.hpp"
#include "Settings.hpp"
#include "MagentaTV.hpp"

namespace OMOTE {
std::shared_ptr<UI::UIBase> ui = nullptr;
std::shared_ptr<Settings> settings = nullptr;
std::shared_ptr<MagentaTV> magentaTV = nullptr;

void setup() {
  HardwareFactory::getAbstract().init();
  ui = std::make_unique<UI::Basic::OmoteUI>();
  magentaTV = std::make_unique<MagentaTV>(HardwareFactory::getAbstract().display());
  settings = std::make_unique<Settings>(HardwareFactory::getAbstract().display());
  #ifdef OMOTE_SIM
  UI::Basic::OmoteUI::getInstance()->setActiveTab(2);
  #endif
  lv_timer_handler(); // Run the LVGL UI once before the loop takes over
}

void loop() {
  HardwareFactory::getAbstract().loopHandler();
  ui->loopHandler();
}

} // namespace OMOTE
