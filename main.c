#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dir.h>
#include <process.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <winsock2.h>
#include <time.h>
#include "cJSON.c"
#define MAX 80
#define PORT 12345
#define SA struct sockaddr
struct users{
    char name[1000];
    char token[33];
    char channel[100];
};
int i;
int w;
char request_str[5000];
char* response;
char address[1000];
char previous_chancontent[10000];
int previous_arraaynum=0;
struct users members[1000];
int Sock_Creator(int server_socket);
void token_creator(char* a);
void registering();
void login();
void logout();
void create_channel();
void join_channel();
void leave();
void send_message();
void membersOFchannel();
void refresh();
void wtd_request(int server_socket){
    int sock;
    sock=Sock_Creator(server_socket);
    recv(sock,request_str,5000,0);
    printf("%s\n",request_str);
    if(strncmp(request_str,"register ",9)==0)
        registering();
    if(strncmp(request_str,"login ",6)==0)
        login();
    if(strncmp(request_str,"logout ",7)==0)
        logout();
    if(strncmp(request_str,"create channel ",15)==0)
        create_channel();
    if(strncmp(request_str,"join channel ",13)==0)
        join_channel();
    if(strncmp(request_str,"leave ",6)==0)
        leave();
    if(strncmp(request_str,"send ",5)==0)
        send_message();
    if(strncmp(request_str,"channel members ",16)==0)
        membersOFchannel();
    if (strncmp(request_str,"refresh ",8)==0)
        refresh();
    if(sock == -1)
        return;
    printf("%s\n",response);
    send(sock,response,strlen(response),0);
    closesocket(sock);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
    server_socket = socket(AF_INET, SOCK_STREAM, 6);
	if (server_socket == INVALID_SOCKET)
        wprintf(L"socket function failed with error = %d\n", WSAGetLastError() );
    else
        printf("Socket successfully created..\n");
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    if ((bind(server_socket, (SA *)&server, sizeof(server))) != 0)
    {
        printf("Socket binding failed...\n");
        return -1;
    }
    else
        printf("Socket successfully bound..\n");
    memset(previous_chancontent,0,10000);
    while(1){
        memset(request_str,0,sizeof(request_str));
        wtd_request(server_socket);
    }
    return 0;
}

int Sock_Creator(int server_socket){
    struct sockaddr_in client;
    if ((listen(server_socket, 5)) != 0)
    {
        printf("Listen failed...\n");
        return -1;
    }
    else
        printf("Server listening..\n");

    int len = sizeof(client);
    int client_socket = accept(server_socket, (SA *)&client, &len);
    if (client_socket < 0)
    {
        printf("Server accceptance failed...\n");
        return -1;
    }
    else{
        printf("Server acccepted the client...\n");
        return client_socket;
    }
}

void token_creator(char* user){
    srand(time(NULL));
    strcpy(members[i].name, user);
    for (int j=0;j<32;j++){
        members[i].token[j]=65+(rand()%26);
    }
    i++;
}

void registering(){
    char user[1000], pass[1000];
    sscanf(request_str,"%*s%*c%s%s", user, pass);
    int length = strlen(user);
    user[length-1]=0;
    printf("%s : %s\n",user, pass);
    char* dirname = "Resources";
    mkdir(dirname);
    mkdir("Resources/Users");
    mkdir("Resources/Channels");
    char path[1000];
    strcpy(path,"Resources/Users/");
    strcat(path,user);
    strcat(path,".json");
    FILE *cfPtr;
    if ((cfPtr = fopen(path, "r"))==NULL){
        if ((cfPtr = fopen(path, "w"))==NULL){
            printf("File failed to open!");
        }
        else {
            cJSON* client = cJSON_CreateObject();
            cJSON_AddStringToObject(client,"username", user);
            cJSON_AddStringToObject(client,"password", pass);
            char* jsonclient = cJSON_PrintUnformatted(client);
            fprintf(cfPtr,"%s",jsonclient);
        }
        cJSON* success=cJSON_CreateObject();
        cJSON_AddStringToObject(success,"type","Successful");
        cJSON_AddStringToObject(success,"content","");
        response = cJSON_PrintUnformatted(success);
        fclose(cfPtr);
    }
    else {
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","This username already exists!");
        response = cJSON_PrintUnformatted(error);
    }
}

