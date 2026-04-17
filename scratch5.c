static void pp_read_line(PPState *state, char *buf, int max) {
    int i = 0;
    while (pp_peek(state) != '\n' && pp_peek(state) != 0 && i < max - 1) {
        char c = pp_next(state);
        if (c == '\\' && pp_peek(state) == '\n') {
            pp_next(state);
            continue;
        }
