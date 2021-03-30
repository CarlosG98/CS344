#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>





int main(int argc, char* argv[]){

  if(argc < 2){
    perror("USAGE: keygen key_length\n");
    exit(EXIT_FAILURE);
  }

  srand(time(0));
  int keyl = atoi(argv[1]);
  char *key = malloc(1+keyl*sizeof(char));
  int keych;
  for(int i=0; i < keyl; i++){
    keych = rand() % 27;
    if(keych != 26){
      key[i] = (char)(65 + keych);
    }else{
      key[i] = (char)(32);
    }
  }

  printf("%s\n", key);
  free(key);

return 0;

}
