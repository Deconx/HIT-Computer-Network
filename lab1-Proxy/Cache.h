//
// Created by 10670 on 2022/10/3.
//
#include "type.h"
#ifndef LAB1_PROXY_CACHE_H
#define LAB1_PROXY_CACHE_H
BOOL query_Cache(char* buf, HttpHeader* httpHeader, char* filename); // ��ѯ�Ƿ��ڻ�����
void write_Cache(char *url, char *buf);                              // д�뻺��
void read_Cache(char *filename, char *buf);                          // ������
void generate_File(char *url, char *filename);                       // ���ɻ����ʶ
void Parse_Date(char *buffer, char *field, char *tempDate);          // ��������
void generate_HTTP(char *buffer, char *value);
#endif //LAB1_PROXY_CACHE_H
