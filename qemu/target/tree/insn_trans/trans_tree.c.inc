/*
 * RISC-V translation routines for the RVB Standard Extension.
 *
 * Copyright (c) 2020 Kito Cheng, kito.cheng@sifive.com
 * Copyright (c) 2020 Frank Chang, frank.chang@sifive.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define GEN_TRANS_PRESHF(SHFOPC)                                                    \
static bool trans_##SHFOPC##_preshf(DisasContext *ctx, arg_##SHFOPC##_preshf *a)    \
{                                                                                   \
    TCGv shfdst = tcg_temp_new();                                                   \
    TCGv shfamt = tcg_temp_new();                                                   \
    TCGv shfsrc = tcg_temp_new();                                                   \
    TCGv dst = tcg_temp_new();                                                      \
    TCGv src = tcg_temp_new();                                                      \
                                                                                    \
    gen_get_gpr(shfsrc, a->rs2);                                                    \
    tcg_gen_movi_tl(shfamt, a->shfamt);                                             \
                                                                                    \
    switch(a->shftype) { /* by shftype */                                           \
        case 0: /* 00, sll */                                                       \
            tcg_gen_shl_tl(shfdst, shfsrc, shfamt);                                 \
            break;                                                                  \
        case 1: /* 01, srl */                                                       \
            tcg_gen_shr_tl(shfdst, shfsrc, shfamt);                                 \
            break;                                                                  \
        case 2: /* 10, sra */                                                       \
            tcg_gen_sar_tl(shfdst, shfsrc, shfamt);                                 \
            break;                                                                  \
        case 3: /* 11, ror */                                                       \
            tcg_gen_rotr_tl(shfdst, shfsrc, shfamt);                                \
            break;                                                                  \
        default:                                                                    \
            return false;                                                           \
    }                                                                               \
                                                                                    \
    gen_get_gpr(dst, a->rd);                                                        \
    gen_get_gpr(src, a->rs1);                                                       \
                                                                                    \
    tcg_gen_##SHFOPC##_tl(dst, src, shfdst); /* by shfopc */                        \
                                                                                    \
    gen_set_gpr(a->rd, dst);                                                        \
                                                                                    \
    tcg_temp_free(shfdst);                                                          \
    tcg_temp_free(shfsrc);                                                          \
    tcg_temp_free(shfamt);                                                          \
    tcg_temp_free(src);                                                             \
    tcg_temp_free(dst);                                                             \
                                                                                    \
    return true;                                                                    \
}                                                                                   \

static bool trans_or_preshf(DisasContext *ctx, arg_or_preshf *a)
{
    TCGv shfdst = tcg_temp_new();
    TCGv shfamt = tcg_temp_new();
    TCGv shfsrc = tcg_temp_new();
    TCGv dst = tcg_temp_new();
    TCGv src = tcg_temp_new();

    gen_get_gpr(shfsrc, a->rs2);
    tcg_gen_movi_tl(shfamt, a->shfamt);

    switch(a->shftype) { /* by shftype */
        case 0: /* 00, sll */
            tcg_gen_shl_tl(shfdst, shfsrc, shfamt);
            break;
        case 1: /* 01, srl */
            tcg_gen_shr_tl(shfdst, shfsrc, shfamt);
            break;
        case 2: /* 10, sra */
            tcg_gen_sar_tl(shfdst, shfsrc, shfamt);
            break;
        case 3: /* 11, ror */
            tcg_gen_rotr_tl(shfdst, shfsrc, shfamt);
            break;
        default:
            return false;
    }

    gen_get_gpr(dst, a->rd);
    gen_get_gpr(src, a->rs1);

    tcg_gen_or_tl(dst, src, shfdst); /* by shfopc */

    gen_set_gpr(a->rd, dst);

    tcg_temp_free(shfdst);
    tcg_temp_free(shfsrc);
    tcg_temp_free(shfamt);
    tcg_temp_free(src);
    tcg_temp_free(dst);

    return true;
}

GEN_TRANS_PRESHF(add)
GEN_TRANS_PRESHF(sub)
//GEN_TRANS_PRESHF(or)
GEN_TRANS_PRESHF(xor)
GEN_TRANS_PRESHF(and)

static bool trans_ldmia(DisasContext *s, arg_ldmia *a)
{
    int callee_reg_list[16] = { 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 9, 8, 2, -1, -1, -1 };
    int caller_reg_list[16] = { 31, 30, 29, 28,  17, 16, 15, 14, 13, 12, 11, 10, 7, 6, 5, 1 };
    int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
    unsigned int mask = a->gpr_mask;

    for (int i = 0; i < 16; ++i) {
        if (a->rs1 == reg_list[i]) {
            return false;
        }
    }

    TCGv target_mem_addr = tcg_temp_new();
    TCGv src_reg_index;

    gen_get_gpr(target_mem_addr, a->rs1);

    for (int i = 0; i < 16; ++i) {
        if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
        if (!(mask & (1 << i))) continue; // check if masked

        src_reg_index = tcg_temp_new();

        tcg_gen_qemu_ld_tl(src_reg_index, target_mem_addr, s->mem_idx, MO_TEQ); // port from trans_ld
        gen_set_gpr(reg_list[i], src_reg_index);
        tcg_gen_addi_tl(target_mem_addr, target_mem_addr, 8);

        tcg_temp_free(src_reg_index);
    }

    tcg_temp_free(target_mem_addr);

    return true;
}

