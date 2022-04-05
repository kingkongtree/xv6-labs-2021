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

static bool trans_c_lbu(DisasContext *ctx, arg_i *a)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    unsigned int uimm = (unsigned int)a->imm;
    unsigned int uimm_l1 = ((uimm >> 5) & 0x1) << 0;
    unsigned int uimm_m2 = ((uimm >> 0) & 0x3) << 1;
    unsigned int uimm_h2 = ((uimm >> 2) & 0x3) << 3;
    uimm = uimm_l1 | uimm_m2 | uimm_h2;

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, uimm);

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
    int imm = (unsigned int)a->imm;
    int imm_l1 = ((imm >> 5) & 0x1) << 0;
    int imm_m2 = ((imm >> 0) & 0x3) << 1;
    int imm_h2 = ((imm >> 2) & 0x3) << 3;
    imm = imm_l1 | imm_m2 | imm_h2;

    gen_get_gpr(t0, a->rs1);
    tcg_gen_addi_tl(t0, t0, imm);
    gen_get_gpr(dat, a->rs2);

    tcg_gen_qemu_st_tl(dat, t0, ctx->mem_idx, MO_SB);
    tcg_temp_free(t0);
    tcg_temp_free(dat);
    return true;
}