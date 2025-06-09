#include "parse.h"
#include "handle.h"
#include "types.h"

void parse_ramldr2_cmd(unsigned char *in, short len) {
  return;
}

struct COMMAND_NAME
{
  char cmd[10];
  void (*func)(char*);
};

static struct COMMAND_NAME moto_table[20];

void make_table() {
  moto_table[0] = (struct COMMAND_NAME){ "RQVN", handle_version };
  moto_table[1] = (struct COMMAND_NAME){ "HLP", handle_ack };
  moto_table[2] = (struct COMMAND_NAME){ "ECHO", handle_echo };
  moto_table[3] = (struct COMMAND_NAME){ "RAMLDR", handle_ramldr };
  moto_table[4] = (struct COMMAND_NAME){ "DUMP", handle_dump };
}

void parse_moto_cmd(unsigned char *in, short len) {
  char cmd[10];
  char arg[20];
  char *to;

  short _len = len;
  short fragment_len = 0;
  short skip = 1;
  memset(cmd, 0, 10);
  memset(arg, 0, 20);

  while (_len--) {
    switch(*(in+fragment_len)) {
      case ETX:
      case RS:
        if(cmd[0])
          to = (char*)arg;
        else
          to = (char*)cmd;

        memcpy(to, (in + skip), fragment_len-skip);
          
        skip += fragment_len;
      default:
        fragment_len++;
    }
  }

  int i;
  for(i=0; i<20; i++) {

    if(!strcmp(moto_table[i].cmd, cmd)) {
      moto_table[i].func(arg);
      return;
    }
  }

  handle_err();

}

void parse_ramldr2_read(unsigned char *in, short len) {
  unsigned int addr = 0, size=0;

  memcpy(&addr, in, 4);
  memcpy(&size, in+4, 4);

  dump(addr, size);
}

void parse_cmd(unsigned char *in, short len) {

  switch(*in) {
    case STX :

      parse_moto_cmd(in, len);
      break;

    case 'R': // RAMLDR2 read block
      in++;
      parse_ramldr2_read(in, len);
      break;

    case 'C': // RAMLDR2 report flash id
      dump_id();
      break;
    case 'F': // RAMLDR2 write
    case 'Z': // RAMLDR2 shutdown
    case 'A': // RAMLDR2 init
    case 'B': // RAMLDR2 set blank
    case 'S': // RAMLDR2 start phone
      parse_ramldr2_cmd(in, len);
    break;
  }
}

