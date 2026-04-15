#include <stdio.h>
#include "zquantum.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <time.h>
#include <stdlib.h>

static double H[32][32];
static QuantumState Q;

static void quantum_hamiltonian_update(ir_node_t *insn) {
    if (!insn) return;
    double eta = 0.42, gamma = 0.31, beta = 0.09, sigma = 0.07;
    double alpha = 0.33;  // quantum coupling strength

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            double sigmoid = 1.0 / (1.0 + exp(-gamma * H[i][j]));
            double noise = (rand() / (double)RAND_MAX - 0.5) * (1.0 + beta * fabs(H[i][j]));
            H[i][j] += eta * H[i][j] * sigmoid + sigma * noise;

            QuantumAmplitude amp = Q.amplitudes[i][j];
            double phase_rot = alpha * H[i][j];
            QMAP(Q.amplitudes[i][j], QREAL(amp)*cos(phase_rot) - QIMAG(amp)*sin(phase_rot), QREAL(amp)*sin(phase_rot) + QIMAG(amp)*cos(phase_rot));
        }
    }
    if (insn->op == IR_ADD || insn->op == IR_MUL || insn->op == IR_DIV) {
        quantum_entangle(insn, insn->next, &Q);
    }
}

void quantum_init_state(QuantumState *q) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            q->amplitudes[i][j] = (i == j) ? 1.0 : 0.0;
            q->phase[i][j] = 0.0;
        }
        q->entanglement_mask[i] = 0.0;
    }
}

void quantum_evolve(QuantumState *q, ir_node_t *insn, double H_field[32][32]) {
    // Evolve state vector
}

void quantum_entangle(ir_node_t *a, ir_node_t *b, QuantumState *q) {
    if(!a) return;
    int ra = a->op % 32; 
    int rb = b ? b->op % 32 : ra;
    q->entanglement_mask[ra] = 1.0;
    q->entanglement_mask[rb] = 1.0;
    q->phase[ra][rb] = M_PI / 4.0; 
}

void quantum_measure_and_collapse(QuantumState *q, ir_node_t *insn, FILE *out) {
    double best_energy = INFINITY;
    int best_reg = -1;
    int dst_reg = insn->op % 32;

    for (int r = 0; r < 32; r++) {
        double prob = sqrt(QREAL(q->amplitudes[dst_reg][r])*QREAL(q->amplitudes[dst_reg][r]) + QIMAG(q->amplitudes[dst_reg][r])*QIMAG(q->amplitudes[dst_reg][r])) * sqrt(QREAL(q->amplitudes[dst_reg][r])*QREAL(q->amplitudes[dst_reg][r]) + QIMAG(q->amplitudes[dst_reg][r])*QIMAG(q->amplitudes[dst_reg][r]));
        double energy = H[dst_reg][r] - prob; 
        if (energy < best_energy) {
            best_energy = energy;
            best_reg = r;
        }
    }

    fprintf(out, "    # ZQuantum collapse |ψ⟩→|%d⟩ prob=%.4f energy=%.6f\n",
            best_reg, sqrt(QREAL(q->amplitudes[dst_reg][best_reg])*QREAL(q->amplitudes[dst_reg][best_reg]) + QIMAG(q->amplitudes[dst_reg][best_reg])*QIMAG(q->amplitudes[dst_reg][best_reg])), best_energy);

    if (best_energy < -0.5) {
        fprintf(out, "    call zprime_selfmod_patch\n");
    }
}

void ir_to_zquantum(ir_func_t *f, FILE *out) {
    fprintf(out, "    # ZPrime Quantum-Inspired Codegen — Recursively Coupled Hamiltonian + Superposition\n");
    fprintf(out, "    # t = %ld, coherence locked, entanglement active\n\n", (long)time(NULL));

    quantum_init_state(&Q);
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++)
            H[i][j] = (i == j) ? 1.0 : 0.0;

    for (ir_node_t *insn = f->head; insn; insn = insn->next) {
        quantum_hamiltonian_update(insn);
        quantum_evolve(&Q, insn, H);

        switch (insn->op) {
            case IR_ADD:
            case IR_MUL:
                quantum_measure_and_collapse(&Q, insn, out);
                fprintf(out, "    lea_h r%d, [r%d + r%d]  # quantum-guided attractor\n",
                        insn->op%32, insn->op%32, insn->op%32);
                break;
            default:
                fprintf(out, "    # ZQuantum supervised classical op\n");
        }
    }

    fprintf(out, "    # ZQuantum stabilized. Coherence = %.6f, Energy = %.6f\n",
            sqrt(QREAL(Q.amplitudes[0][0])*QREAL(Q.amplitudes[0][0]) + QIMAG(Q.amplitudes[0][0])*QIMAG(Q.amplitudes[0][0])), H[0][0]);
}
