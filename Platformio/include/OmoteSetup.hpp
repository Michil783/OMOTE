//#include "BasicUI.hpp"
#include "OmoteUI.hpp"
#include "HardwareFactory.hpp"
#include "Settings.hpp"

namespace OMOTE {
std::shared_ptr<UI::UIBase> ui = nullptr;
std::shared_ptr<Settings> settings = nullptr;

void setup() {
  HardwareFactory::getAbstract().init();
  ui = std::make_unique<UI::Basic::OmoteUI>();
  settings = std::make_unique<Settings>(HardwareFactory::getAbstract().display());
  lv_timer_handler(); // Run the LVGL UI once before the loop takes over
}

void loop() {
  HardwareFactory::getAbstract().loopHandler();
  ui->loopHandler();
}

} // namespace OMOTE
