/*
 * Command parser debugger.
 *
 * gcc simula.c -nostdinc -fno-builtin -Wno-implicit-function-declaration -o simula
 *
 */

#include "../types.h"
#include "../parse.h"

void *memcpy(void *dest, const void *src, size_t n)
{
	const u8 *s = (const u8*)src;
	u8 *d = (u8 *)dest;

	while(n-- > 0)
		*d++ = *s++;

	return dest;
}

int memcmp(void *a, void *b, int size)
{
	unsigned char *aa=a, *bb=b;
	int i;

	for(i=0; i < size; i++)
		if(aa[i] != bb[i]) return 1;

	return 0;
}


void memset(void *start, unsigned char val, unsigned long size)
{
	int i;
	for(i=0; i < size; i++ ) {
		((unsigned char*)start)[i] = val;
	}
}

int strlen(const char *s)
{
	int i = 0;

	for(;*s != '\0'; s++)
		i++;

	return i;

}

int strcmp(const char * cs,const char * ct)
{
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

void parse_ramldr2_cmd(char *in) {
  printf("OK in: %s\n", in);
}


struct COMMAND_NAME
{
  char cmd[10];
  void (*func)(char*);
};

static struct COMMAND_NAME moto_table[20];

void make_table() {
  moto_table[0] = (struct COMMAND_NAME){ "RQVN", parse_ramldr2_cmd };
  moto_table[1] = (struct COMMAND_NAME){ "HLP", parse_ramldr2_cmd };
  moto_table[2] = (struct COMMAND_NAME){ "ECHO", parse_ramldr2_cmd };
  moto_table[3] = (struct COMMAND_NAME){ "RAMLDR", parse_ramldr2_cmd };
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

  printf("Err\n");

}

int main(void) {
  const char *cmd = "\x02" "ECHO" "\x1E" "\x03";
  make_table();
  parse_moto_cmd((char *) cmd, strlen(cmd));
  return 0;
}
