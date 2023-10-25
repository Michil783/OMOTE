#include "StatusBar.hpp"
#include "Colors.hpp"
#include "ScreenManager.hpp"

using namespace UI;
using namespace UI::Screen;

StatusBar::StatusBar(Page::Base::Ptr aPage)
    : Screen::Base(UI::ID::Screens::PopUp) {

  mContentPage = AddElement<Page::Base>(std::move(aPage));

  mContentPage->SetHeight(STATUSBAR_HEIGHT);
  mContentPage->SetY(STATUSBAR_Y);
  
  mContentPage->SetBgColor(Color::GREY);
}

/*
bool StatusScreen::OnKeyEvent(KeyPressAbstract::KeyEvent aKeyEvent) {
  return mContentPage->OnKeyEvent(aKeyEvent);
}
*/