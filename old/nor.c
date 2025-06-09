#define nor_cmd(off, cmd) {*(unsigned short*)(nor+off) = (short)cmd;}

int get_id(int addr) {
  unsigned char *nor = (unsigned char *)addr;
  unsigned int id;

  // send magic commands to nor
  nor_cmd(0xAAA, 0xAA);
  nor_cmd(0x554, 0x55);
  nor_cmd(0xAAA, 0x90);

  // read 4-byte answer
  memcpy(&id, nor, 4);

  return id;
}

// find id and dump it to usb
void dump_id() {
  int id = get_id(0x10000000);

  usb_write_ep2(&id, 4);
}
