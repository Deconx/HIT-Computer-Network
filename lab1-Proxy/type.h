//
// Created by 10670 on 2022/10/3.
//

#ifndef LAB1_PROXY_TYPE_H
#define LAB1_PROXY_TYPE_H
#include <cstdio>
#include <Windows.h>
#include <process.h>
#include <cstring>
#include <vector>
#include <string>
#define MAXSIZE 102400       //�������ݱ��ĵ���󳤶�
#define MAX_FILE_NAME 100   //��󻺴��ļ�����
#define DATE_SIZE 50        //���ڳ���
#define HTTP_PORT 80        //http �������˿�


//Http ��Ҫͷ������
struct HttpHeader{
    char method[4]{}; // POST ���� GET��ע����ЩΪ CONNECT����ʵ���ݲ�����
    char url[1024]{}; // ����� url
    char host[1024]{}; // Ŀ������
    char cookie[1024 * 10]{};   //cookie
    int valid{};               //�Ƿ���Ч
    HttpHeader(){
        memset(this,0,sizeof(HttpHeader));
    }
};
struct ProxyParam{
    SOCKET clientSocket;
    SOCKET serverSocket;
};
#endif //LAB1_PROXY_TYPE_H
