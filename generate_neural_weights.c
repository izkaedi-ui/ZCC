#include "tqpu_neural.h"
#include <stdio.h>

int main() {
    NeuralPathwayMap nm;
    neural_init(&nm);
    for(int i=0; i<300; i++) {
        neural_update(&nm, 0.5, 0.8, 0.9, 0.98);
        neural_update(&nm, 0.3, 0.2, 0.4, 0.65);
    }
    neural_save(".zcc_neural_weights");
    printf("Epochs Trained: %d\n", nm.epochs_trained);
    printf("Logical BMU Depth (Weight Bias Offset): %.4f\n", nm.weights[4][4][0]);
    return 0;
}
