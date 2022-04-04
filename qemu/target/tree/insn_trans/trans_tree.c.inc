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
    int callee_reg_list[16] = { 2, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1, -1 };
    int caller_reg_list[16] = { 1, 5, 6, 7,  10, 11, 12, 13, 14, 15, 16, 17, 28, 29, 30, 31 };
    int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
    unsigned int mask = a->gpr_mask;

    TCGv target_mem_addr = tcg_temp_new();
    TCGv src_reg_index = tcg_temp_new();

    gen_get_gpr(target_mem_addr, a->rs1);

    for (int i = 0; i < 16; ++i) {
        if (reg_list[i] == -1) break; // if meet undef reg, ignore all left.
        if (!(mask & (1 << i))) continue; // check if masked
        tcg_gen_qemu_ld_tl(src_reg_index, target_mem_addr, s->mem_idx, MO_TEQ); // port from trans_ld
        gen_set_gpr(reg_list[i], src_reg_index);
        tcg_gen_addi_tl(target_mem_addr, target_mem_addr, 8);
    }

    tcg_temp_free(target_mem_addr);
    tcg_temp_free(src_reg_index);

    return true;
}

static bool trans_stmia(DisasContext *s, arg_stmia *a)
{
    int callee_reg_list[16] = { 2, 8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, -1, -1, -1 };
    int caller_reg_list[16] = { 1, 5, 6, 7,  10, 11, 12, 13, 14, 15, 16, 17, 28, 29, 30, 31 };
    int *reg_list = (a->ldm_e) ? caller_reg_list : callee_reg_list;
    unsigned int mask = a->gpr_mask;

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