void login(){
    char user[1000], pass[1000];
    sscanf(request_str,"%*s%*c%s%s", user, pass);
    int length = strlen(user);
    user[length-1]=0;
    printf("%s : %s\n", user, pass);
    char path[1000];
    strcpy(path,"Resources/Users/");
    strcat(path, user);
    strcat(path,".json");
    FILE *cfPtr;
    if ((cfPtr = fopen(path, "r"))==NULL){
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","This username doesn't exist!");
        response = cJSON_PrintUnformatted(error);
        fclose(cfPtr);
    }
    else {
        fclose(cfPtr);
        for (int j=0;j<i;j++){
            if (strcmp(user,members[j].name)==0){
                cJSON* error=cJSON_CreateObject();
                cJSON_AddStringToObject(error,"type","Error");
                cJSON_AddStringToObject(error,"content","This user is already logged in!");
                response = cJSON_PrintUnformatted(error);
                return;
            }
        }
        FILE* cfPtr2;
        char loginpasscheck[1000];
        cfPtr2=fopen(path, "r");
        fscanf(cfPtr2,"%s",loginpasscheck);
        fclose(cfPtr2);
        cJSON* loginpasscheckj=cJSON_Parse(loginpasscheck);
        printf("%s\n",loginpasscheck);
        cJSON* jsonpass = cJSON_GetObjectItemCaseSensitive(loginpasscheckj,"password");
        memset(loginpasscheck,0,sizeof(loginpasscheck));
        strcpy(loginpasscheck,jsonpass->valuestring);
        if (strcmp(loginpasscheck, pass)==0){
            token_creator(user);
            cJSON* success=cJSON_CreateObject();
            cJSON_AddStringToObject(success,"type","AuthToken");
            cJSON_AddStringToObject(success,"content",members[i-1].token);
            response = cJSON_PrintUnformatted(success);
        }
        else {
            cJSON* error=cJSON_CreateObject();
            cJSON_AddStringToObject(error,"type","Error");
            cJSON_AddStringToObject(error,"content","Wrong password!");
            response = cJSON_PrintUnformatted(error);
        }
    }
}

void logout(){
    char token[1000];
    sscanf(request_str,"%*s%*c%s", token);
    for (int j=0;j<i;j++){
            if (strcmp(token, members[j].token)==0){
                cJSON* success=cJSON_CreateObject();
                cJSON_AddStringToObject(success,"type","Successful");
                cJSON_AddStringToObject(success,"content","");
                response = cJSON_PrintUnformatted(success);
                memset(members[j].name,0,sizeof(members[j].name));
                memset(members[j].token,0,sizeof(members[j].token));
                return;
            }
    }
}

void create_channel(){
    char nameOFchannel[1000];
    sscanf(request_str,"%*s%*s%*c%[^\n]s",nameOFchannel);
    int len=strlen(nameOFchannel);
    char token1[33];
    for(int j=0;j<32;j++)
        token1[j]=nameOFchannel[len+j-32];
    token1[32]='\0';
    for (int j=len;j>=len-34;j--)
        nameOFchannel[j]='\0';
    char name[100];
    int w=-1;
    for (int j=0;j<=i;j++)
        if (strcmp(members[j].token,token1)==0){
            strcpy(name,members[j].name);
            w=j;
        }
    memset(address,0,sizeof(address));
    strcpy(address,"Resources/Channels/");
    strcat(address,nameOFchannel);
    strcat(address,".json");
    printf("%s\n",address);
    FILE *cfPtr;
    if (w==(-1)) {
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","Token not found!");
        response = cJSON_PrintUnformatted(error);
    }
    else{
        if ((cfPtr = fopen(address, "r"))!=NULL){
            cJSON* error=cJSON_CreateObject();
            cJSON_AddStringToObject(error,"type","Error");
            cJSON_AddStringToObject(error,"content","This channel name already exists!");
            response = cJSON_PrintUnformatted(error);
        }
        else {
            fclose(cfPtr);
            cfPtr=fopen(address,"w");
            cJSON* whole = cJSON_CreateObject();
            cJSON* message = cJSON_CreateArray();
            cJSON* m;
            cJSON_AddItemToArray(message , m = cJSON_CreateObject());
            strcat(name," Created -> ");
            strcat(name,nameOFchannel);
            strcat(name," <-");
            strcpy(members[w].channel,nameOFchannel);
            cJSON_AddStringToObject(m, "sender" , "Server");
            cJSON_AddStringToObject(m, "content" , name);
            cJSON_AddItemToObject(whole,"messages",message);
            char* jsonchannel = cJSON_PrintUnformatted(whole);
            printf("%s\n",jsonchannel);
            fprintf(cfPtr,"%s%c",jsonchannel,'\n');
            cJSON* success=cJSON_CreateObject();
            cJSON_AddStringToObject(success,"type","Successful");
            cJSON_AddStringToObject(success,"content","");
            response = cJSON_PrintUnformatted(success);
            w++;
            fclose(cfPtr);
        }
    }
}

