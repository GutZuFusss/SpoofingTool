#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

ssize_t getdelim(char **linep, size_t *n, int delim, FILE *fp){
    int ch;
    size_t i = 0;
    if(!linep || !n || !fp){
        errno = EINVAL;
        return -1;
    }
    if(*linep == NULL){
        if(NULL==(*linep = malloc(*n=128))){
            *n = 0;
            errno = ENOMEM;
            return -1;
        }
    }
    while((ch = fgetc(fp)) != EOF){
        if(i + 1 >= *n){
            char *temp = realloc(*linep, *n + 128);
            if(!temp){
                errno = ENOMEM;
                return -1;
            }
            *n += 128;
            *linep = temp;
        }
        (*linep)[i++] = ch;
        if(ch == delim)
            break;
    }
    (*linep)[i] = '\0';
    return !i && ch == EOF ? -1 : i;
}
ssize_t getline(char **linep, size_t *n, FILE *fp){
    return getdelim(linep, n, '\n', fp);
}

inline int ms_rand(int *seed)
{
    *seed = *seed*0x343fd+0x269EC3;  // a=214013, b=2531011
    return (*seed >> 0x10) & 0x7FFF;
}

int main()
{
    int seed = 0;
    int tmpseed = 0;
    int port = 0;
    int result = 0;
    int i = 0;

    FILE *ips = fopen("ips.txt", "r");
    if(ips == NULL)
    {
        printf("error opening ips.txt");
        return 0;
    }

    FILE *fix = fopen("ipsfix.txt", "w");
    if(fix == NULL)
    {
        printf("error opening ipsfix.txt");
        return 0;
    }

    char * line = NULL;
    size_t len = 0;
    char *cip;
    char *cport;

    while (getline(&line, &len, ips) != -1)
    {
        cip = strtok(line, ":");
        cport = strtok(NULL, ":");

        port = atoi(cport);
        seed = time(NULL);
        for(i = 0;i < 6*60*60; i++)
        {
            tmpseed = seed;
            result = (ms_rand(&tmpseed) % 64511) + 1024;
            ms_rand(&tmpseed);
            if(((ms_rand(&tmpseed) % 64511) + 1024) == port)
            {
                break;
            }
            result = 0;
            seed--;
        }

        if(!result)
            fprintf (fix, "%s:%s", cip, cport);
        else
            fprintf (fix, "%s:%d\n", cip, result);
    }

    fclose(ips);
    fclose(fix);

    if(!remove("ips.txt"))
    {
        printf("File deleted successfully\n");
    }
    else
    {
        printf("Error: unable to delete the file\n");
    }

    if(!rename("ipsfix.txt", "ips.txt"))
    {
        printf("File renamed successfully\n");
    }
    else
    {
        printf("Error: unable to rename the file\n");
    }
    return 0;
}
