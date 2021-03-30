/******************************************
 * Author: Carlos Gonzalez
 * 
 * Program 4: Multi-threaded Producer Consumer Pipeline
 * 
 * Date: Nov. 14, 2020
 * **/
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#define NUM_LINES 50
#define BUF_SIZE 1000


//First buffer. Connects input thread and line-separator thread.
char buffer_1[NUM_LINES][BUF_SIZE];
int count_1 = 0;
int prod_idx_1 = 0;
int con_idx_1 = 0;
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

//Second buffer. Connects line-separator thread and plus-remover thread.
char buffer_2[NUM_LINES][BUF_SIZE];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

//Third buffer. Connects plus_remover thread with output thread.
char buffer_3[NUM_LINES][BUF_SIZE];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;
// 


/**********************************************
 * Function: get_user_input
 * 
 * This function collects input from stdin. could be from user or text file
 * **************/
char* get_user_input(){

    char* userinput = malloc(1000*sizeof(char));
    ssize_t bufsize = 1000;
    getline(&userinput, &bufsize, stdin);

return userinput;
}

/**********************************************
 * Function: put_buff_1()
 * 
 * Buffer between input and line-separator
 * Accepts the string retrieved from get_user_input()
 * **************/
void put_buff_1(char *orig_str){

    pthread_mutex_lock(&mutex_1); //lock before placing items in buffer
    strcpy(buffer_1[prod_idx_1], orig_str); //copy string to next spot in buffer
    prod_idx_1++;
    count_1++;

    pthread_cond_signal(&full_1); //signal to consumer that buffer is no longer empty
    pthread_mutex_unlock(&mutex_1);

}

/**********************************************
 * Function: get_input()
 * 
 * function used by thread 1, (input thread).
 * continuously calls get_user_input until user enters "STOP"
 * places input in buffer 1
 * **************/
void *get_input(void *args){

    char *item;
    int count = 0; //should be no more than 50 lines entered

    item = get_user_input();
    put_buff_1(item);
    count++;
    
    while((strcmp(item, "STOP\n") != 0) && (count <=50)){ //keep getting input until max lines or "STOP" is entered
        free(item);
        item = get_user_input();
        put_buff_1(item);
        count++;
    }
    
    free(item);

return NULL;
}

/******************************************
 * Function: get_buff_1()
 * 
 * This function gets data from buffer 1
 * ************/
char *get_buff_1(){

    pthread_mutex_lock(&mutex_1);
    while(count_1 == 0)
        pthread_cond_wait(&full_1, &mutex_1); //wait until buffer 1 has data
    
    char *item = buffer_1[con_idx_1]; //get the next line in buffer
    con_idx_1++;
    count_1--;
    pthread_mutex_unlock(&mutex_1);

return item;
}

/******************************************
 * Function: put_buff_2()
 * 
 * accepts a string
 * places string in buffer 2 which connect line-separator thread and plus-remover thread.
 * ************/
void put_buff_2(char* str){

    pthread_mutex_lock(&mutex_2);
    strcpy(buffer_2[prod_idx_2], str);
    prod_idx_2++;
    count_2++;
    pthread_cond_signal(&full_2);
    pthread_mutex_unlock(&mutex_2);

}

/******************************************
 * Function: lin_sep()
 * 
 * function used by thread 2 to removed newline characters
 * places new str in buffer 2
 * ************/
void *line_sep(void *args){

    char* str;
    do{
        str = get_buff_1();
        for(int i=0; i < strlen(str); i++){
            if(str[i] == '\n'){
                str[i] = ' ';
            }
        }
        put_buff_2(str);
    }while(strcmp(str, "STOP ") != 0);

return NULL;
}

/******************************************
 * Function: get_buff_2()
 * 
 * method in which thread 3 collects data from buffer 2.
 * ************/
char *get_buff_2(){

    pthread_mutex_lock(&mutex_2);
    while(count_2 == 0)
        pthread_cond_wait(&full_2, &mutex_2); //wait until buffer has data
    
    char *item = buffer_2[con_idx_2];
    con_idx_2++;
    count_2--;
    pthread_mutex_unlock(&mutex_2);

return item;
}

/******************************************
 * Function: put_buff_3()
 * 
 * Method for thread 3 to send data to thread 4.
 * ************/
void put_buff_3(char *str){

    pthread_mutex_lock(&mutex_3);
    strcpy(buffer_3[prod_idx_3],str);
    prod_idx_3++;
    count_3++;
    pthread_cond_signal(&full_3);
    pthread_mutex_unlock(&mutex_3);

}

/******************************************
 * Function: replace_pluses()
 * 
 * function used by thread 3 to removed double plus signs, "++", and replace with a single carat, '^'.
 * placed into buffer 3 for thread 4 to collect.
 * ************/
void *replace_pluses(void *args){

    char* str, *temp;
    int temp_idx = 0, c = 0;
    do{
        str = get_buff_2();
        temp = malloc(1000*sizeof(char));
        temp_idx = 0;
        c =0;
        for(int i=0; i <strlen(str)-c+1; i++){
            if( (str[i+c] == '+') && (str[i+c+1] == '+')){
                c++;
                temp[temp_idx] = '^';
                temp_idx++;
            }else{
                temp[temp_idx] = str[i+c];
                temp_idx++;
            }
        }
        strcpy(str, temp);
        put_buff_3(str);
        free(temp);
    }while( strcmp(str, "STOP ") != 0);

return NULL;
}

/******************************************
 * Function: get_buff_3()
 * 
 * Method in which thread 4 collects data from thread 3.
 * ************/
char *get_buff_3(){

    pthread_mutex_lock(&mutex_3);
    while(count_3 == 0)
        pthread_cond_wait(&full_3, &mutex_3); //wait for buffer to contain data
    
    char *item = buffer_3[con_idx_3];
    con_idx_3++;
    count_3--;
    pthread_mutex_unlock(&mutex_3);

return item;
}

/******************************************
 * Function print_item()
 * 
 * Thread 4, output thread, prints lines of exactly 80 chars with an attached line separator. 
 * Must not print if less than 80 chars available to write.
 * ************/
void *print_item(void *args){

    char *item;
    char output[50*1000] = ""; 
    int out_idx = 0; //index tracker for output string
    
    do{
        item = get_buff_3();
        if(strcmp(item, "STOP ") != 0){
            strcat(output, item); //concatenate buffer item to output string
        }
        if(output[out_idx + 79]){ //if 80 chars available
            for(int i =0; i < 80; i++){
                printf("%c", output[out_idx+i]); //print each char
            }
            printf("\n"); //attach new line char
            out_idx = out_idx + 80; //jump to beginning of next 80 chars in output string.
        }  
    }while(strcmp(item, "STOP ") != 0); //repeat until buffer retrieves terminate signal

    //do-while loop does not print last available 80 chars so this is to not leave last 80 chars hanging.
    if(output[out_idx + 79]){
        for(int i =0; i < 80; i++){
            printf("%c", output[out_idx+i]);
        }
        printf("\n");
        out_idx = out_idx + 80;
    }

return NULL;
}


int main(){

  pthread_t input_t, line_sep_t, replace_plus_t, output_t;

  pthread_create(&input_t, NULL, get_input, NULL);
  pthread_create(&line_sep_t, NULL, line_sep, NULL);
  pthread_create(&replace_plus_t, NULL, replace_pluses, NULL);
  pthread_create(&output_t, NULL, print_item, NULL);

  pthread_join(input_t, NULL);
  pthread_join(line_sep_t, NULL);
  pthread_join(replace_plus_t, NULL);
  pthread_join(output_t, NULL);

return 0;
}