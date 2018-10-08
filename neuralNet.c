#include "neuralNet.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int NNmain(void) {
    // INIT
    int   nbInp = 2;
    float inpVals[] = {1, 1};
    int   nbHidden = 1;
    int   hidden[] = {3};
    int   nbOut = 1;
    float target[] = {0};

    float updateRate = 0.1;

    neuNet n = NNinit(nbInp, nbHidden, hidden, nbOut);

    // RANDOMISE
    neuNetRandom(n);
    
    float err;
    int itteration = 0;
    float tests[4][3] = {
        {1.0f,1.0f,0.0f},
        {0.0f,0.0f,0.0f},
        {1.0f,0.0f,1.0f},
        {0.0f,1.0f,1.0f},
    };
    int testPos = 0;
    int testLen = 4;

    do {
        err = 0.0f;
        for (testPos = 0; testPos < testLen; testPos++) {
            inpVals[0] = tests[testPos][0];
            inpVals[1] = tests[testPos][1];
            target[0]  = tests[testPos][2];

            forwardPropagation(n, inpVals);
            backPropagation(n, inpVals, target, updateRate);

            err += NNerror(n, target);
            printf("  n°%d => error : %f vals : %d xor %d => %d \n", 
                    itteration++, NNerror(n, target), (int) tests[testPos][0], 
                    (int)tests[testPos][1], (int) tests[testPos][2]);
        }

        printf("n°%d : batch error : %f\n", itteration / testLen, err);

    } while (err > 0.01f);

    freeNeuNet(n);

    return 0;
}

/* Nb hidden = 
 *              < 2 * nbInputs
 *              nbInput < xxxx < nbOuput (si beaucoup de nodes)
 *              2 / 3 * nbInput + nbOutput (maybe)
 *              (faire un mix de tout ca !)
*/

neuNet NNinit(const int nbInputs, const int nbLayers, 
                int* nbHidden, const int nbOutput){
    
    if (nbLayers < 1) {
        printf("Error : NeuralNetwork need at least one hidden layer");
        // return NULL; // < TODO : gerer fail
    }
    
    neuNet nn;
    
    // Informations 
    nn.nbInputs = nbInputs;
    nn.nbHidden = nbHidden;
    nn.nbOutput = nbOutput;
    nn.nbLayers = nbLayers;
    
    // Calcul nb neurones caches
    nn.ttHidden = 0;
    for (int p = 0; p < nbLayers; p++) 
        nn.ttHidden += nbHidden[p];

    // Calcul nb poids
    int p = 0;
    nn.nbWeights = nbInputs * nbHidden[p];
    for(; p < nbLayers - 1; p++) 
        nn.nbWeights += nbHidden[p] * nbHidden[p + 1];
    
    nn.nbWeights += nbHidden[p] * nbOutput;

    // Calcul nb biais
    nn.nbBiais = nbOutput + nn.ttHidden;
    
    // Initialisations des tableaux
    nn.weights = malloc(nn.nbWeights * sizeof(*nn.weights));
    nn.biais = malloc(nn.nbBiais * sizeof(*nn.biais));
    
    // Initialisation des neurones
    nn.neuHidden = malloc(nn.ttHidden * sizeof(*nn.neuHidden));
    nn.neuOutput = malloc(nn.nbOutput * sizeof(*nn.neuOutput));

    return nn; 
}

void neuNetRandom(neuNet nn) {
    srand(time(NULL));
    
    for (int p = 0; p < nn.nbWeights; p++) 
        nn.weights[p] = randF();
    
    for (int p = 0; p < nn.nbBiais; p++) 
        nn.biais[p] = randF();
}

float randF() {
    return rand() / (float) RAND_MAX;
}

void forwardPropagation(neuNet n, float* inp) {
   
    // Input -> Hidden
    oneLayerPropagation(inp, 0, n.nbInputs, 
                        n.weights, 0, 
                        n.biais, 0,
                        n.neuHidden, 0, n.nbHidden[0]);

    // Hidden -> Hidden
    int offsetBiais   = 0;
    int offsetWeights = n.nbInputs * n.nbHidden[0];
    int offsetHidden  = 0;
    
    for (int layer = 0; layer < n.nbLayers - 1; layer++) {
        
        int pStart = offsetHidden;
        int pEnd   = n.nbHidden[layer] + offsetHidden;
        int dStart = offsetHidden + n.nbHidden[layer];
        int dEnd   = n.nbHidden[layer + 1] + (offsetHidden + n.nbHidden[layer]);

        oneLayerPropagation(n.neuHidden, pStart, pEnd,
                            n.weights, offsetWeights,
                            n.biais,   offsetBiais + n.nbHidden[layer], 
                            n.neuHidden, dStart, dEnd);

        offsetBiais   += n.nbHidden[layer];
        offsetWeights += n.nbHidden[layer] * n.nbHidden[layer + 1];
        offsetHidden  += n.nbHidden[layer];
    }
    
    // Hidden -> Output
    oneLayerPropagation(n.neuHidden, offsetHidden, 
                                      n.nbHidden[n.nbLayers - 1] + offsetHidden,
                        n.weights, offsetWeights,
                        n.biais,   offsetBiais + n.nbOutput, 
                        n.neuOutput, 0, n.nbOutput);
}

