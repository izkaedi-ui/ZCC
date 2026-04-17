#include "zsurface.h"
#include "zprime.h"
#include <stdio.h>

// Fake the global IR pointers needed by ir.c


int main() {
    printf("--- INITIALIZING ZKAEDI PRIME TOPOLOGICAL SURFACE ---\n");
    ir_module_t *mod = ir_module_create();
    ir_func_t *f = ir_func_create(mod, "quantum_maze", IR_TY_I32, 0);
    
    // Inject arithmetic nodes to force the Hamiltonian to evaluate and collapse
    ir_emit(f, IR_CONST, IR_TY_I32, "t1", "", "", "", 42, 1);
    ir_emit(f, IR_CONST, IR_TY_I32, "t2", "", "", "", 77, 2);
    ir_emit(f, IR_ADD, IR_TY_I32, "t3", "t1", "t2", "", 0, 3);
    ir_emit(f, IR_MUL, IR_TY_I32, "t4", "t3", "t3", "", 0, 4);
    ir_emit(f, IR_RET, IR_TY_VOID, "", "t4", "", "", 0, 5);

    FILE *out = stdout; 
    printf("--- COMMENCING SURFACE-CODE PROTECTED CODEGEN ---\n");
    ir_to_zsurface(f, out);
    emit_zprime_selfmod_runtime(out);
    
    printf("\n--- EXECUTION DECOHERED ---\n");
    return 0;
}
