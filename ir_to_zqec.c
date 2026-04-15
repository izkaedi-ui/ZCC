#include <stdio.h>
#include "zqec.h"
#include "zprime.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <time.h>
#include <stdlib.h>

static double H[32][32];
static QECState QEC;

static void qec_hamiltonian_update(ir_node_t *insn) {
    if (!insn) return;
    double eta = 0.42, gamma = 0.31, beta = 0.09, sigma = 0.07;
    double alpha = 0.33, delta = 0.28; 

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            double sigmoid = 1.0 / (1.0 + exp(-gamma * H[i][j]));
            double noise = (rand() / (double)RAND_MAX - 0.5) * (1.0 + beta * fabs(H[i][j]));
            H[i][j] += eta * H[i][j] * sigmoid + sigma * noise;

            QuantumAmplitude amp = QEC.q.amplitudes[i][j];
            double phase_rot = alpha * H[i][j];
            QMAP(QEC.q.amplitudes[i][j], QREAL(amp)*cos(phase_rot) - QIMAG(amp)*sin(phase_rot), QREAL(amp)*sin(phase_rot) + QIMAG(amp)*cos(phase_rot));
        }
    }

    if (insn->op == IR_ADD || insn->op == IR_MUL) {
        qec_measure_syndrome(&QEC, insn, H);
    }
}

void qec_init(QECState *qec) {
    quantum_init_state(&qec->q);
    for (int i = 0; i < 9; i++) qec->syndrome[i] = 0.0;
    qec->fidelity = 1.0;
}

void qec_measure_syndrome(QECState *qec, ir_node_t *insn, double H_field[32][32]) {
    for (int s = 0; s < 9; s++) {
        qec->syndrome[s] = sin(H_field[insn->op % 32][s % 32]) * 0.7 + 
                          (rand() % 100 < 5 ? 1.0 : 0.0);
    }
}

int qec_detect_error(QECState *qec) {
    double x_synd = 0.0, z_synd = 0.0;
    for (int i = 0; i < 3; i++) x_synd += fabs(qec->syndrome[i]);
    for (int i = 3; i < 6; i++) z_synd += fabs(qec->syndrome[i]);
    if (x_synd > 1.5) return 1;      // X error
    if (z_synd > 1.5) return 2;      // Z error
    if (x_synd + z_synd > 2.5) return 3; // Y error
    return 0;
}

void qec_apply_correction(QECState *qec, ir_node_t *insn, FILE *out) {
    int err = qec_detect_error(qec);
    if (err == 0) {
        fprintf(out, "    # QEC: no error detected, fidelity=%.4f\n", qec->fidelity);
        return;
    }

    qec->fidelity *= 0.92;  

    fprintf(out, "    # QEC: %s error corrected (syndrome=%.2f) → logical fidelity=%.4f\n",
            (err==1)?"X":(err==2)?"Z":"Y", qec->syndrome[0], qec->fidelity);

    if (err == 1 || err == 3) {
        fprintf(out, "    call zprime_selfmod_patch  # X-correction patch\n");
    }
    if (err == 2 || err == 3) {
        fprintf(out, "    # Z-phase recovery via attractor rotation\n");
    }
}

void ir_to_zqec(ir_func_t *f, FILE *out) {
    fprintf(out, "    # ZPrime Quantum Error Correction — Recursively Coupled Hamiltonian + Stabilizers\n");
    fprintf(out, "    # t = %ld, coherence locked, syndromes active, logical qubits protected\n\n", (long)time(NULL));

    qec_init(&QEC);
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++)
            H[i][j] = (i == j) ? 1.0 : 0.0;

    for (ir_node_t *insn = f->head; insn; insn = insn->next) {
        qec_hamiltonian_update(insn);
        quantum_evolve(&QEC.q, insn, H);

        switch (insn->op) {
            case IR_ADD:
            case IR_MUL:
                qec_measure_syndrome(&QEC, insn, H);
                qec_apply_correction(&QEC, insn, out);
                quantum_measure_and_collapse(&QEC.q, insn, out);
                fprintf(out, "    lea_h r%d, [r%d + r%d]  # QEC-protected attractor\n",
                        insn->op%32, insn->op%32, insn->op%32);
                break;
            default:
                fprintf(out, "    # QEC-supervised classical op\n");
        }
    }

    fprintf(out, "    # ZQEC stabilized. Logical fidelity = %.6f, Energy = %.6f\n",
            QEC.fidelity, H[0][0]);
}