void join_channel(){
    char nameOFchannel[1000];
    sscanf(request_str,"%*s%*s%*c%[^\n]s",nameOFchannel);
    int length=strlen(nameOFchannel);
    char token1[33];
    for(int j=0;j<32;j++)
        token1[j]=nameOFchannel[length+j-32];
    token1[32]='\0';
    for (int j=length; j >= length-34; j--)
        nameOFchannel[j]='\0';
    char name[100];
    int w=-1;
    for (int j=0;j<=i;j++)
        if (strcmp(members[j].token,token1)==0){
            strcpy(name,members[j].name);
            w=j;
        }
    memset(address,0,sizeof(address));
    strcpy(address,"Resources/Channels/");
    strcat(address,nameOFchannel);
    strcat(address,".json");
    printf("%s\n",address);
    FILE *cfPtr;
    if (w==(-1)) {
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","Token not found!");
        response = cJSON_PrintUnformatted(error);
    }
    else{
        if ((cfPtr = fopen(address, "r"))==NULL){
            cJSON* error=cJSON_CreateObject();
            cJSON_AddStringToObject(error,"type","Error");
            cJSON_AddStringToObject(error,"content","This channel name doesn't exist!");
            response = cJSON_PrintUnformatted(error);
        }
        else {
            char chancontent[10000];
            fscanf(cfPtr,"%[^\n]s",chancontent);
            printf("%s\n",chancontent);
            fclose(cfPtr);
            cfPtr=fopen(address,"w");
            cJSON* jsoncont = cJSON_Parse(chancontent);
            cJSON* jsonmessage = cJSON_GetObjectItemCaseSensitive(jsoncont,"messages");
            cJSON* m;
            cJSON_AddItemToArray(jsonmessage , m = cJSON_CreateObject());
            strcat(name," Joined -> ");
            strcat(name,nameOFchannel);
            strcat(name," <-");
            strcpy(members[w].channel,nameOFchannel);
            cJSON_AddStringToObject(m, "sender" , "Server");
            cJSON_AddStringToObject(m, "content" , name);
            cJSON* whole= cJSON_CreateObject();;
            cJSON_AddItemToObject(whole,"messages",jsonmessage);
            char* jsonchannel = cJSON_PrintUnformatted(whole);
            printf("%s\n",jsonchannel);
            fprintf(cfPtr,"%s",jsonchannel);
            cJSON* success=cJSON_CreateObject();
            cJSON_AddStringToObject(success,"type","Successful");
            cJSON_AddStringToObject(success,"content","");
            response = cJSON_PrintUnformatted(success);
            w++;
            fclose(cfPtr);
        }
    }
}

void leave(){
    char token1[33];
    sscanf(request_str,"%*s%s",token1);
    int j;
    for (j=0;j<i;j++){
        if (strcmp(token1, members[j].token) == 0){
            break;
        }
    }
    FILE* cfPtr;
    cfPtr = fopen(address, "r");
    char chancontent[10000];
    fscanf(cfPtr,"%[^\n]s",chancontent);
    printf("%s\n",chancontent);
    fclose(cfPtr);
    cfPtr=fopen(address,"w");
    cJSON* jsoncont = cJSON_Parse(chancontent);
    cJSON* jsonmessage = cJSON_GetObjectItemCaseSensitive(jsoncont,"messages");
    cJSON* m;
    cJSON_AddItemToArray(jsonmessage , m = cJSON_CreateObject());
    char name[1000];
    strcpy(name,members[j].name);
    strcat(name," Left channel -> ");
    strcat(name,members[j].channel);
    strcat(name," <-");
    cJSON_AddStringToObject(m, "sender" , "Server");
    cJSON_AddStringToObject(m, "content" , name);
    cJSON* whole= cJSON_CreateObject();;
    cJSON_AddItemToObject(whole,"messages",jsonmessage);
    char* jsonchannel = cJSON_PrintUnformatted(whole);
    printf("%s\n",jsonchannel);
    fprintf(cfPtr,"%s",jsonchannel);
    cJSON* success=cJSON_CreateObject();
    cJSON_AddStringToObject(success,"type","Successful");
    cJSON_AddStringToObject(success,"content","");
    response = cJSON_PrintUnformatted(success);
    memset(members[j].channel,0,sizeof(members[j].channel));
    memset(previous_chancontent,0,sizeof(previous_chancontent));
    previous_arraaynum=0;
    fclose(cfPtr);
}

