Type *parse_declarator(Compiler *cc, Type *base, char *name_out);
static long long parse_const_expr_ternary(Compiler *cc);
static long long parse_const_expr_lor(Compiler *cc);
static long long parse_const_expr_land(Compiler *cc);
static long long parse_const_expr_bor(Compiler *cc);
static long long parse_const_expr_bxor(Compiler *cc);
static long long parse_const_expr_band(Compiler *cc);
static long long parse_const_expr_eq(Compiler *cc);
static long long parse_const_expr_rel(Compiler *cc);
static long long parse_const_expr_shift(Compiler *cc);
static long long parse_const_expr_add(Compiler *cc);
static long long parse_const_expr_mul(Compiler *cc);
static long long parse_const_expr_unary(Compiler *cc);
static long long parse_const_expr_primary(Compiler *cc);

static long long parse_const_expr(Compiler *cc) { return parse_const_expr_ternary(cc); }
            st = parse_declarator(cc, st, dummy);
            expect(cc, TK_RPAREN);
            return parse_const_expr_unary(cc);
        }
                st = parse_declarator(cc, st, dummy);
                expect(cc, TK_RPAREN);
                return type_size(st);
            }
            ftype = parse_declarator(cc, base_ftype, fname);

            /* Ignore bitfield size since ZCC allocates full integers for them */
            if (cc->tk == TK_COLON) {
                next_token(cc);
                parse_const_expr(cc);
            }
                ftype2 = parse_declarator(cc, base_ftype, fname2);

                /* Ignore bitfield size since ZCC allocates full integers */
                if (cc->tk == TK_COLON) {
                    next_token(cc);
                    parse_const_expr(cc);
                }
Type *parse_declarator(Compiler *cc, Type *base, char *name_out) {
    Type *type;

    type = base;
    name_out[0] = 0;

    /* pointers */
    while (cc->tk == TK_STAR) {
        next_token(cc);
        /* skip const/volatile after * */
        while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) {
            next_token(cc);
        }
            inner = parse_declarator(cc, cc->ty_int, name_out);
            expect(cc, TK_RPAREN);
            /* process outer dimensions and function args first! */
            if (cc->tk == TK_LBRACKET) {
                int arr_lens[16];
                int arr_num = 0;
                while (cc->tk == TK_LBRACKET) {
                    int len = 0;
                    next_token(cc);
                    if (cc->tk != TK_RBRACKET) len = (int)parse_const_expr(cc);
                    expect(cc, TK_RBRACKET);
                    if (arr_num < 16) arr_lens[arr_num++] = len;
                }
                            ptype = parse_declarator(cc, ptype, pname);
                            if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);
                            if (ftype->num_params < MAX_PARAMS) {
                                ftype->params[ftype->num_params] = ptype;
                                ftype->num_params++;
                            }
                    ptype = parse_declarator(cc, ptype, pname);

                    if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);

                    if (ftype->num_params < MAX_PARAMS) {
                        ftype->params[ftype->num_params] = ptype;
                        ftype->num_params++;
                    }
            cast_type = parse_declarator(cc, cast_type, dummy);
            expect(cc, TK_RPAREN);
            n = node_new(cc, ND_CAST, line);
            n->cast_type = cast_type;
            n->lhs = parse_unary(cc);
            n->type = cast_type;
            return n;
        }
        ret_type = parse_declarator(cc, ret_type, dummy);
        expect(cc, TK_RPAREN);

        n = node_new(cc, ND_VA_ARG, line);
        n->lhs = ap_node;
        n->type = ret_type;
        return n;
    }
                st = parse_declarator(cc, st, dummy);
                expect(cc, TK_RPAREN);
                n = node_num(cc, type_size(st), line);
                return n;
            }
            t = parse_declarator(cc, t, dummy);
            expect(cc, TK_RPAREN);
        } else {
            cast_type = parse_declarator(cc, cast_type, dummy);
            expect(cc, TK_RPAREN);
            if (cc->tk == TK_LBRACE) {
                Symbol *sym;
                char tmp_name[32];
                sprintf(tmp_name, ".L_comp_%d", cc->label_count++);
                sym = scope_add_local(cc, tmp_name, cast_type);

                Node *var_n_final = node_new(cc, ND_VAR, line);
                strncpy(var_n_final->name, tmp_name, MAX_IDENT - 1);
                var_n_final->sym = sym;
                var_n_final->type = cast_type;

                Node *expr = 0;
                next_token(cc); /* skip { */

                if (cast_type->kind == TY_STRUCT) {
                    StructField *sf = cast_type->fields;
                    while (cc->tk != TK_RBRACE && cc->tk != TK_EOF && sf) {
                        Node *var_n = node_new(cc, ND_VAR, line);
                        strncpy(var_n->name, tmp_name, MAX_IDENT - 1);
                        var_n->sym = sym;
                        var_n->type = cast_type;

                        Node *mem_n = node_new(cc, ND_MEMBER, line);
                        mem_n->lhs = var_n;
                        strncpy(mem_n->member_name, sf->name, MAX_IDENT - 1);
                        mem_n->member_offset = sf->offset;
                        mem_n->type = sf->type;
                        mem_n->member_size = type_size(sf->type);

                        Node *asgn_v = parse_assign(cc);
                        Node *asgn_n = node_new(cc, ND_ASSIGN, line);
                        asgn_n->lhs = mem_n;
                        asgn_n->rhs = asgn_v;
                        asgn_n->type = mem_n->type;

                        if (expr) {
                            Node *cma = node_new(cc, ND_COMMA_EXPR, line);
                            cma->lhs = expr;
                            cma->rhs = asgn_n;
                            cma->type = asgn_n->type;
                            expr = cma;
                        } else {
                vtype = parse_declarator(cc, base, vname);

                if (vname[0]) {
                    if (is_typedef) {
                        sym = scope_add(cc, vname, vtype);
                        sym->is_typedef = 1;
                    } else if (is_static_local) {
                ptype = parse_declarator(cc, ptype, pname);

                if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);

                if (func->num_params < MAX_PARAMS) {
                    func->param_types[func->num_params] = ptype;
                    strncpy(func->param_names_buf[func->num_params], pname, MAX_IDENT - 1);
                    psym = scope_add_local(cc, pname, ptype);
                    func->num_params++;
                }
                                    ptype = parse_declarator(cc, ptype, pname);
                                    if (ftype->num_params < MAX_PARAMS) {
                                        ftype->params[ftype->num_params] = ptype;
                                        ftype->num_params++;
                                    }
                                    ptype = parse_declarator(cc, ptype, pname);
                                    if (ftype->num_params < MAX_PARAMS) {
                                        ftype->params[ftype->num_params] = ptype;
                                        ftype->num_params++;
                                    }
                dtype = parse_declarator(cc, base, name2);
                sym = scope_add(cc, name2, dtype);
                sym->is_typedef = 1;
            }
                dtype2 = parse_declarator(cc, base, name2);
                gvar2 = node_new(cc, ND_GLOBAL_VAR, line);
                strncpy(gvar2->name, name2, MAX_IDENT - 1);
                gvar2->type = dtype2;
                gvar2->is_extern = is_extern;
                gvar2->is_static = is_static;

                sym2 = scope_add(cc, name2, dtype2);
                sym2->is_global = 1;

                if (cc->tk == TK_ASSIGN) {
                    next_token(cc);
                    gvar2->initializer = parse_assign(cc);
                }
