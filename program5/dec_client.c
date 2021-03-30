#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>

/*******************************************************************
 * Function: error()
 * 
 * printf message and exits program
 * ****************************************************************/
void error(const char* msg){
  perror(msg);
  exit(0);
}

/*******************************************************************
 * Function: setupAddressstruct()
 * 
 * setups communication by setting IPv4, port and localhost
 * ****************************************************************/
void setupAddressStruct(struct sockaddr_in* address, int portNumber){

  memset((char*)address, '\0', sizeof(*address));

  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);

  struct hostent* hostInfo = gethostbyname("localhost");
  if(hostInfo == NULL){
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }

  memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

}

/*******************************************************************
 * Function: processFile()
 * 
 * opens a file and reads the line. returns string
 * ****************************************************************/
char *processFile(char* filePath){

  FILE* movieFile = fopen(filePath, "r");

  char *currLine = NULL;
  size_t len = 0;
  size_t nread;

  nread = getline(&currLine, &len, movieFile);
  fclose(movieFile);

return currLine;
}

/*******************************************************************
 * Function: check_chars()
 * 
 * takes in a string and checks for valid chars. should be A-Z or space
 * ****************************************************************/
int check_chars (char* s){

  for(int i=0; i < strlen(s); i++){
    if( (s[i] >= 65 && s[i] <= 90) || s[i]==32){
      return 0;
    }else{
      return -1;
    }
  }
}

int main(int argc, char *argv[]){

  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[1000];
  
  //check usage and args
  if(argc < 3){
    fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  //process plaintext and key file
  char *ciphertxt = processFile(argv[1]);
  ciphertxt[strcspn(ciphertxt, "\n")] = '\0';
  //fprintf(stderr, "PLAINTEXT: %s\n", ciphertxt);
  
  char *keystr = processFile(argv[2]);
  keystr[strcspn(keystr, "\n")] = '\0';
  //fprintf(stderr, "KEY: %s\n", keystr);
  
  //verify key length is at least same as pltxt
  if(strlen(keystr) < strlen(ciphertxt)){
    fprintf(stderr, "KEY IS SHORTER THAN PLAINTEXT.\n");
    exit(1);
  }
  //check for illegal chars
  if( check_chars(ciphertxt) == -1 || check_chars(keystr) == -1){
    fprintf(stderr, "CLIENT: Invalid characters in message or key.");
    exit(1);
  }

  //bundle up messages into one large message
  //had trouble making multiple send/recv calls so im done being a nice guy.
  //I'm sending everything at once.
  char FullMessage[500000]; 
  strcat(FullMessage, "d"); //add client id to the front 
  strcat(FullMessage, ciphertxt); //add plaintext
  strcat(FullMessage, "+"); //add + to separate plaintext and key
  strcat(FullMessage, keystr); //add key
  strcat(FullMessage, "||"); //add termination character
  
//------------------------SETUP SOCKET------------------------------------------------
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if(socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  if(connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

//-------------------------SEND PLAINTEXT AND KEY---------------------------------------

  int count = 0;

  //send full message to server and ensure all bytes were sent
  while(count < strlen(FullMessage)){
    charsWritten = send(socketFD, FullMessage, strlen(FullMessage), 0);
    if(charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
    count += charsWritten;
  }
  
  //fprintf(stderr,"CLIENT: sent message\n");
  
  char *DecMessage = malloc(100000*sizeof(char)); //will hold decrypted message w/ termination chars

  do{
    memset(buffer, '\0', sizeof(buffer));
    charsRead = recv(socketFD, buffer, 500, 0); //recv message from server
    if(charsRead < 0) { error("CLIENT: ERROR reading from socket");}
    strcat(DecMessage, buffer); //add to message until buffer receives termination chars
  }while(strstr(DecMessage, "||") == NULL);

  char *DM = strtok(DecMessage, "||"); //strip off termination chars
  
  if(DM[strlen(DM)-1] == '-'){ //if server did not allow communication
    fprintf(stderr, "ERROR a decryption client cannot connect to a encryption server.");
    free(DecMessage);
    close(socketFD);
    exit(EXIT_FAILURE);
  }else{
    DM = strtok(DM, "-");
    fprintf(stdout, "%s\n", DM);
  }
  close(socketFD);
  free(DecMessage);
return 0;
}
 