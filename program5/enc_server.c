#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>


/*******************************************************************
 * Function: error()
 * 
 * prints error message and exits program
 * ****************************************************************/
void error(const char *msg){
  perror(msg);
  exit(1);
}

/*******************************************************************
 * Function: setupAddressStruct()
 * 
 * setups up the address by setting IPv4 and portnumber
 * ****************************************************************/
void setupAddressStruct(struct sockaddr_in* address, int portNumber){

  memset((char*)address, '\0', sizeof(*address));

  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}

/*******************************************************************
 * Function: encrypt_message()
 * 
 * accepts the plaintext and key
 * returns the encrypted message
 * ****************************************************************/
char* encrypt_message(char* pltxt, char* key){

  for(int i = 0; i < strlen(pltxt); i++){

    if(pltxt[i] != 32 && key[i] != 32){ //if both are A-Z
      pltxt[i] = (pltxt[i]-65)+(key[i]-65); //get them in the range 0-25 and add together
      if(pltxt[i] > 26){
        pltxt[i] -= 27;
      }
      pltxt[i] += 65;
      if(pltxt[i] == 91){
        pltxt[i] = 32;//91 means char should be a space
      }
    }else if(pltxt[i] == 32 && key[i]!= 32){ //if one is space and other is A-Z
      pltxt[i] = 26 + (key[i]-65); //26 is a space
      if(pltxt[i] > 26){
        pltxt[i] -= 27;
      }
      pltxt[i] += 65;
      if(pltxt[i] == 91){
        pltxt[i] = 32;
      }
    }else if(pltxt[i] != 32 && key[i] == 32){// if key is space and plaintext is A-Z for i
      pltxt[i] = (pltxt[i]-65) + 26;
      if(pltxt[i] > 26){
        pltxt[i] -= 27;
      }
      pltxt[i] += 65;
      if(pltxt[i] == 91){
        pltxt[i] = 32;
      }
    }else if(pltxt[i] == 32 && key[i] == 32){ //if both are spaces 
      pltxt[i] = 90; //result will always be 90
    }
  }

  return pltxt;
}


int main( int argc, char *argv[]){

  int connectionSocket, charsRead, charsWritten, num_pids = 0, count = 0;
  char buffer[1000];
  char *msg; 
  pid_t pid_arr[5];

  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  //check usage and args
  if(argc < 2){
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }
  //create a listening socket
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(listenSocket < 0){
    error("ERROR opening socket");
  }
  //set up address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  //bind socket to the port
  if(bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }
  //begin listening for connections. Allow up to 5 in queue
  listen(listenSocket, 5);


  while(1){
    //accept the conenction request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr*)&clientAddress, &sizeOfClientInfo);
    if(connectionSocket < 0){
      error("ERROR on accept");
    }
    
    //fork a child
    pid_t spawnpid = fork();
    if(spawnpid == -1){
      perror("fork() failed.\n");
    }
    if(spawnpid == 0){
      
      msg = malloc(500000*sizeof(char)); //msg will hold all data because its easier to just have client send everything at once than to make multiple recv/send calls. idk why.
      
      //get data from enc_client. should be one long message.
      do{
        memset(buffer, '\0', 1000);
        charsRead = recv(connectionSocket, buffer, sizeof(buffer)-1, 0);
        if(charsRead < 0){ error("ERROR reading from socket");}
        strcat(msg, buffer); //add buffer to msg each time until termination char
      }while(strstr(msg, "||") == NULL); // || is the termination character that will signify end of transmission



      char client_id = msg[0]; //first char in message is the client identifier. should be "e" for this server.
      char *plaintext = strtok(msg, "+"); //+ is in between plaintext and key
      plaintext = plaintext+1; //remove 'e' symbolizing client type
      char *key = strtok(NULL, "|\n"); //extract key
      
      
      char *EncMessage = encrypt_message(plaintext, key);
      if(client_id == 'd'){
        strcat(EncMessage, "-"); //if dec_client send char telling it that connection is not allowed.
      }
      strcat(EncMessage, "||"); //add termination chars
      
      if(client_id != 'd'){
        fprintf(stderr, "SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
      }
      count = 0;
      while(count < strlen(EncMessage)){
        charsWritten = send(connectionSocket, EncMessage, strlen(EncMessage), 0); //send until all bytes have been written
        if(charsWritten < 0){ error("ERROR writing to socket");}
        count += charsWritten;
      }
      
     
      free(msg);
      close(connectionSocket);
       
    }  

    close(connectionSocket);
  }
  close(listenSocket);

return 0;
}