void send_message(){
    char message[1000];
    sscanf(request_str,"%*s%*c%[^\n]s",message);
    int length=strlen(message);
    char token1[33];
    for(int j=0;j<32;j++)
        token1[j]=message[length+j-32];
    token1[32]='\0';
    for (int j=length; j >= length-34; j--)
        message[j]='\0';
    char name[100];
    int w=-1;
    for (int j=0;j<=i;j++)
        if (strcmp(members[j].token,token1)==0){
            strcpy(name,members[j].name);
            w=j;
        }
    FILE *cfPtr;
    if (w==(-1)) {
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","Token not found!");
        response = cJSON_PrintUnformatted(error);
    }
    else{
        cfPtr = fopen(address, "r");
        char chancontent[10000];
        fscanf(cfPtr,"%[^\n]s",chancontent);
        printf("%s\n",chancontent);
        fclose(cfPtr);
        cfPtr=fopen(address,"w");
        cJSON* jsoncont = cJSON_Parse(chancontent);
        cJSON* jsonmessage = cJSON_GetObjectItemCaseSensitive(jsoncont,"messages");
        cJSON* m;
        cJSON_AddItemToArray(jsonmessage , m = cJSON_CreateObject());
        cJSON_AddStringToObject(m, "sender" , name);
        cJSON_AddStringToObject(m, "content" , message);
        cJSON* whole= cJSON_CreateObject();;
        cJSON_AddItemToObject(whole,"messages",jsonmessage);
        char* jsonchannel = cJSON_PrintUnformatted(whole);
        printf("%s\n",jsonchannel);
        fprintf(cfPtr,"%s",jsonchannel);
        cJSON* success=cJSON_CreateObject();
        cJSON_AddStringToObject(success,"type","Successful");
        cJSON_AddStringToObject(success,"content","");
        response = cJSON_PrintUnformatted(success);
        fclose(cfPtr);
    }
}

void membersOFchannel(){
    char token1[33];
    sscanf(request_str,"%*s%*s%s",token1);
    char name[100];
    int w=-1;
    for (int j=0;j<=i;j++)
        if (strcmp(members[j].token,token1)==0){
            strcpy(name,members[j].name);
            w=j;
        }
    if (w==(-1)) {
        cJSON* error=cJSON_CreateObject();
        cJSON_AddStringToObject(error,"type","Error");
        cJSON_AddStringToObject(error,"content","Token not found, user hasn't joined the channel!");
        response = cJSON_PrintUnformatted(error);
    }
    else{
        cJSON* whole = cJSON_CreateObject();
        cJSON* memlist = cJSON_CreateArray();
        for (int j=0;j<=i;j++){
            if(strcmp(members[w].channel, members[j].channel)==0){
                cJSON_AddItemToArray(memlist,cJSON_CreateString(members[j].name));
            }
        }
        cJSON_AddStringToObject(whole,"type","List");
        cJSON_AddItemToObject(whole,"content",memlist);
        response = cJSON_PrintUnformatted(whole);
    }
}

void refresh(){
    FILE* cfPtr;
    char chancontent[10000];
    memset(chancontent,0,sizeof(chancontent));
    cfPtr = fopen(address,"r");
    fscanf(cfPtr,"%[^\n]s",chancontent);
    fclose(cfPtr);
    cJSON* jsonchancontent=cJSON_CreateArray();
    cJSON* saver=cJSON_Parse(chancontent);
    cJSON* message=cJSON_GetObjectItemCaseSensitive(saver,"messages");
    for (int j=previous_arraaynum;j<cJSON_GetArraySize(message);j++){
        printf("---%d---\n",j);
        printf("--%s\n",cJSON_PrintUnformatted(jsonchancontent));
        printf("--%s\n",cJSON_PrintUnformatted(message));
        if (j==previous_arraaynum)
            cJSON_AddItemToArray(jsonchancontent,cJSON_GetArrayItem(message, j));
    }
    printf("refresh done\n");
    previous_arraaynum=cJSON_GetArraySize(message);
    cJSON* whole=cJSON_CreateObject();
    cJSON_AddStringToObject(whole,"type","List");
    cJSON_AddItemToObject(whole,"content",jsonchancontent);
    response = cJSON_PrintUnformatted(whole);
}

