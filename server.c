// created by Jordon Zeigler 3/3/2021
// adjusted later for assignment 2
// code referenced from Professor Girard
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
  #include <winsock2.h>
  #include <Ws2tcpip.h>
#else
  #include <sys/socket.h>
  //#include <arpa/inet.h>
  #include <netdb.h> 
  #include <unistd.h>
#endif
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#ifndef _WIN32
  #define SOCKET int
#endif
#define MAX 10
#define PORT 4446
#define SA struct sockaddr
#define sock
#define CLIENT_MAX 3
#pragma comment(lib, "WS2_32.lib")
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int client_list[3];
int num_clients = 0;
time_t curr_time;

struct connectInfo{
    int fd;
    int num_con;
};


// function intended to read input specifically from client
void readFromConnection(int incomingSocket, char* data)
{
    //declares a temporary string to store input along with
    // emptying it out with bzero
    char tmp[MAX+1];
    bzero(tmp,MAX+1);
    //declares count for later use
    int count = 0;
    
    // while loop to receive from other socket
    while (count != MAX)
    {
        int start = count;
        // controls how much is read from connection
        int maxRead = MAX - start;
        
        count += recv(incomingSocket,tmp,maxRead,0);
        // in case count fails, break out of the loop
        if(count == 0)
        {
          break;
        }
        
        //stores characters from tmp into data argument
        for (int x = start; x < count; x++)
        {
            data[x] = tmp[x-start];
        }
    }
}

void sendToNext(int current_con, char* buff){
    if(current_con == num_clients){
       send(client_list[0],buff,strlen(buff),0); 
    }
    else{
        if(num_clients == 1){
            send(client_list[current_con - 1],"only one connection",20,0);
        }else{
        // the reason the array position is determined by just the current condition number is because 
            send(client_list[current_con],buff,strlen(buff),0);
            printf("%s\n", buff+1);
        }
    }

}

void sendToAll(char* buff){
    for(int i = 0; i < num_clients; i++){
        send(client_list[i],buff,strlen(buff),0);
    }

}


// designed for chat between client and server
void * func(void *con)
{
    printf("got here\n");
    struct connectInfo conInfo = *(struct connectInfo*)con;
    // stores output from other socket
    char buff[MAX+1];
    // opens log file for storing info concerning connection
    //FILE *fp = fopen("log", "a");
    int n;
    FILE  *fp2 = fopen("server2log.txt", "a");
    // while loop that will continuously read from the connection, reverse it, then send it back
    while(1){
        // empties out the string so that no characters are left over for the following processes
        bzero(buff,MAX+1);
        // stores socket info
        int s = conInfo.fd;
        
        readFromConnection(s, buff);
        printf("%s\n", buff);
        time(&curr_time);
        
        fprintf(fp2, "message received at %s from client %d\n", ctime(&curr_time), conInfo.num_con);
        // checks if there is no input from connection from client, meaning client has terminated
        // and this while loop should end via break
        if(strlen(buff) == 0){
          break;
        }

        // ensures that last character in string is \0 so that each string can successfully end
        buff[MAX+1] = '\0';


        if(buff[0] == 'F'){
            sendToAll(buff+1);
            fprintf(fp2, "and message was a broadcast type\n");
        }

        if(buff[0] == '1'){
            sendToNext(conInfo.num_con, buff+1);
            fprintf(fp2, "and message was a send to next type\n");
        }
        
        if(buff[0] == '0'){
            // sends result back to client
            printf("in echo back\n");
            //memmove(buff, buff+1, strlen(buff));
            send(client_list[ conInfo.num_con - 1],buff+1,strlen(buff)-1,0);
            fprintf(fp2, "and message was an echo back type\n");
            
        }
        printf("got here too\n");
    }
    fclose(fp2);
    // closes socket
    close(conInfo.fd);
    // declares time variable for the purpose of recording a closing time for the thread
    time_t t2;
    time(&t2);
    
    //fprintf(fp, "closed: %s\n", ctime(&t2));
    // closes log file
    //fclose(fp);
    // exits thread
    pthread_exit(NULL);
    
    
}

int main(int argc, char * argv[])
{
    // declares ints for saving socket info along with a length variable, len
    int sockfd, connfd, *newfd, len;
    // declares sockaddr_in variables for the server and client
    struct sockaddr_in servaddr, client;
    char port_num[4];
    for(int i = 0; i < 4; i++){
        port_num[i] = '\0';
    }
    printf("please enter a port number\n");
    scanf("%4s", port_num);
    // creates a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // basic messages that convery if socket creation was successful
    if (sockfd == -1)
    {
        printf("socket creation failed\n");
        exit(0);
    }
    else
    {
        printf("Socket successfully created\n");
    }

    // clears servaddr memory
    bzero(&servaddr, sizeof(servaddr));

    // assign IP and PORT
    int port = atol(port_num);
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(port);

    // binding newly created socket to given IP
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed\n");
        exit(0);
    }
    else
    {
        printf("socket successfully binded\n");
    }

    // server is ready to listen and verify
    if ((listen(sockfd, 5)) != 0)
    {
        printf("listen failed, we'll get em next time\n");
        exit(0);
    }
    else
    {
        printf("server is now listening\n");
    }
    len = sizeof(client);

    // waits on server to accept connection from client
    while(connfd = accept(sockfd, (SA*)&client, &len))
    {
      // messages indicating whether connection failed or not
      if (connfd < 0) 
      {
          printf("server accept failed\n");
          exit(0);
      }
      else
      {
          printf("server has accepted the client\n");
      }
      
      // string intended to store client's ip address along with actually retrieving it
      pthread_mutex_lock(&lock);
      char client_ip[50];
      inet_ntop(AF_INET, &client.sin_addr, client_ip, sizeof(client_ip));
      
      // retrieves port number of client and stores it in client_port
      int client_port = ntohs(client.sin_port);
      
      // opens log file for storing info concerning connection
      FILE *fp = fopen("log", "a");
      // stores client ip and port into log file
      fprintf(fp,"clients ip: %s\n", client_ip);
      
      fprintf(fp, "clients port: %d\n", client_port);
      // declares time variable to list time that thread was opened in the log file
      time_t t;
      time(&t);
      
      fprintf(fp,"opened: %s\n", ctime(&t));
      
      // closes the log file
      fclose(fp);

      // declares thread variable so that threading can occur
      pthread_t sniffer_thread;
      // stores connection info into newfd so that multithreading can occur
      struct connectInfo con;
      newfd = malloc(sizeof(connfd));
      *newfd = connfd;
      if(num_clients < CLIENT_MAX){
        num_clients++;
        client_list[num_clients - 1] = *newfd;
        con.num_con = num_clients;
        con.fd = *newfd;
      }
      
      
      // calls the func function under a new thread
      if( pthread_create( &sniffer_thread , NULL ,  func, &con) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        pthread_mutex_unlock(&lock);
      
    }
    // close the server
    close(connfd);
    close(sockfd);
}