static bool trans_stmia(DisasContext *s, arg_stmia *a)
{
    int callee_reg_list[16] = { 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 9, 8, 2, -1, -1, -1 };
    int caller_reg_list[16] = { 31, 30, 29, 28,  17, 16, 15, 14, 13, 12, 11, 10, 7, 6, 5, 1 };
    int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
    unsigned int mask = a->gpr_mask;

    for (int i = 0; i < 16; ++i) {
        if (a->rs1 == reg_list[i]) {
            return false;
        }
    }

    TCGv target_mem_addr = tcg_temp_new();
    TCGv src_reg_index;

    gen_get_gpr(target_mem_addr, a->rs1);

    for (int i = 0; i < 16; ++i) {
        if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
        if (!(mask & (1 << i))) continue; // check if masked

        src_reg_index = tcg_temp_new(); // renew temp once

        gen_get_gpr(src_reg_index, reg_list[i]);
        tcg_gen_qemu_st_tl(src_reg_index, target_mem_addr, s->mem_idx, MO_TEQ); // port from trans_sd
        tcg_gen_addi_tl(target_mem_addr, target_mem_addr, 8); // 64bit / 8 = 8
    
        tcg_temp_free(src_reg_index);
    }

    tcg_temp_free(target_mem_addr);

    return true;
}

static bool trans_prefi(DisasContext *s, arg_prefi *a)
{
    return true;
}

static bool trans_prefd(DisasContext *s, arg_prefd *a)
{
    return true;
}

static int ex_tree_cl(DisasContext *ctx, int imm)
{
    /* 2-1-0-4-3 -> 0-4-3-2-1 convertion */
    unsigned int b0 = imm & 0x1;
    unsigned int b1 = (imm >> 1) & 0x1;
    unsigned int b2 = (imm >> 2) & 0x1;
    unsigned int b3 = (imm >> 3) & 0x1;
    unsigned int b4 = (imm >> 4) & 0x1;
    
    return (b1 << 4) | (b0 << 3) | (b4 << 2) | (b3 << 1) | b2;
}

static bool trans_c_lbu(DisasContext *ctx, arg_i *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);

    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_UB);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_sb(DisasContext *ctx, arg_s *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm);
    gen_get_gpr(dat, a->rs2);

    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, MO_SB);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}

