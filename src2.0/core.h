#pragma once

void Output(char *pBuf);

const char *GenerateIP();
void exec(char* cmd);









/* Defines */
#define MAX_CLIENTS 64
#define MAX_DUMMIES_PER_CLIENT 64
#define BUFLEN 2048

#define DUMMIES_PORT 1337

#define TIMEOUT 60
#define TIMEOUT_SEC TIMEOUT + 1 //we kick on 1, not 0





