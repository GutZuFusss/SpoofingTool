#pragma once

void Output(const char *pBuf);
void Output(char *fmt, ...);

const char *GenerateIP();
int GetRand(int Start, int End);
void exec(char* cmd);









/* Defines */
//#define MAX_CLIENTS 64
#define MAX_DUMMIES_PER_CLIENT 67
#define BUFLEN 2048

#define DUMMIES_PORT 1337

#define TIMEOUT 60
#define TIMEOUT_SEC TIMEOUT + 1 //we kick on 1, not 0





