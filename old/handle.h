#ifndef HANDLE_H
#define HANDLE_H

void handle_ack();
void handle_err();
void handle_version();
void handle_ramldr();
void handle_echo(char *arg);
void handle_dump(char *arg);

#endif
