#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#define PREFIX "movies_"
#define EXTENSION ".csv"

struct movie{
    char* title;
    int year;
    int num_langs;
    char languages[5][20];
    float rating;
    struct movie *next;
};

/*******
 * function: create_languages(char*, struct movie*)
 * 
 * This function accepts a movie struct and an unparsed string of languages. The function parses the 
 * string and places each language string into the movie struct's language array.
 * */
void create_languages(char* langs, struct movie* m){

    char *saveptr;
    int count = 0;

    char* token = strtok_r(langs, ";", &saveptr);

    strcpy(m->languages[count], token);
    count++; 

    token = strtok_r(NULL, ";", &saveptr);
    while(token!=NULL){
        strcpy(m->languages[count], token);
        count++;
        token = strtok_r(NULL, ";", &saveptr);
    }
    m->num_langs = count;
}

/*******
 * function: create_movie(char*, int*)
 * 
 * This function accepts an unparsed line and a pointer to an int that tracks the number of parsed movies.
 * The line is broken into tokens and stored in the movie structs appropriate entry. Each struct and its title is
 * dynamically allocated. Returns the pointer to the struct.
 * */
struct movie* create_movie(char* info){

    
    struct movie* mov = malloc(sizeof(struct movie)); //allocate movie 

    char *saveptr;

    char *token = strtok_r(info, ",", &saveptr);
    mov->title = malloc(1+strlen(token)*sizeof(char)); //allocate title
    strcpy(mov->title, token);

    token = strtok_r(NULL, ",", &saveptr);
    int num = atoi(token);
    mov->year = num;

    token = strtok_r(NULL, "[,]" ,&saveptr); //gives us parsed string of languages
    create_languages(token, mov);

    token = strtok_r(NULL, ",", &saveptr);
    char* ptr;
    double rate = strtod(token, &ptr);
    mov->rating = rate;

    mov->next = NULL;

    return mov;
}

/*******
 * function: processFile()
 * 
 * This function accepts in a csv file.
 * file is processed line by line where movie structs will be allocated and set with each lines info.
 * Returns a linked list of movies.
 * */
struct movie* processFile(char* filePath) {
    FILE* movieFile = fopen(filePath, "r");

    char* currLine = NULL;
    size_t len = 0;
    size_t nread;
    char* token;

    //LL head node
    struct movie* head = NULL;
    //LL tail node
    struct movie* tail = NULL;

    getline(&currLine, &len, movieFile);

    //read line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1) {

        //use current line to make a new movie node
        struct movie* newmov = create_movie(currLine);

        //first node?
        if (head == NULL) {
            //then this is first node in LL
            //set head and tail to this node
            head = newmov;
            tail = newmov;
        }
        else {
            //not first node
            //add node to list
            tail->next = newmov;
            tail = newmov;
        }
    }
    free(currLine);
    fclose(movieFile);
    return head;
}


/*******
 * function: create_years()
 * 
 * This function accepts in a movie linked list, and a counter for number of years
 * Dynamically allocates 150 entries of possible movie releases. 
 * Updates the value of counter pointer
 * Returns dynamic int array
 * */
int* create_years(struct movie* m, int *count){   
    
    *count = 0; //start counter at zero
    int *movie_years = (int*)malloc(150*sizeof(int)); //assume first movie was made in 1870
    //struct dynarray *movie_years = create_array();
    movie_years[0] = m->year;
    (*count)++;
    //m = m->next;
    while(m != NULL){
        int has_year = -1;
        for(int i=0; i < *count; i++){ //for every movie node, loop through array of years
            if(m->year == movie_years[i]){ 
                has_year = 1; //if year is already in array, set has_year to 1
            }
        }
        if(has_year == -1){
            movie_years[(*count)] = m->year; //if year not in array, append it
            (*count)++;
        }
        m = m->next;
    }
    
    return movie_years;
}

/*******
 * function: create_direc()
 * 
 * This function accepts the directory name as a string.
 * Creates the directory with set permissions
 * */
void create_direc(char* direc_name){

    int status = mkdir(direc_name,0750);
    if(status == 0){
        printf("Created directory with name %s\n", direc_name);
    }else{
        printf("Unable to create directory.\n");
    }
}

/*******
 * function: free_list(struct movie*)
 * 
 * Travels down movie list, freeing the movie and it's title
 * */
void free_list(struct movie* list){

    while(list != NULL){
        struct movie* temp = list->next;
        free(list->title);
        free(list);
        list = temp;
    }
}

/*******
 * function: create_files()
 * 
 * accepts in array of years, array size, movie list, and directory name
 * creates text files for each movie year, writes title of movie to text file of that movie's year
 * */
void create_files(int *yrs, int nyrs, struct movie* lst, char* direc){

    struct movie* traverse = lst;
    char fname[35]; 

    for(int i=0; i < nyrs; i++){

        int file_descriptor;
        sprintf(fname, "%s/%d.txt", direc, yrs[i]); //format directory path as string
        file_descriptor = open(fname, O_CREAT | O_WRONLY | O_APPEND, 0640); //create and open file
        traverse = lst;
        while(traverse != NULL){
            if(traverse->year == yrs[i]){
                int bts = write(file_descriptor, traverse->title, strlen(traverse->title)); //write title of movie to file if movie's year matches current year text file
                int bts2 = write(file_descriptor, "\n", strlen("\n"));
            }
            traverse = traverse->next;
        }
        close(file_descriptor);
    }

}

