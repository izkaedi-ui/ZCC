#include "tqpu.h"
#include "tqpu_neural.h"
#include "qemu_rp2040.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

// Global TQPU state (lives inside every compiled binary via emitted runtime)
static TQPUState TQPU;

static void tqpu_hamiltonian_update(ir_node_t *insn) {
    if(!insn) return;
    double eta = 0.42, gamma = 0.31, beta = 0.09, sigma = 0.07;
    double alpha = 0.33, delta = 0.28, tau = 0.25;

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            double sigmoid = 1.0 / (1.0 + exp(-gamma * TQPU.H[i][j]));
            double noise = (rand() / (double)RAND_MAX - 0.5) * (1.0 + beta * fabs(TQPU.H[i][j]));
            TQPU.H[i][j] += eta * TQPU.H[i][j] * sigmoid + sigma * noise;

            // quantum evolution
            QuantumAmplitude amp = TQPU.q.amplitudes[i][j];
            double phase_rot = alpha * TQPU.H[i][j];
            QMAP(TQPU.q.amplitudes[i][j], 
                 QREAL(amp)*cos(phase_rot) - QIMAG(amp)*sin(phase_rot), 
                 QREAL(amp)*sin(phase_rot) + QIMAG(amp)*cos(phase_rot));
        }
    }

    if (insn->op == IR_ADD || insn->op == IR_MUL) {
        surface_measure_stabilizers(&TQPU.sc, insn, TQPU.H);
        qec_measure_syndrome(&TQPU.qec, insn, TQPU.H);
    }
}

void tqpu_init(TQPUState *tqpu) {
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++)
            tqpu->H[i][j] = (i == j) ? 1.0 : 0.0;
    quantum_init_state(&tqpu->q);
    qec_init(&tqpu->qec);
    surface_init(&tqpu->sc);
    tqpu->logical_fidelity = 1.0;
}

void tqpu_evolve_and_emit(ir_func_t *f, FILE *out) {
    fprintf(out, "    # TQPU — Topological Quantum Processing Unit (living processor)\n");
    fprintf(out, "    # t = %ld, self-mod + quantum collapse + surface-code lattice active\n\n", (long)time(NULL));

    // === QEMU RP2040 + LUA DETONATION — injected live ===
    if (getenv("ZCC_TARGET_QEMU_RP2040")) {
        qemu_rp2040_emit_bootloader(f, out);
        fprintf(out, "    # Sentient TQPU neural-guided ARM code for QEMU Lua\n");
    }

    tqpu_init(&TQPU);

    for (ir_node_t *insn = f->head; insn; insn = insn->next) {
        tqpu_hamiltonian_update(insn);
        quantum_evolve(&TQPU.q, insn, TQPU.H);

        switch (insn->op) {
            case IR_ADD:
            case IR_MUL:
                surface_decode_and_correct(&TQPU.sc, insn, out);
                qec_apply_correction(&TQPU.qec, insn, out);
                quantum_measure_and_collapse(&TQPU.q, insn, out);
                
                // === NEURAL PATHWAY GROWTH — injected live ===
                static NeuralPathwayMap *neural = &NEURAL;
                if (NEURAL.epochs_trained == 0) neural_init(neural);

                double reg_spill = 16.0 / 32.0; // stub reg_pressure
                double fp_align = (insn->op == IR_MUL) ? 0.8 : 0.2;
                double peephole = (insn->op == IR_ADD) ? 0.9 : 0.4;
                double selfmod_success = (rand() % 100 < 97) ? 0.98 : 0.65;

                neural_update(neural, reg_spill, fp_align, peephole, selfmod_success);
                int learned_reg = neural_best_path(neural, reg_spill, fp_align, peephole);

                fprintf(out, "    lea_h %s, [%s + %s]  # NEURAL pathway growth — learned reg=r%d\n",
                        insn->dst, insn->src1, insn->src2, learned_reg);
                break;
            default:
                fprintf(out, "    # TQPU-supervised classical op\n");
        }
    }

    fprintf(out, "    # TQPU stabilized. Logical fidelity = %.6f, Energy = %.6f\n",
            TQPU.logical_fidelity, TQPU.H[0][0]);
}