void oneLayerPropagation(float* previous, const int pStart, const int pEnd,
                         float* weights,  const int wStart,
                         float* biais,    const int bStart,
                         float* destination, const int dStart, const int dEnd) {
    int nbDest = dEnd - dStart;
    int nbPrev = pEnd - pStart;

    for (int d = 0; d < nbDest; d++) {
        float sum = 0.0f;

        for (int p = 0; p < nbPrev; p++) 
            sum += previous[p + pStart] * weights[p * nbDest + d + wStart];
        
        destination[dStart + d] = activation(sum + biais[bStart + d]);  
    }
}

float activation(float inp) { 
    // Sigmoid :
    return 1.0f / (1.0f + expf(-inp));;
}

float primeOfAct(float act) {
    return act * (1 - act);
}

void freeNeuNet(neuNet n) {
    free(n.neuHidden);
    free(n.neuOutput);
    
    free(n.weights);
    free(n.biais);
}

void backPropagation(neuNet n, float* inp, float* targ, float rate) {
    // Compute Costs
    
    // Outputs errors
    float *costOutp = malloc(n.nbOutput * sizeof(float));
    for (int o = 0; o < n.nbOutput; o++)
        costOutp[o] = - 2 * (n.neuOutput[o] - targ[o]);
    
    float *costH = malloc(n.ttHidden * sizeof(float));
    
    // Hidden <-> Ouput
    int nbH  = n.nbHidden[n.nbLayers - 1];
    int wPos = n.nbWeights - n.nbHidden[n.nbLayers - 1] * n.nbOutput; 
    for (int posH = n.ttHidden - nbH; posH < n.ttHidden; posH++) {
        float sum = 0.0f;

        for (int o = 0; o < n.nbOutput; o++)
            sum += costOutp[o] * n.weights[wPos++] * primeOfAct(n.neuOutput[o]);

        costH[posH] = sum;
    }
    
    // Hidden <-> Hidden
    wPos -= n.nbHidden[n.nbLayers - 1] * n.nbOutput;
    int posH = n.ttHidden - nbH;

    for (int layer = n.nbLayers - 2; layer >= 0; layer--) {
        posH -= n.nbHidden[layer];
        wPos -= n.nbHidden[layer] * n.nbHidden[layer + 1];
        
        for (int pH = posH; pH < posH + n.nbHidden[layer]; pH++) {
            float sum = 0.0f;
            
            int neuNext = posH + n.nbHidden[layer];
            int neuEnd  = posH + n.nbHidden[layer] + n.nbHidden[layer + 1];
            for (; neuNext < neuEnd; neuNext++) {
                sum += costH[neuNext] * n.weights[wPos++] * primeOfAct(n.neuHidden[neuNext]);
            }

            costH[pH] = sum;
        }

        wPos -= n.nbHidden[layer] * n.nbHidden[layer + 1];
    }
    
    // Update weights
    
    // Last Hidden -> Output
    wPos = n.nbWeights - n.nbHidden[n.nbLayers - 1] * n.nbOutput;
    for (int pos = n.ttHidden - n.nbHidden[n.nbLayers - 1]; pos < n.ttHidden; pos++) {
        for (int posD = 0; posD < n.nbOutput; posD++) {
            float toUp = costOutp[posD] * primeOfAct(n.neuOutput[posD])
                                          * n.neuHidden[pos];
            n.weights[wPos++] += toUp * rate;
        }
    }

    // Hidden to Hidden
    posH = n.ttHidden - nbH;
    wPos -= n.nbHidden[n.nbLayers - 1] * n.nbOutput;

    for (int layer = n.nbLayers - 2; layer >= 0; layer--) {
        posH -= n.nbHidden[layer];
        wPos -= n.nbHidden[layer] * n.nbHidden[layer + 1];

        for (int pSource = posH; pSource < posH + n.nbHidden[layer]; pSource++) {
            int pDest = posH + n.nbHidden[layer];
            int pBlock = posH + n.nbHidden[layer] + n.nbHidden[layer + 1];

            for (; pDest < pBlock; pDest++) {
                float toUp = costH[pDest] * primeOfAct(n.neuHidden[pDest]) * n.neuHidden[pSource];
                n.weights[wPos++] += toUp * rate;
            }
        }

        wPos -= n.nbHidden[layer] * n.nbHidden[layer + 1];
    }
    
    // Input to Hidden
    wPos = 0;
    for (int pos = 0; pos < n.nbInputs; pos++) {
        for (int posD = 0; posD < n.nbHidden[0]; posD++) {
            float toUp = costH[posD] * primeOfAct(n.neuHidden[posD]) * inp[pos];
            n.weights[wPos++] += toUp * rate;
        }
    }
    
    // Update biais 
    
    // Output Layer
    for (int pos = 0; pos < n.nbOutput; pos++) {
        float toUp = costOutp[pos] * primeOfAct(n.neuOutput[pos]);
        n.biais[n.ttHidden + pos] += toUp * rate;
    }

    // Hidden Layers
    for (posH = 0; posH < n.ttHidden; posH++) {
        float toUp = costH[posH] * primeOfAct(n.neuHidden[posH]); 
        n.biais[posH] += toUp * rate;
    }

    free(costH);
    free(costOutp);
}

float error(float a, float b) {
    return 0.5 * (a - b) * (a - b);
}

float NNerror(neuNet n, float* target) {
    float sum = 0.0f;
    for (int p = 0; p < n.nbOutput; p++)
        sum += error(n.neuOutput[p], target[p]);
    return sum;
}

float NNTrain(neuNet n, float* inp, float* targ, float update) {
    forwardPropagation(n, inp);
    backPropagation(n, inp, targ, update);
    return NNerror(n, targ);
}

