/** If the query file is not included in the terminal,
    the program will as for query names, type each individual
    query cafe name and keep pressing enter. Once all the query
    names are added, press ^D to end. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_CHAR_LEN 128
#define MAX_QUERIES 2
#define MAX_FIELD 14
#define INIT_SIZE 2

typedef struct cafe cafe_t;
// A list node for a particular cafe
struct cafe {
    int cen_year;
    int block_id;
    int prop_id;
    int base_prop_id;
    char *build_add;
    char *clue_small_area;
    char *bus_area;
    char *trad_name;
    int indus_code;
    char *indus_desc;
    char *seat_type;
    int num_of_seats;
    double longitude;
    double latitude;
    cafe_t *next;                   // Points to the next node of the list
};

typedef struct list list_t;
struct list {
	cafe_t *head;                   // Points to the first node of the list
	cafe_t *tail;                   // Points to the last node of the list
	size_t n;                       // Number of elements in the list
};

list_t *getCafeList(FILE *f);
void cafeSkipHeaderLine(FILE *f);
void cafeSkipHeaderLine(FILE *f);
list_t *listCreate();
void cafeRead(FILE *f, list_t *list);
cafe_t *getCafeDict(cafe_t *c, char* fieldArray[]);
void listAppend(list_t *list, cafe_t *c);
int esnureQuerySize(int queryCount, int querySize);
void search(FILE *f, list_t *list, char **queries, int queryCount);
void findMatch(cafe_t *curr, char **matches, int *ptrMatchCount);
int checkMatch(cafe_t *curr, char **matches, int matchCount);
void printOutFile(list_t *list, char **matches, int matchCount);
void freeCafes(list_t *list);
void freeQueries(char** queries, int queryCount);

int main(int argc, char *argv[]){
    // Check to have at least 3 arguments in command line
	// open input and output files
    if (argc < 3) {
		fprintf(stderr, "Usage: %s input_file_name output_file_name\n", argv[0]);
		exit(EXIT_FAILURE);
	}
    // Opens the input file
    FILE *inFile = fopen(argv[2], "r");
    // Opens the output file
    FILE *outFile = fopen(argv[3], "w");
    assert(inFile && outFile);
    // Creates a list of cafes
    list_t *cafes = getCafeList(inFile);

    char query[MAX_CHAR_LEN + 1];
    char **queries;
    int querySize = INIT_SIZE;
    queries = (char**) malloc(sizeof(char *) * querySize);
    int c;
    int queryCount =0;

    // Parses through the query file or stdin until a
    // newline or End of File is reached
    while (scanf("%[^\n]", query) == 1) {
        while ((c = getchar()) != '\n' && c != EOF);
        // Store the particular queries in an array
        queries[queryCount] = strdup(query);
        queryCount++;
        // If the number of elements in the query array reaches
        // the current size, it is going to allocate more memory
        // for the array
        if (esnureQuerySize(queryCount, querySize)){
            querySize = querySize * 2;
            queries = realloc(queries, querySize * sizeof(char *));
        }
    }

    // Searches for the target cafes from the list of cafes that was made
    search(outFile, cafes, queries, queryCount);
    // Frees the list of cafes
    freeCafes(cafes);
    // Frees the queries array
    freeQueries(queries, queryCount);
    // Closes the input file
    fclose(inFile);
    // Closes the output file
    fclose(outFile);

    return 0;
}

// Makes a linked list of the cafes from the dataset file
list_t *getCafeList(FILE *f){
    list_t *cafes = listCreate();
    cafeSkipHeaderLine(f);
    cafeRead(f, cafes);
    return cafes;
}

// The header line of the csv is skipped
// Taken from W2.6
void cafeSkipHeaderLine(FILE *f) {
	while (fgetc(f) != '\n');
}

// Creates & returns an empty linked list
// This function is taken from W3.8
list_t *listCreate(){
    list_t *list = malloc(sizeof(*list));
    assert(list);
	list->head = list->tail = NULL;
	list->n = 0;
	return list;
}

// Reads the input file character by character and stores each field
// in an array as a string
void cafeRead(FILE *f, list_t *list) {
    cafe_t *c;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    // The code below is inspired and modified from chatGPT
    // Read each line of the dataset until End of File is reached
    while ((read = getline(&line, &len, f)) != -1) {
        // The fields are split by a comma
        char *field = strtok(line, ",");
        char *fieldArray[MAX_FIELD];
        int fieldCount = 0;
        // Check if the field starts with a double quote
        while (field != NULL) {
            char extractedValue[MAX_CHAR_LEN + 1];
            // If the field starts with a double quote, remove
            // the double quotation and copy the remaining part
              if (field[0] == '"') {
                strcpy(extractedValue, field + 1);
                field = strtok(NULL, ",");
                while (field != NULL) {
                    // Concatenate the comma and the current field to the extracted value
                    strcat(extractedValue, ",");
                    strcat(extractedValue, field);
                    // The loop breaks when the double quotation is found again
                    // at the end of the field
                    if (field[strlen(field) - 1] == '"') {
                        break;
                    }
                }
                extractedValue[strlen(extractedValue) - 1] = '\0';
            } else {
                // If the field does not end with a quote copy it as it is
                strcpy(extractedValue, field);
            }
            // Allocate memory for the extracted value and copy it to storedValue
            char *storedValue = malloc(strlen(extractedValue) + 1);
            strcpy(storedValue, extractedValue);
            // Stores the stored field in an array as a string
            fieldArray[fieldCount] = storedValue;
            fieldCount++;
            // Move to the next field
            field = strtok(NULL, ",");
        }
        // Fills in the data for the particular cafe
        c = getCafeDict(c, fieldArray);
        // Appends the cafe node to the list of cafes
        listAppend(list, c);
        // Frees the field array
        for (int i = 0; i < fieldCount; i++) {
            free(fieldArray[i]);
        }
    }
    // Frees the line pointer
    free(line);
}

// The function fills in the data for cafe node
cafe_t *getCafeDict(cafe_t *c, char* fieldArray[]){
    // Allocates space for the cafe node
    c = malloc(sizeof(*c));
    // atoi converts the string to integer
    c->cen_year = atoi(fieldArray[0]);
    c->block_id = atoi(fieldArray[1]);
    c->prop_id = atoi(fieldArray[2]);
    c->base_prop_id = atoi(fieldArray[3]);
    c->build_add = strdup(fieldArray[4]);
    c->clue_small_area = strdup(fieldArray[5]);
    c->bus_area = strdup(fieldArray[6]);
    c->trad_name = strdup(fieldArray[7]);
    c->indus_code = atoi(fieldArray[8]);
    c->indus_desc = strdup(fieldArray[9]);
    c->seat_type = strdup(fieldArray[10]);
    c->num_of_seats = atoi(fieldArray[11]);
    // atof converts the string to double
    c->longitude = atof(fieldArray[12]);
    c->latitude = atof(fieldArray[13]);
    c->next = NULL;
    return c;
}

// Adds the give cafe node to the end of the
// linked list
void listAppend(list_t *list, cafe_t *c){
    // If the list is empty, then the particular cafe
    // node is the first element in the list
    if (list->head == NULL){
        list->head = list->tail = c;
    }
    // If the list has one element only, the given
    // cafe becomes the second element of the list
    else if (list->n == 1){
        list->head->next = c;
        c->next = NULL;
        list->tail = c;
    }
    // Append the cafe to the end of the list
    else{
        list->tail->next = c;
        c->next = NULL;
        list->tail = c;
    }
    // increment the size of the linked list
    (list->n)++;
}

// Check if the number of elements in the query
// is equal to its size
int esnureQuerySize(int queryCount, int querySize){
    if (queryCount == querySize){
        return 1;
    }
    return 0;
}

// Search funtion to search for the desired cafes and prints them out to
// stdout and makes a output file
void search(FILE *f, list_t *list, char **queries, int queryCount) {
    // Loops through the list of target cafes and searches for
    // them in the linked list and keeps track of number of times
    // the loop had encountered the same cafe name in the linked list
    for (int i = 0; i < queryCount; i++) {
        int tallyCount =0;
        fprintf(f, "%s\n", queries[i]);
        cafe_t *curr = list->head;
        // Iterates through the linked list
        while (curr) {
            if (!strcmp(queries[i], curr->trad_name)) {
                fprintf(f, "--> census_year: %d || block_id: %d || property_id: %d"
                 "|| base_property_id: %d || building_address: %s || clue_small_area: %s ||"
                 " business_address: %s || trading_name: %s || industry_code: %d ||"
                 " industry_description: %s || seating_type: %s || number_of_seats: %d ||"
                 " longitude: %lf || latitude: %lf ||\n", curr->cen_year, curr->block_id,\
                  curr->prop_id, curr->base_prop_id, curr->build_add, curr->clue_small_area,\
                  curr->bus_area, curr->trad_name, curr->indus_code, curr->indus_desc,\
                  curr->seat_type, curr->num_of_seats, curr->longitude, curr->latitude);

                tallyCount++;
            }
            curr = curr->next;
        }
        // Prints out to stdout
        // If tallyCount is 0, that means the cafe
        // was not found in the list
        if (tallyCount == 0){
            printf("%s --> NOT FOUND\n", queries[i]);
        }
        // Else the total number of cafe found in the list is printed
        else{
            printf("%s --> %d\n", queries[i], tallyCount);
        }
    }
}

// Frees the the linked list
void freeCafes(list_t *list) {
    cafe_t *curr = list->head;
    // Iterates through the linked list and frees the field
    while (curr) {
        cafe_t *temp = curr;
        curr = curr->next;
        free(temp->build_add);
        free(temp->clue_small_area);
        free(temp->bus_area);
        free(temp->trad_name);
        free(temp->indus_desc);
        free(temp->seat_type);
        free(temp);
    }
    // Frees the complete list
    free(list);
}

// Frees the array of queries
void freeQueries(char** queries, int queryCount){
    for (int i=0; i<queryCount; i++){
        free(queries[i]);
    }
    free(queries);
}