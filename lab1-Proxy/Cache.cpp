//
// Created by 10670 on 2022/10/3.
//
#include "Cache.h"
#include "type.h"
BOOL query_Cache(char* buf, HttpHeader* httpHeader, char* filename) {
    char *field = "Date";
    char fileBuffer[MAXSIZE];
    char *DateBuffer;
    DateBuffer = (char*)malloc(MAXSIZE);
    ZeroMemory(DateBuffer, strlen(buf) + 1);
    memcpy(DateBuffer, buf, strlen(buf) + 1);
    generate_File(httpHeader->url, filename);
    char date[DATE_SIZE];  //�����ֶ�Date
    ZeroMemory(date, DATE_SIZE);
    ZeroMemory(fileBuffer, MAXSIZE);
    FILE *in;
    if ((in = fopen(filename, "rb")) != NULL) {
        printf("��ѯ������\n");
        fread(fileBuffer, sizeof(char), MAXSIZE, in);
        fclose(in);
        printf("******\n");
        Parse_Date(fileBuffer, field, date);
        printf("date: %s\n", date);
        generate_HTTP(buf, date);
        return TRUE;
    }
}
void write_Cache(char *url, char *buf) {
    char num[10], tempBuffer[MAXSIZE + 1];
    const char * delim = "\r\n";
    ZeroMemory(num, 10);
    ZeroMemory(tempBuffer, MAXSIZE + 1);
    memcpy(tempBuffer, buf, strlen(buf));
    char *p = strtok(tempBuffer, delim);//��ȡ��һ��
    memcpy(num, &p[9], 3);
    if (strcmp(num, "200") == 0) {  //�ж�״̬���Ƿ�Ϊ200
        //printf("url : %s\n", url);
        char filename[100] = { 0 };  // �����ļ���
        generate_File(url, filename);
        FILE *out;
        out = fopen(filename, "w");
        fwrite(buf, sizeof(char), strlen(buf), out);
        fclose(out);
        printf("������: %s\n", filename);
    }
}
void read_Cache(char *filename, char *buf) {
    printf("��û���: %s\n", filename);
    ZeroMemory(buf, strlen(buf));
    FILE *in;
    if ((in = fopen(filename, "r")) != NULL) {
        fread(buf, sizeof(char), MAXSIZE, in);
        fclose(in);
    }
}
void generate_File(char *url, char *filename) {
    int i = 0;
    while (i < MAX_FILE_NAME && *url != '\0') {
        if (*url != '/' && *url != ':' && *url != '.' && *url != '?') {
            *filename++ = *url;
        }
        url++;
        i++;
    }
    strcat(filename, ".cache");
}
void Parse_Date(char *buffer, char *field, char *tempDate) {
    char *p, *ptr, temp[5];

    const char *delim = "\r\n";
    ZeroMemory(temp, 5);
    p = strtok(buffer, delim);

    int len = strlen(field) + 2;
    while (p) {
        if (strstr(p, field) != NULL) {
            memcpy(tempDate, &p[len], strlen(p) - len);
            return;
        }
        p = strtok(NULL, delim);
    }
}
void generate_HTTP(char *buffer, char *value) {
    const char *field = "Host";
    const char *newfield = "If-Modified-Since: ";
    char temp[MAXSIZE];
    ZeroMemory(temp, MAXSIZE);
    char *pos = strstr(buffer, field);
    int i = 0;
    for (i = 0; i < strlen(pos); i++) {
        temp[i] = pos[i];
    }
    *pos = '\0';
    // ���� if-Modified-Since
    while (*newfield != '\0') {
        *pos++ = *newfield++;
    }
    while (*value != '\0') {
        *pos++ = *value++;
    }
    *pos++ = '\r';
    *pos++ = '\n';
    for (i = 0; i < strlen(temp); i++) {
        *pos++ = temp[i];
    }
}
