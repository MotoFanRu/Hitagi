unsigned short *watchdog = (unsigned short*)0x24849002;

void alive() {
  *watchdog = 0x5555;
  *watchdog = 0xAAAA;
}

