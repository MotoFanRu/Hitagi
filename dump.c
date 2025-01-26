#define MAX_CHUNK 0x10

void dump(int addr, int size) {

  unsigned short* data = (unsigned short*)addr;

  while(size >0) {
    int chunk = size;

    if (size > MAX_CHUNK)
      chunk = MAX_CHUNK;

    usb_write_ep2(data, chunk);

    size -= MAX_CHUNK;
    data += MAX_CHUNK/2;
  }
}