/*******
 * function: procf()
 * 
 *This function begins the file processing sequence. accepts name of csv as string.
 * */
void procf(char* filename){
    
    printf("\nNow processing the chosen file named %s\n", filename);
    int rand_num = rand() % 100000;
    char direc[25];
    sprintf(direc, "gonzalc5.movies.%d", rand_num);
    create_direc(direc);
    
    struct movie* list = processFile(filename); //create movie list
    int num_years;
    
    int *years = create_years(list, &num_years); //create array of movie years
    create_files(years, num_years, list, direc); //write to files

    free(years);
    free_list(list);
}

/*******
 * function: find_lf()
 * 
 * This function finds the largest csv file with prefix "movies_" then calls the procf function
 * */
void find_lf(){

    DIR* currDir = opendir(".");
    struct dirent *aDir;
    off_t FileSize;
    struct stat dirStat;
    int i = 0;
    char entryname[256];
    char filename[256];

    while((aDir = readdir(currDir)) != NULL){
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, aDir->d_name);
        int f_len = strlen(filename);
        char* file_ext = &filename[f_len-4] ; //store the extension of current file 
        if( (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0) && (strcmp(EXTENSION, file_ext) ==0)){ //if file has prefix "movies_" and extension ".csv"
            stat(aDir->d_name, &dirStat);
        
            if(i == 0 || (dirStat.st_size > FileSize)){ //if file is larger than current largest file, update
                FileSize = dirStat.st_size;
                memset(entryname, '\0', sizeof(entryname));
                strcpy(entryname, aDir->d_name);
            }
        i++;
        }
    }
    closedir(currDir);
    printf("Largest file: %s\n", entryname);
    procf(entryname); //process largest file
}

/*******
 * function: find_sf()
 * 
 * finds and processes the smallest csv file with prefix "movies_"
 * */
void find_sf(){

    DIR* currDir = opendir(".");
    struct dirent *aDir;
    off_t FileSize;
    struct stat dirStat;
    int i = 0;
    char entryname[256];
    char filename[256];

    while((aDir = readdir(currDir)) != NULL){
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, aDir->d_name);
        int f_len = strlen(filename);
        char* file_ext = &filename[f_len-4] ;//store the extension of current file
        if( (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0) && (strcmp(EXTENSION, file_ext) ==0)){//if prefix is "movies_" and extension is ".csv"
            stat(aDir->d_name, &dirStat);
        
            if(i == 0 || (dirStat.st_size < FileSize)){//if file smaller than current smallest, update
                FileSize = dirStat.st_size;
                memset(entryname, '\0', sizeof(entryname));
                strcpy(entryname, aDir->d_name);
            }
        i++;
        }
    }
    closedir(currDir);
    printf("Smallest file: %s\n", entryname);
    procf(entryname); //process file
}

/*******
 * function: scan_dir()
 * 
 * scans top directory for file matching name entered by user.
 * returns int indicating if file found
 * */
int scan_dir(const char* filename){

    DIR* currDir = opendir(".");
    struct dirent *aDir;
    int found = 0;

    while( (aDir = readdir(currDir)) != NULL){
        if( strcmp(aDir->d_name, filename) == 0){
            found = 1;
        }
    }
    closedir(currDir);
    return found;
}

/*******
 * function: find_specf()
 * 
 * ask user to name specific file they wish to process. returns to main menu if file not found
 * */
int find_specf(){

    char fname[35];
    printf("Enter the name of the file you wish to process: ");
    scanf("%s", fname);
    int matched = scan_dir(fname);
    int num;
    if(matched == 1){
        printf("File found.\n");
        procf(fname);
        num = 1;
    }else{
        printf("File not found\n");
        num = 0;
    }
return num;
}

/*******
 * function: process_menu()
 * 
 * ask user which file to process, calls appropriate process function
 * */
void process_menu(){
    int specf = 1;
    int process_choice;
    do{
        
        printf("\n");
        printf("Which file do you want to process?\n");
        printf("Enter 1 to pick the largest file.\n");
        printf("Enter 2 to pick the smallest file.\n");
        printf("Enter 3 to specify the name of a file.\n");
        printf("Enter a choice from 1 to 3: ");
        scanf("%d", &process_choice);

        while(process_choice < 1 || process_choice > 3){
            printf("Please choose a number between 1 and 3: ");
            scanf("%d", &process_choice);
        }

        
        if(process_choice == 1){
            find_lf();
        }
        if(process_choice == 2){
            find_sf();
        }
        if(process_choice == 3){
            specf = find_specf();
        }
    }while(specf == 0 && process_choice == 3); // repeat if user's chosen file is not found
}

/*******
 * function: main_menu()
 * 
 * shows main menu to user, asks if they wish to exit or process file
 * */
void main_menu(){

    int menu_choice;
    do{
    printf("\n");
    printf("1. Select a file to process.\n");
    printf("2. Exit the program.\n");
    printf("Enter a choice 1 or 2: ");
    scanf("%d", &menu_choice);

    while(menu_choice < 1 || menu_choice > 2){
        printf("Please choose 1 or 2: ");
        scanf("%d", &menu_choice);
    }

    if(menu_choice == 1){
        process_menu();
    }
    }while(menu_choice == 1);
}

int main(int argc, char* argv[]){

    srand(time(0));
    
    main_menu();
  
    return 0;
}