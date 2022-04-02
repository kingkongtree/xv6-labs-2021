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

GEN_TRANS_PRESHF(add)
GEN_TRANS_PRESHF(sub)
GEN_TRANS_PRESHF(or)
GEN_TRANS_PRESHF(xor)
GEN_TRANS_PRESHF(and)