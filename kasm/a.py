opcodes = ['sto', 'db', 'add', 'adda', 'addb', 'addc', 'sub', 'suba', 'subb', 'subc', 'mul', 'mula', 'mulb', 'mulc', 'div', 'diva', 'divb', 'divc', 'mod', 'moda', 'modb', 'modc', 'and', 'anda', 'andb', 'not', 'nota', 'nand', 'nanda', 'nandb', 'or', 'ora', 'orb', 'nor', 'nora', 'norb', 'xor', 'xora', 'xorb', 'lar', 'lara', 'larb', 'larc', 'lss', 'lssa', 'lssb', 'lssc', 'equ', 'equa', 'equb', 'equc', 'nequ', 'nequa', 'nequb', 'nequc', 'leq', 'leqa', 'leqb', 'leqc', 'lsq', 'lsqa', 'lsqb', 'lsqc', 'stcmp', 'stcmpa', 'inp', 'inpa', 'inpr', 'out', 'outa', 'outr', 'jmp', 'jif', 'label']
headers = """void opcode_sto(const std::string & dst, const std::string & val);
void opcode_db(const std::string & mem, const std::string & val);
void opcode_add(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_adda(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_addb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_addc(const std::string & val1, const std::string & val2, const std::string & dst);
void opcode_sub(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_suba(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_subb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_subc(const std::string & val1, const std::string & val2, const std::string & dst);
void opcode_mul(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_mula(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_mulb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_mulc(const std::string & val1, const std::string & val2, const std::string & dst);
void opcode_div(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_diva(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_divb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_divc(const std::string & val1, const std::string & val2, const std::string & dst);
void opcode_mod(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_moda(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_modb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_modc(const std::string & val1, const std::string & val2, const std::string & dst);
void opcode_and(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_anda(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_andb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_not(const std::string & src, const std::string & dst);
void opcode_nota(const std::string & val, const std::string & dst);
void opcode_nand(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_nanda(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_nandb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_or(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_ora(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_orb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_nor(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_nora(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_norb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_xor(const std::string & src1, const std::string & src2, const std::string & dst);
void opcode_xora(const std::string & src1, const std::string & val, const std::string & dst);
void opcode_xorb(const std::string & val, const std::string & src1, const std::string & dst);
void opcode_lar(const std::string & src1, const std::string & src2);
void opcode_lara(const std::string & src1, const std::string & val);
void opcode_larb(const std::string & val, const std::string & src1);
void opcode_larc(const std::string & val1, const std::string & val2);
void opcode_lss(const std::string & src1, const std::string & src2);
void opcode_lssa(const std::string & src1, const std::string & val);
void opcode_lssb(const std::string & val, const std::string & src1);
void opcode_lssc(const std::string & val1, const std::string & val2);
void opcode_equ(const std::string & src1, const std::string & src2);
void opcode_equa(const std::string & src1, const std::string & val);
void opcode_equb(const std::string & val, const std::string & src1);
void opcode_equc(const std::string & val1, const std::string & val2);
void opcode_nequ(const std::string & src1, const std::string & src2);
void opcode_nequa(const std::string & src1, const std::string & val);
void opcode_nequb(const std::string & val, const std::string & src1);
void opcode_nequc(const std::string & val1, const std::string & val2);
void opcode_leq(const std::string & src1, const std::string & src2);
void opcode_leqa(const std::string & src1, const std::string & val);
void opcode_leqb(const std::string & val, const std::string & src1);
void opcode_leqc(const std::string & val1, const std::string & val2);
void opcode_lsq(const std::string & src1, const std::string & src2);
void opcode_lsqa(const std::string & src1, const std::string & val);
void opcode_lsqb(const std::string & val, const std::string & src1);
void opcode_lsqc(const std::string & val1, const std::string & val2);
void opcode_stcmp(const std::string & src);
void opcode_stcmpa(const std::string & val);
void opcode_inp(const std::string & dst);
void opcode_inpa(const std::string & mem);
void opcode_inpr(const std::string & mem, const std::string & val);
void opcode_out(const std::string & mem);
void opcode_outa(const std::string & val);
void opcode_outr(const std::string & mem, const std::string & val);
void opcode_jmp(const std::string & val);
void opcode_jif(const std::string & val);
void opcode_label(const std::string & val);""".split("\n")
import re
def count_args(cpp_header):
    # Regular expression to match the function arguments
    match = re.search(r'\(([^)]*)\)', cpp_header)
    
    if match:
        # Extract the part inside the parentheses, split by commas and filter empty parts
        args = match.group(1).split(',')
        args = [arg.strip() for arg in args if arg.strip()]
        return len(args)
    else:
        # If no match is found, return 0 (no arguments)
        return 0
b = []
for i in headers:
    b += [count_args(i)]

# Create the if-else chain for each opcode
if_else_chain = ""
for opcode, c in zip(opcodes, b):
    if_else_chain += f'    else if (opcode == "{opcode}") {{\n'
    if_else_chain += f'        opcode_{opcode}({", ".join([f"args[{i}]" for i in range(c)])});\n'
    if_else_chain += f'        instruction_addr += 1;\n'
    if_else_chain += '    }\n'
print(if_else_chain)