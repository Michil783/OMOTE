#include "OmoteSetup.hpp"
#include "sdl/sdl.h"

int main() {
  sdl_init();

  OMOTE::setup();
  while (true) {
    OMOTE::loop();
  }
}