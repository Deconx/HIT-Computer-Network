#include "Cache.h"
#include "type.h"
#include "wall.h"
#pragma comment(lib,"Ws2_32.lib")


BOOL InitSocket();
BOOL ParseHttpHead(char *Cachebuffer, HttpHeader *httpHeader, char *buffer);
BOOL ConnectToServer(SOCKET *serverSocket,char *host);
unsigned int __stdcall ProxyThread(LPVOID lpParameter);


//������ز���
SOCKET ProxyServer;
sockaddr_in ProxyServerAddr;
const int ProxyPort = 10240;


int main()
{
    printf("�����������������\n");
    printf("��ʼ��...\n");
    if(!InitSocket()){
        printf("socket ��ʼ��ʧ��\n");
        return -1;
    }
    printf("����������������У������˿� %d\n",ProxyPort);
    SOCKET acceptSocket = INVALID_SOCKET;
    ProxyParam *lpProxyParam = NULL;
    HANDLE hThread;

    //������������ϼ���
    while(true){
        acceptSocket = accept(ProxyServer,NULL,NULL);
        lpProxyParam = new ProxyParam;
        if(lpProxyParam==NULL) {
            continue;
        }
        lpProxyParam->clientSocket = acceptSocket;
        // �����߳�
        hThread = (HANDLE)_beginthreadex(NULL, 0,
                                         &ProxyThread,(LPVOID)lpProxyParam, 0, 0);
        CloseHandle(hThread);
        Sleep(200);
    }
    closesocket(ProxyServer);
    WSACleanup();
    return 0;
}
//************************************
// Method: InitSocket
// FullName: InitSocket
// Access: public
// Returns: BOOL
// Qualifier: ��ʼ���׽���
//************************************
BOOL InitSocket(){
    //�����׽��ֿ⣨���룩
    WORD wVersionRequested;
    WSADATA wsaData;
    //�׽��ּ���ʱ������ʾ
    int err;
    //�汾 2.2
    wVersionRequested = MAKEWORD(2, 2);
    //���� dll �ļ� Scoket ��
    err = WSAStartup(wVersionRequested, &wsaData);
    if(err != 0){
        //�Ҳ��� winsock.dll
        printf("���� winsock ʧ�ܣ��������Ϊ: %d\n", WSAGetLastError());
        return FALSE;
    }
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) !=2)
    {
        printf("�����ҵ���ȷ�� winsock �汾\n");
        WSACleanup();
        return FALSE;
    }
    // TCP ����
    ProxyServer= socket(AF_INET, SOCK_STREAM, 0);
    if(INVALID_SOCKET == ProxyServer){
        printf("�����׽���ʧ�ܣ��������Ϊ��%d\n",WSAGetLastError());
        return FALSE;
    }
    ProxyServerAddr.sin_family = AF_INET;
    ProxyServerAddr.sin_port = htons(ProxyPort);
    // ����IP, �����û�
    ProxyServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.5");
    //ProxyServerAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    //ProxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    //���׽���
    if(bind(ProxyServer,(SOCKADDR*)&ProxyServerAddr,sizeof(SOCKADDR)) == SOCKET_ERROR){
        printf("���׽���ʧ��\n");
        return FALSE;
    }

    if(listen(ProxyServer, SOMAXCONN) == SOCKET_ERROR){
        printf("�����˿�%d ʧ��",ProxyPort);
        return FALSE;
    }
    return TRUE;
}
//************************************
// Method: ProxyThread
// FullName: ProxyThread
// Access: public
// Returns: unsigned int __stdcall
// Qualifier: �߳�ִ�к���
// Parameter: LPVOID lpParameter

