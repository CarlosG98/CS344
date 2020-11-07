#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dynarray.h"

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
struct movie* create_movie(char* info, int* par){

    (*par)++; //increment movie count
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

    return mov;
}

/*******
 * function: processFile()
 * 
 * This function accepts in a csv file and a pointer to an int that tracks number of movies parsed.
 * file is processed line by line where movie structs will be allocated and set with each lines info.
 * Returns a linked list of movies.
 * */
struct movie* processFile(char* filePath, int* par) {
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
        struct movie* newmov = create_movie(currLine, par);

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

/*
void printMov(struct movie* mov) {
    printf("%s, %d, ", mov->title, mov->year);
    
    if(mov->num_langs==1){
        printf("[%s], ", mov->languages[0]);
    }else{
        printf("[%s", mov->languages[0]);
        for(int i = 1; i < mov->num_langs-1; i++){
            printf(", %s", mov->languages[i]);
        }
        printf("], ");
    }

    printf("%.1f\n", mov->rating);
}


void printMovList(struct movie* list) {
    while (list != NULL) {
        printMov(list);
        list = list->next;
    }
}
*/

/*******
 * function: specific_year(struct movie*)
 * 
 * This function is called when the user chooses to view a list of movies in the given year.
 * The user will enter the year for which movies to see. 
 * */
void specific_year(struct movie* lst){
    int year;
    int found = -1; //indicates if any movie matched requested year
    printf("Enter the year for which you want to see movies: ");
    scanf("%d", &year);

    //struct movie* temp = lst;
    while(lst != NULL){ //traverse list
        if(lst->year == year){
            found = 1; //1 if any movie's year matches requested year
            printf("%s\n", lst->title); //print title if movie's year matches requested year
        }
        lst = lst->next;
    }
    if(found == -1){
        printf("No movies were found.\n"); //print message if no movies matched requested year
    }
}

/*******
 * function: specific_lang(struct movie*)
 * 
 * Function is called when user chooses to view list of movies available in a specific language
 * Prints every movie's year release and title that is available in requested language
 * */
void specific_lang(struct movie* lst){
    char lang[20];
    int found = -1;
    printf("Enter the language for which you want to see movies: ");
    scanf("%s", &lang);

    struct movie* temp = lst; //used temp the traverse list to avoid traversing original list for repeatability. unsure if this is necessary.
    while(temp != NULL){
        for(int i = 0; i < temp->num_langs; i++){
            if( strcmp(temp->languages[i], lang) == 0){
                found = 1;
                printf("%d %s\n", temp->year, temp->title); 
            }
        }
        temp = temp->next;
    }
    if(found == -1){
        printf("No movies were found.\n");
    }
}


/*******
 * function: create_years(struct movie*)
 * 
 * This function accepts a movie linked list  
 * Creates a dynamically allocated array using the functions from dynarray.h
 * Traverses the movie list, examining that movies year release
 * If a new year release is detected, it is added to the dynamic array
 * Returns array of each release year found in movie list
 * */
struct dynarray* create_years(struct movie* m){

    struct dynarray* years = create_array(); //allocated memory for dynamic array
    struct movie* temp = m;
    dynarray_insert(years, -1, temp->year); //places the first year in the list into the array
    while(temp != NULL){ //traverse movie list
        int has_year = -1;
        for(int i = 0; i < array_length(years); i++){ //for each movie, compare its year to each element in array
            int num = array_get(years, i);
            if(temp->year == num){
                has_year = 1; //if movies year matched any year in array, has_year is set to 1
            }
        }
        if(has_year == -1){ //if movie's year did not match any in array
            dynarray_insert(years, -1, temp->year); //add year to array
        }
        temp = temp->next;
    }   
    return years; //return array of years
}

/*
int year_match(struct movie* m, struct dynarray* y){

    int matched = -1; //-1 if movie's year is not in array yet
    for(int i=0; i < array_length(y); i++){
        if(m->year == array_get(y,i)){
            matched = 1; 
        }
    }
    return matched;
}
*/

/*******
 * function: rate_by_year(struct movie*)
 * 
 * This function accepts a movie list as input
 * creates an array of unique years in movie list
 * loops through each element in year array, traverses movie list to find highest rated movie for that year and prints info
 * */
void rate_by_year(struct movie* lst){

    
    struct movie* temp = lst; //use temp to traverse in create_years(). unsure if necessary, but works.
    struct dynarray* years = create_years(temp);
    /*
    for(int i = 0; i < array_length(years); i++){
        printf("%d\n", array_get(years,i));
    */
    struct movie* traverse = lst; //use traverse to travel down list, used instead of original list for repeatability. unsure if necessary. 

    for(int i=0; i < array_length(years); i++){
        traverse = lst; //reset traverse to beginning of list
        struct movie* highest_rated; 
        
            while( traverse->year != array_get(years,i)){ //find first node in list that matches given year
                traverse = traverse->next;
            }
            highest_rated = traverse; //assign it to highest rate node
            traverse = traverse->next;
            while(traverse != NULL){ //travel down list examining each node trying to find next highest rated movie
                if( (traverse->year == array_get(years,i)) ){
                    if(traverse->rating > highest_rated->rating){
                        //printf("hr: %.2f | trav: %.2f\n", highest_rated->rating, traverse->rating);
                        highest_rated = traverse;
                    }
                }
                traverse = traverse->next;
            }
        
        printf("%d %.2f %s\n", highest_rated->year, highest_rated->rating, highest_rated->title);
    }

    free_array(years);
}


/*******
 * function: select_options(struct movie*)
 * 
 * Creates interface for selecting different options 
 * Calls appropriate function depending on user input
 * Returns if user chooses to exit program
 * */
int select_options(struct movie* lst){
    int option;
    do{
        struct movie* temp = lst;
        printf("\n1. Show movies released in the specified year\n");
        printf("2. Show highest rated movie for each year\n");
        printf("3. Show the title and year of release of all movies in a specific langauage\n");
        printf("4. Exit from the program\n");

        printf("\nEnter a choice from 1 to 4: ");
        
        scanf("%d", &option);
        while(option < 1 || option > 4){
            printf("Please enter an integer from 1 to 4: ");
            scanf("%d", &option);
        }

        if(option == 1){
            specific_year(temp);
        }
        if(option == 2){
            rate_by_year(temp);
        }
        if(option == 3){
            specific_lang(temp);
        }
    }while(option != 4);
return option;
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
        //list = list->next;
    }
}

/*******
 * function: main(int, char*)
 * 
 * Main function. Exits if user did not provide csv file. also exits if user chose option 4 in select_options. 
 * */
int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Please input a csv file\n");
        printf("Example: ./movies filename.csv\n");
        return 1;
    }
    int parsed = 0;
    struct movie* list = processFile(argv[1], &parsed);
    printf("Processed file %s and parsed data for %d movies\n", argv[1], parsed);
    int end = select_options(list);
    //printMovList(list);
    free_list(list);
    return EXIT_SUCCESS;

}