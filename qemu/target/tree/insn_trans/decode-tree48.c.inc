/* This file is porting from auto-generated decode-tree32.c.inc scripts/decodetree.py.  */

/* insn arg_set */
typedef struct {
    int imm32;
    int rd;
} arg_l_li;

/* insn trans function */
static bool trans_l_li(DisasContext *s, arg_l_li *a)
{
    TCGv tmp;

    if (a->rd != 0) {
        tmp = tcg_temp_new();
        tcg_gen_movi_tl(tmp, a->imm32);
        gen_set_gpr(a->rd, tmp);
        tcg_temp_free(tmp);

        return true;
    }

    return false;
}

/* insn extract func */
static void decode_tree48_extract_l_li(DisasContext *ctx, arg_l_li *a, uint64_t insn)
{
    a->imm32 = extract64(insn, 16, 32);
    a->rd = extract64(insn, 7, 5);
}

/* main decode func */
static bool decode_tree48(DisasContext *ctx, uint64_t insn)
{
    union {
        arg_l_li f_l_li;
    } u;

    switch (insn & 0x0000007f) {
        case 0x0000001f:
            /* ........ ........ ........ ........ .0011111 */
            decode_tree48_extract_l_li(ctx, &u.f_l_li, insn);
            switch ((insn >> 12) & 0xf) {
                case 0x0:
                    /* ........ ........ ........ 0000.... .0011111 */
                    if (trans_l_li(ctx, &u.f_l_li)) return true;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return false;
}