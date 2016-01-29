#include "stdafx.h"
#include <stdarg.h>
#include <string>
#include <ctime>



#include "core.h"


/* Print messages */
void PrintTimestamp()
{
	time_t rawtime;
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	printf("[%2.i:%2.i:%2.i]", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

}
void Output(const char *pBuf)
{
	PrintTimestamp();
	printf(" %s\n", pBuf);
}
void Output(char *fmt, ...)
{
	/*FILE *pFile = fopen("Sniffer.txt","a+");
	fprintf(pFile,"%s", pBuf);
	fclose(pFile);*/
	va_list vl;
	va_start(vl, fmt);
	PrintTimestamp(); printf(" ");
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");
}

/* Generate random IP */
const char *GenerateIP()
{
	int Oktett[4];
	static char aIP[32];

	// do the initial random process
	Oktett[0] = rand() % 255 + 1;
	Oktett[1] = rand() % 255 + 1;
	Oktett[2] = rand() % 255 + 1;
	Oktett[3] = rand() % 255 + 1;

	// check for invalid IPs
	while (Oktett[0] == 192 || Oktett[0] == 10 || Oktett[0] == 172 || Oktett[0] == 127)
	{
		Oktett[0] = rand() % 255 + 1;
	}

	sprintf_s(aIP, sizeof(aIP), "%i.%i.%i.%i", Oktett[0], Oktett[1], Oktett[2], Oktett[3]);

	return aIP;
}

/* Easier random numbers */
int GetRand(int Start, int End)
{
    int randnum = End + rand() / (RAND_MAX / (Start - End + 1) + 1);
    
    return randnum;
}

void exec(char* cmd) 
{
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return;
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
}
