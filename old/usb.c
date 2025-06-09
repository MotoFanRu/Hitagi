unsigned short *usbint = (unsigned short*)0x24852004;
unsigned short *e2cr = (unsigned short*) 0x2485200C;
unsigned short *cpucr = (unsigned short*) 0x24852014;

void usb_wait_ep1() {
  unsigned short *e1cr = (unsigned short*) 0x2485200A;
  unsigned char *in = (unsigned char*)0x24852090;

  while(!(*usbint & (1<<5)));;

  short len = *e1cr & 0x3F;

  parse_cmd(in, len);

  *e1cr |= 1<<13;
}

void usb_write_ep2(unsigned short* data, short len) {


  unsigned short *out = (unsigned short*)0x248520b0;


  while(!(*usbint & 1<<6));;// wait for transfer end on ep2
//  while(*e2cr & (1<<10));;

  short _len = len;
  
  while(len > 0) {
    *out++ = *data++;
    len -= 2;
  }

  *e2cr &= ~0x3f;
  *e2cr |= _len;

  *e2cr |= 1<<13;
  *e2cr |= 1<<10;

}

void usb_init() {
  *cpucr = 1;
}
