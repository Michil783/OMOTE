#pragma once
#include "PageBase.hpp"
#include "ScreenBase.hpp"

namespace UI::Screen {

/// @brief A Screen that allows easy display of all status icons
///        like WiFi, USB and battery status
class StatusBar : public Base {
public:
  StatusBar(UI::Page::Base::Ptr aPage);

  //bool OnKeyEvent(KeyPressAbstract::KeyEvent aKeyEvent) override;

private:
  UI::Page::Base *mContentPage = nullptr;
};

} // namespace UI::Screen