static bool trans_c_pop(DisasContext *ctx, arg_c_push *a)
{
    int reg_list[16] = { 0, 1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 10, 11 };
    int n_h_size[16] = { 0, 1, 1, 1, 1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4  };
    int sp_offset = (n_h_size[a->c_rcount] + a->c_sp16imm) * 16;
    int r;
    TCGv t0, t1;

    if (!a->c_rcount) {
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_get_gpr(t0, 2); // t0 = sp
    tcg_gen_addi_tl(t0, t0, sp_offset - a->c_rcount * 4);

    for (r = 1; r <= a->c_rcount; ++r) {
        tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_TESW);
        gen_set_gpr(reg_list[a->c_rcount + 1 - r], t1);
        tcg_gen_addi_tl(t0, t0, 4);
    }

    gen_get_gpr(t0, 2); // t0 = sp
    tcg_gen_addi_tl(t0, t0, sp_offset);
    gen_set_gpr(2, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_c_popret(DisasContext *ctx, arg_c_push *a)
{
    trans_c_pop(ctx, a);

    exit_tb(ctx); /* no chaining */
    ctx->base.is_jmp = DISAS_NORETURN;

    return true;
}

static bool trans_c_push(DisasContext *ctx, arg_c_push *a)
{
    int reg_list[16] = { 0, 1, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 10, 11 };
    int n_h_size[16] = { 0, 1, 1, 1, 1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4  };
    int sp_offset = (n_h_size[a->c_rcount] + a->c_sp16imm) * 16;
    int r;
    TCGv t0, t1;

    if (!a->c_rcount) {
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_get_gpr(t0, 2); // t0 = sp
    tcg_gen_addi_tl(t0, t0, 0 - a->c_rcount * 4);

    for (r = 1; r <= a->c_rcount; ++r) {
        gen_get_gpr(t1, reg_list[a->c_rcount + 1 - r]);
        tcg_gen_qemu_st_tl(t1, t0, ctx->mem_idx, MO_TESW);
        tcg_gen_addi_tl(t0, t0, 4);
    }

    gen_get_gpr(t0, 2); // t0 = sp
    tcg_gen_subi_tl(t0, t0, sp_offset);
    gen_set_gpr(2, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

// porting from gen_branch
static bool gen_branchi(DisasContext *ctx, arg_bi *a, TCGCond cond)
{
    TCGLabel *l = gen_new_label();
    TCGv source1, source2;
    source1 = tcg_temp_new();
    source2 = tcg_temp_new();
    gen_get_gpr(source1, a->rs1);
    tcg_gen_movi_tl(source2, a->bi_cmp);

    tcg_gen_brcond_tl(cond, source1, source2, l);
    gen_goto_tb(ctx, 1, ctx->pc_succ_insn);
    gen_set_label(l); /* branch taken */

    if (!has_ext(ctx, RVC) && ((ctx->base.pc_next + (a->bi_offset << 1)) & 0x3)) { // bi_offset align to 16bytes
        /* misaligned */
        gen_exception_inst_addr_mis(ctx);
    } else {
        gen_goto_tb(ctx, 0, ctx->base.pc_next + a->bi_offset);
    }
    ctx->base.is_jmp = DISAS_NORETURN;

    tcg_temp_free(source1);
    tcg_temp_free(source2);

    return true;
}

static bool trans_beqi(DisasContext *ctx, arg_beqi *a)
{
    return gen_branchi(ctx, a, TCG_COND_EQ);
}

static bool trans_bnei(DisasContext *ctx, arg_bnei *a)
{
    return gen_branchi(ctx, a, TCG_COND_NE);
}

static bool trans_blti(DisasContext *ctx, arg_blti *a)
{
    return gen_branchi(ctx, a, TCG_COND_LT);
}

static bool trans_bgei(DisasContext *ctx, arg_bgei *a)
{
    return gen_branchi(ctx, a, TCG_COND_GE);
}

static bool trans_bltui(DisasContext *ctx, arg_bltui *a)
{
    return gen_branchi(ctx, a, TCG_COND_LTU);
}

static bool trans_bgeui(DisasContext *ctx, arg_bgeui *a)
{
    return gen_branchi(ctx, a, TCG_COND_GEU);
}

static bool trans_c_utxb(DisasContext *ctx, arg_c_utx *a)
{
    TCGv t_rs = tcg_temp_new();
    gen_get_gpr(t_rs, a->rs1);

    tcg_gen_andi_tl(t_rs, t_rs, 255);
    gen_set_gpr(a->rs1, t_rs);

    tcg_temp_free(t_rs);

    return true;
}

static bool trans_c_utxh(DisasContext *ctx, arg_c_utx *a)
{
    TCGv source1 = tcg_temp_new();
    TCGv source2 = tcg_temp_new();

    gen_get_gpr(source1, a->rs1);

    tcg_gen_movi_tl(source2, 16);
    tcg_gen_shl_tl(source1, source1, source2);
    tcg_gen_shr_tl(source1, source1, source2);

    gen_set_gpr(a->rs1, source1);
    tcg_temp_free(source1);
    tcg_temp_free(source2);
    return true;
}

static bool trans_c_lhu(DisasContext *ctx, arg_i *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm << 1); // step 16-bits

    tcg_gen_qemu_ld_tl(t1, t0, ctx->mem_idx, MO_UW);
    gen_set_gpr(a->rd, t1);
    tcg_temp_free(t0);
    tcg_temp_free(t1);
    return true;
}

static bool trans_c_sh(DisasContext *ctx, arg_s *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv dat = tcg_temp_new();

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, a->imm << 1); // step 16-bits
    gen_get_gpr(dat, a->rs2);

    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, MO_SW);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}

static bool trans_muliadd(DisasContext *ctx, arg_muliadd *a)
{
    TCGv tmp_rs1 = tcg_temp_new();
    TCGv tmp_rs2 = tcg_temp_new();

    gen_get_gpr(tmp_rs1, a->rs1);
    gen_get_gpr(tmp_rs2, a->rs2);
    tcg_gen_muli_tl(tmp_rs2, tmp_rs2, a->uimm);
    tcg_gen_add_tl(tmp_rs1, tmp_rs1, tmp_rs2);

    gen_set_gpr(a->rd, tmp_rs1);

    tcg_temp_free(tmp_rs1);
    tcg_temp_free(tmp_rs2);

    return true;
}

static bool trans_j16m(DisasContext *ctx, arg_j16m *a)
{
    gen_goto_tb(ctx, 0, a->simm << 1); // pc = sext(simm * 2)
    return true;
}

static bool trans_jal16m(DisasContext *ctx, arg_j16m *a)
{
    gen_jal(ctx, 1, a->simm << 1); // pc = ra + sext(simm * 2)
    return true;
}