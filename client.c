// created by Jordon Zeigler 3/3/2021
// adjusted later on for assignment 2
// code referenced from Professor Girard
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
  #include <winsock2.h>
  #include <Ws2tcpip.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> 
  #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#pragma comment(lib, "Ws2_32.lib")

//#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iphlpapi.h>


#define MAX 10

#define SA struct sockaddr
#define sock
time_t curr_time;


// function intended to read input specifically from client
void readFromConnection(int incomingSocket, char* data)
{
    //declares a temporary string to store input along with
    // emptying it out with bzero
    char tmp[MAX];
    bzero(tmp,MAX);
    //declares count for later use
    int count = 0;

    // while loop to receive from other socket
    while (count != MAX-1)
    {
        int start = count;
        // controls how much is read from connection
        int maxRead = MAX-1 - start;
        count += recv(incomingSocket,tmp,maxRead,0);
        
        //stores characters from tmp into data argument
        for (int x = start; x < count; x++)
        {
            data[x] = tmp[x-start];
        }
    }

    
}

void *alternateFunc(void *sockfd){
  char buff[10];
  int s = *(int*)sockfd;
  printf("entered other thread\n");
  while(1){
    FILE  *fp2 = fopen("client_received_log.txt", "a");
    bzero(buff, sizeof(buff));
    // reads adjusted string from server
        readFromConnection(s, buff);
        if(strlen(buff) == 0){
          break;
        }
        printf("%s\n", buff);
        fflush(stdout);
        time(&curr_time);
        fprintf(fp2, "message received at %s from server: %s\n", ctime(&curr_time), buff);
        fclose(fp2);
        

  }
  
}

//void print_reverse(char* reverse){
  //printf("%s\n", reverse);
//}
// designed for chat between client and server
void func(int sockfd)
{
    // stores output from other socket
    char buff[MAX];
    int n;
    
    //int len = strlen(f)-1;

    // opens both files up for reading and writing respectively
    //FILE *fp = fopen(f, "r");
   
    
    // while loop to continuously read from connection
    while(1){
       FILE  *fp3 = fopen("client_sent_log.txt", "a");
        // empties out buff string
        bzero(buff, sizeof(buff));

        // reads text from standard input
        scanf("%10s", buff);
        fflush(stdin);
        printf("before for loop\n");
        // adds spaces onto the buff string if it isn't ten characters long
        if(strlen(buff) < MAX){
          for(int i = strlen(buff); i < MAX; i++){
            buff[i] = ' ';
            
          }
        }
        printf("after for loop\n");

        // adds \0 onto the end to ensure string can end correctly
        if(strlen(buff) > MAX)
        {
          buff[MAX] = '\0';
        }
        
      
        // sends string to server
        send(sockfd, buff, sizeof(buff), 0);
        time(&curr_time);
        fprintf(fp3, "message sent at %s from server: %s\n", ctime(&curr_time), buff);
        // reads adjusted string from server
        //readFromConnection(sockfd, buff);
        //printf("%s", buff);
        
        // writes results into second file
        //fwrite(buff, 1, strlen(buff), fp2);
        printf("end of client\n");
        fclose(fp3);
        
        
        

    }
    // closes both files
    //fclose(fp);
    
    
}

int main(int argc, char * argv[])
{
    // declares ints for saving socket info
    int sockfd, connfd;
    // declares sockaddr_in variables for the server and client
    struct sockaddr_in servaddr, client;
    //char ip[50];
    int i = 0;
    char c;
    printf("please enter IP address:\n");
    char ip[16];
    scanf("%15s", ip);
    //while((c = getchar()) != '\n' && i < 50){
      //ip[i] = c;
      //i++;
    //}
    //ip[i] = '\0';
    char port_num[4];
    i = 0;
    printf("please enter port number\n");
    scanf("%4s", port_num);
    //while((c = getchar()) != '\n' && i < 4){
    //  port_num[i] = c;
    //  i++;
    //}
    //port_num[i] = '\0';
    //printf("please enter IP address:\n");
    //scanf("%s", ip);
    //printf("please enter port number\n");
    //scanf("%s", port_num);

    //socket creation note leftover from when it was originally being developed as cross platform 
    // rather than solely linux/unix
    #ifdef _WIN32
        //sockfd = sockInit();
    #endif
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
     // basic messages that can vary if socket creation was successful
    if (sockfd == -1) 
    {
        printf("socket creation failed\n");
        exit(0);
    }
    else{
        printf("socket created\n");
    }

    struct in_addr addr;
    // clears servaddr memory
    bzero(&servaddr, sizeof(servaddr));

    // assign IP and PORT
    int port = atoi(port_num);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_aton(ip, &addr);
    servaddr.sin_port = htons(port);

    // connect client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with server failed\n");
        exit(0);
    }
    else
    {
        printf("connected to the server\n");
    }
    pthread_t sniffer_thread;
    pthread_create( &sniffer_thread, 0, alternateFunc, &sockfd);
    // function to initiate chat
    func(sockfd);
    //printf("function exited\n");
    // closes the soccket
    close(sockfd);
}