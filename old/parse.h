#ifndef PARSE_H
#define PARSE_H

#define STX  0x02
#define ETX  0x03
#define RS   0x1E

void parse_cmd(unsigned char *in, short len);

#endif