//************************************
unsigned int __stdcall ProxyThread(LPVOID lpParameter){
    char Buffer[MAXSIZE];
    char *CacheBuffer;
    memset(Buffer, 0, MAXSIZE);

    int recvSize;
    int ret = 1;
    bool flag = false;
    // �ӿͻ��˽���
    recvSize = recv(((ProxyParam*)lpParameter)->clientSocket,Buffer,MAXSIZE,0);
    if(recvSize <= 0) {
        goto error;
    }

    HttpHeader *httpHeader;
    httpHeader = new HttpHeader();
    CacheBuffer = new char[recvSize + 1];
    // ����һ�ױ���
    memset(CacheBuffer, 0, recvSize + 1);
    memcpy(CacheBuffer,Buffer,recvSize);

    char filename[MAX_FILE_NAME];
    ZeroMemory(filename, MAX_FILE_NAME);

    // ����HTTPͷ
    ParseHttpHead(CacheBuffer, httpHeader,Buffer);
    // �ж�host�Ƿ���Ч
    if(httpHeader->valid == 1) {
        printf("����վ�ѱ�����: %s \n", httpHeader->host);
        goto error;
    }

    // ��ѯ�Ƿ��ڻ�����
    flag = query_Cache(Buffer, httpHeader, filename);
    // phish(httpHeader);

    delete CacheBuffer;
    // ���ӷ�����
    if(!ConnectToServer(&((ProxyParam*)lpParameter)->serverSocket,httpHeader->host)) {
        goto error;
    }
    printf("������������ %s �ɹ�\n",httpHeader->host);
    //���ͻ��˷��͵� HTTP ���ݱ���ֱ��ת����Ŀ�������
    ret = send(((ProxyParam *)lpParameter)->serverSocket,Buffer,strlen(Buffer)+ 1,0);
    if(ret < 0) {
        goto error;
    }
    //�ȴ�Ŀ���������������
    recvSize = recv(((ProxyParam*)lpParameter)->serverSocket,Buffer,MAXSIZE,0);
    if(recvSize <= 0){
        goto error;
    }
    //�ڻ�����
    if(flag == TRUE) {
        char *p, num[10], tempBuffer[MAXSIZE + 1];
        const char * delim = "\r\n";
        ZeroMemory(num, 10);
        ZeroMemory(tempBuffer, MAXSIZE + 1);
        memcpy(tempBuffer, Buffer, strlen(Buffer));
        p = strtok(tempBuffer, delim);
        memcpy(num, &p[9], 3);
        // ���������304����ֱ�Ӷ�����д��
        if (strcmp(num, "304") == 0)
            read_Cache(filename, Buffer);
        else
            // ��������д����
            write_Cache(httpHeader->url, Buffer);
    }
    else {
        write_Cache(httpHeader->url, Buffer);
    }

    //��Ŀ����������ص�����ֱ��ת�����ͻ���
    send(((ProxyParam*)lpParameter)->clientSocket,Buffer,sizeof(Buffer),0);
    //������
    error:
    printf("�ر��׽���\n");
    Sleep(500);
    closesocket(((ProxyParam*)lpParameter)->clientSocket);
    closesocket(((ProxyParam*)lpParameter)->serverSocket);
    free(lpParameter);
    _endthreadex(0);
}
//************************************
// Method: ParseHttpHead
// FullName: ParseHttpHead
// Access: public
// Returns: void
// Qualifier: ���� TCP �����е� HTTP ͷ��
// Parameter: char * buffer
// Parameter: HttpHeader * httpHeader
//************************************
BOOL ParseHttpHead(char *Cachebuffer, HttpHeader * httpHeader, char *buffer){
    char *p;
    char *ptr;
    const char * delim = "\r\n";
    p = strtok_r(Cachebuffer,delim,&ptr);//��ȡ��һ��
    printf("%s\n",p);
    if(p[0] == 'G'){//GET ��ʽ
        memcpy(httpHeader->method,"GET",3);
        memcpy(httpHeader->url,&p[4],strlen(p) -13);
    }else if(p[0] == 'P'){//POST ��ʽ
        memcpy(httpHeader->method,"POST",4);
        memcpy(httpHeader->url,&p[5],strlen(p) - 14);
    }

    printf("%s\n",httpHeader->url);
    p = strtok_r(NULL,delim,&ptr);
    while(p){
        switch(p[0]){
            case 'H'://Host
                memcpy(httpHeader->host,&p[6],strlen(p) - 6);
                break;
            case 'C'://Cookie
                if(strlen(p) > 8){
                    char header[8];
                    memset(header, 0, sizeof(header));
                    memcpy(header, p, 6);
                    if(!strcmp(header,"Cookie")){
                        memcpy(httpHeader->cookie,&p[8],strlen(p) -8);
                    }
                }
                break;
            default:
                break;
        }
        p = strtok_r(NULL,delim,&ptr);
    }
    //�ж��Ƿ��ֹ����
    ban_header(httpHeader, buffer);
    //�ж��Ƿ�Ҫ����
    transfer_header(httpHeader, buffer);
    return TRUE;
}
//************************************
// Method: ConnectToServer
// FullName: ConnectToServer
// Access: public
// Returns: BOOL
// Qualifier: ������������Ŀ��������׽��֣�������
// Parameter: SOCKET * serverSocket
// Parameter: char * host
//************************************
BOOL ConnectToServer(SOCKET *serverSocket,char *host){
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(HTTP_PORT);
    HOSTENT *hostent = gethostbyname(host);
    if(!hostent){
        return FALSE;
    }
    in_addr Inaddr=*((in_addr*) *hostent->h_addr_list);
    serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(Inaddr));
    *serverSocket = socket(AF_INET,SOCK_STREAM,0);
    if(*serverSocket == INVALID_SOCKET){
        return FALSE;
    }
    if(connect(*serverSocket,(SOCKADDR *)&serverAddr,sizeof(serverAddr))== SOCKET_ERROR){
        closesocket(*serverSocket);
        return FALSE;
    }
    return TRUE;
}
