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
 * prints message and exits program
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
 * Function: decrypt_message()
 * 
 * accepts the cipher text and key. 
 * return decrypted message. should be the same as original plaintext.
 * ****************************************************************/
char* decrypt_message(char* ciphertxt, char* key){

  for(int i = 0; i < strlen(ciphertxt); i++){

    if(ciphertxt[i] != 32 && key[i] != 32){ //if both chars are A-Z
      ciphertxt[i] = (ciphertxt[i]-65)-(key[i]-65); //convert them to 0-25 and subtract
      if(ciphertxt[i] < 0 ){
        ciphertxt[i] += 27; //add 27 if negative
      }
      ciphertxt[i] += 65; //convert back to A-Z
      if(ciphertxt[i] == 91){
        ciphertxt[i] = 32; //91 is a space
      }
    }else if(ciphertxt[i] == 32 && key[i]!= 32){ //if one is space and other is A-Z
      ciphertxt[i] = (26 - (key[i]-65)); //26 is space
      if(ciphertxt[i] < 0 ){
        ciphertxt[i] += 27;
      }
      ciphertxt[i] += 65; //convert back to A-Z
      if(ciphertxt[i] == 91){ //91 is space
        ciphertxt[i] = 32;
      }
    }else if(ciphertxt[i] != 32 && key[i] == 32){
      ciphertxt[i] = ((ciphertxt[i]-65) - 26);
      if(ciphertxt[i] < 0 ){
        ciphertxt[i] += 27;
      }
      ciphertxt[i] += 65;
      if(ciphertxt[i] == 91){
        ciphertxt[i] = 32;
      }
    }else if(ciphertxt[i] == 32 && key[i] == 32){ //if both are spaces
      ciphertxt[i] = 65; //result will always be 65
    }
  }
  return ciphertxt;
}


int main( int argc, char *argv[]){

  int connectionSocket, charsRead, charsWritten, count = 0;
  char buffer[1000];
  char *msg; 
  
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
      
      //get data from dec_client. should be one long message.
      do{
        memset(buffer, '\0', 1000);
        charsRead = recv(connectionSocket, buffer, sizeof(buffer)-1, 0);
        if(charsRead < 0){ error("ERROR reading from socket");}
        strcat(msg, buffer); //add buffer to msg each call
      }while(strstr(msg, "||") == NULL); // || is the termination character that will signify end of transmission

      char client_id = msg[0]; //first char in message is the client identifier. should be "E" for this server.
      char *ciphertext = strtok(msg, "+"); //+ is in between ciphertext and key
      ciphertext = ciphertext+1; //remove 'e' symbolizing client type
      char *key = strtok(NULL, "|\n"); //extract key
    
      char *DecMessage = decrypt_message(ciphertext, key);
      if(client_id == 'e'){
        strcat(DecMessage, "-"); //add char telling client communication is not allowed
      }
      strcat(DecMessage, "||"); //add termination chars
      
      if(client_id != 'e'){
        fprintf(stderr, "SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
      }
      count = 0;
      while(count < strlen(DecMessage)){
        charsWritten = send(connectionSocket, DecMessage, strlen(DecMessage), 0); //ensure all bytes were sent
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
