#ifndef IMAGE_H
#define IMAGE_H

#include "queue.h"

typedef struct Pixel Pixel;
struct Pixel {
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

/* SDL */
void DisplayImage(SDL_Surface *image);
Uint32 GetPixel(SDL_Surface *surface, int x, int y);
void PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

/* MATRIX TREATMENT */
void LoadImage(SDL_Surface *image);


/* MATRIX CONVERSION */
void SurfaceToMatrix(Pixel **pixels, SDL_Surface *image, int h, int w);
SDL_Surface *MatrixToSurface(Pixel **pixels, int h, int w);
void BinToPixels(int **matrix, Pixel **pixels, int h, int w);

/* BINARIZATION */
void GreyScale(Pixel **pixels, int h, int w);
int Otsu(Pixel **pixels, int h, int w);
void Binarization(Pixel **pixels, int h, int w, int threshold);
void BinarizeMatrix(Pixel **pixels, int **binarized, int h, int w);
void DebinarizeMatrix(int **binarized, Pixel **pixels, int h, int w);

/*SEGMENTATION */
void Segmentation(int **matrix, int h, int w);
void CutInLine(int **matrix, int *histogram, Queue *queue, int h, int w);
void CutInChar(int **matrix, int *histogram, Queue *queue, int h1, int h2, int w);
void ShowSegmentation(Queue *queue);

void EnqueueMatrix(int **matrix, Queue *queue, int h1, int h2, int w1, int w2);

#endif
