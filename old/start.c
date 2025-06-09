#include "parse.h"

void start (void) __attribute__ ((section (".startup")));

void start() {

  make_table();
  usb_init();

  while(1) {
    alive();
    usb_wait_ep1();
  }

}

