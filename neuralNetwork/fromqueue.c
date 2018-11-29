#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "../imageTreatment/segmentation.h"
#include "../imageTreatment/image.h"
#include "../imageTreatment/matrix.h"
#include "../imageTreatment/queue.h"
#include "neuralNet.h"
#include "DataSet.h"
#include "neuralNet.h"

#define SIZE   20
#define OFFSET 33
void printArr(float* arr) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (arr[SIZE * i + j] == 1.0f) 
                printf("1");
            else
                printf(" ");
            // printf("%1.f", arr[SIZE * i + j]);
        }
        printf("\n");
    }
}

unsigned char getMaxPos(float* arr, char len) {
    // We can limit to char because NN output layer is composed of 94 neurons
    unsigned char max = 0;
    unsigned char pos = -1;

    while(++pos < len)
        if (arr[pos] > arr[max])
            max = pos;

    return max;
}

void NNwriteValue(int **matrix, char value) {
    float inp[SIZE * SIZE];

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            inp[i * SIZE + j] = matrix[i][j];
        }
    }

    printArr(inp);

//    printf("Witch char it is ? (space to not chose)");
//    char value = 10;
//    while (value == 10)
//        scanf("%c", &value);
    printf(" You selected : %c (%d)\n", value, value);
    
    printf(" Confirm ? [Y]/n");
    
    char confirm;
    // scanf(" %c", &confirm);
    confirm = getchar();
    if (confirm != '\n') {
        printf("Nope for this one\n");
        return;
    } else {
        printf("YEAR !!!\n");
    }

    FILE *f;
    f = fopen("ocr_to_train.data", "a+");
    if (f == NULL)
        errx(1, "Something went wrong when oppenning ocr_to_train.data");
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fprintf(f, "%1.4f ", inp[i * SIZE + j]);
        }
    }

    for (int i = OFFSET; i < 126; i++)
        fprintf(f, "%d ", i == value);
    fprintf(f, "%d\n", 127 == value);

    fclose(f);
}


unsigned char NNfindChar(neuNet n, int **matrix) {
    float inp[SIZE * SIZE];

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            inp[i * SIZE + j] = matrix[i][j];
        }
    }

   // printf("\t\tmatrix converted to float list\n");

    float* out = NNGuess(n, inp);
    
    unsigned char result = getMaxPos(out, n.nbOutput) + OFFSET;

    printf("Predicted : %c (%d)\n", result, result);

    return result;
}

char* extractstring(char* filepath, Queue *q) {
    printf("extractstring [\n");
    char   *s;
    Elt    *curr    = NULL;
    size_t t        = 0;
    size_t capacity = 100; 

    s = malloc(sizeof(char) * capacity);
    if (s)
        printf("\ts malloced perfectly\n");
    else
        errx(1, "malloc issue");

    if (q->first)
        curr = q->first;

    printf("Pleaze, insert real string (just the letters):\n");
    char inp[10000] = "";
    
    scanf("%s", inp);
    printf("You just chosed : %s\n", inp);
    
    int posRealString = 0;

    // Loading neuralNetwork
    neuNet n = NNload(filepath);

    while (curr && curr->data) {
        int **c = curr->data->data;

        if (curr->data->width > 1 && curr->data->height > 1){
            s[t] = 'c';
            t++;

            int h = curr->data->height;
            int w = curr->data->width;
            
            if (h != SIZE || w != SIZE)
                errx(1, "Issue in matrix size");

            s[t - 1] = NNfindChar(n, c);
            //NNwriteValue(c, inp[posRealString++]);
            
        } else if (c && c[0][0] == 38) {
            s[t] = '\n';
            t++;
        } else if (c && c[0][0] == 32) {
            s[t] = ' ';
            t++;
        } else  {
            s[t] = '\n';
            t++;
        }

        curr = curr->next;

        if (t == capacity) {
            capacity *= 2;
            s = realloc(s, capacity * sizeof(char));
            if (!s)
                errx(1, "Not enough memory");
            printf("\tstring upgraded to : %lu\n", capacity);
        }
    }
    
    s[t] = 0;
    
    NNfree(n);
    printf("]\n");

    return s;
}
