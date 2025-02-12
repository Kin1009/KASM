#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
// For Windows, we need to use the Windows-specific conio.h
#include <conio.h>

#elif __APPLE__
// For macOS, you can use the curses library (ncurses) as it supports getch() natively.
#include <ncurses.h>

#elif __linux__
// For Linux, ncurses also works.
#include <ncurses.h>

#else
#error "Unsupported platform"
#endif

// Define memory (64KB for now, expandable)
std::unordered_map<uint32_t, uint8_t> memory;
bool compare_flag = false;  // Stores result for conditional jumps
uint instruction_addr = 0;
std::unordered_map<uint, uint> labels;
// Fetch memory value
uint8_t get_mem(uint32_t addr) { return memory[addr]; }

// Store memory value
void set_mem(uint32_t addr, uint8_t value) { memory[addr] = value; }

// Convert operand string to integer value
uint32_t parse_operand(const std::string& operand) {
    if (operand.empty()) return 0;
    if (operand[0] == '%') return std::stoi(operand.substr(1), nullptr, 2);   // Binary
    if (operand[0] == '$') return std::stoi(operand.substr(1), nullptr, 16);  // Hex
    if (operand[0] == '*') return static_cast<uint8_t>(operand[1]);           // Character
    if (operand[0] == '#') return std::stoi(operand.substr(1), nullptr, 10);  // Decimal
    return std::stoi(operand, nullptr, 10);                                   // Default to decimal
}

// Store value in memory (sto MEM VAL)
void opcode_sto(const std::string& dst, const std::string& val) { set_mem(parse_operand(dst), parse_operand(val)); }

// opcode_db (db MEM VAL)
void opcode_db(const std::string& mem, const std::string& val) {
    // Parse the memory address
    uint32_t address = parse_operand(mem);

    // Convert the VAL string (hexadecimal) into a vector of bytes
    size_t length = val.length();

    // Ensure the VAL string has an even number of characters
    if (length % 2 != 0) {
        std::cerr << "Error: VAL string must have an even number of characters." << std::endl;
        return;
    }

    // Loop through the VAL string in 2-character chunks (each representing 1 byte)
    for (size_t i = 0; i < length; i += 2) {
        // Extract the next 2 characters (1 byte)
        std::string hex_byte = val.substr(i, 2);

        // Convert the 2-character hex string to an integer (byte)
        uint8_t byte = static_cast<uint8_t>(std::stoi(hex_byte, nullptr, 16));

        // Store the byte in memory
        set_mem(address, byte);

        // Increment the memory address to store the next byte
        address += 1;
    }
}

// Arithmetic operations (A B DEST)
void opcode_add(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) + get_mem(parse_operand(src2))); }
void opcode_adda(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) + parse_operand(val)); }
void opcode_addb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) + get_mem(parse_operand(src1))); }
void opcode_addc(const std::string& val1, const std::string& val2, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val1) + parse_operand(val2)); }

void opcode_sub(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) - get_mem(parse_operand(src2))); }
void opcode_suba(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) - parse_operand(val)); }
void opcode_subb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) - get_mem(parse_operand(src1))); }
void opcode_subc(const std::string& val1, const std::string& val2, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val1) - parse_operand(val2)); }

void opcode_mul(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) * get_mem(parse_operand(src2))); }
void opcode_mula(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) * parse_operand(val)); }
void opcode_mulb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) * get_mem(parse_operand(src1))); }
void opcode_mulc(const std::string& val1, const std::string& val2, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val1) * parse_operand(val2)); }

void opcode_div(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) / get_mem(parse_operand(src2))); }
void opcode_diva(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) / parse_operand(val)); }
void opcode_divb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) / get_mem(parse_operand(src1))); }
void opcode_divc(const std::string& val1, const std::string& val2, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val1) / parse_operand(val2)); }

void opcode_mod(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) % get_mem(parse_operand(src2))); }
void opcode_moda(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) % parse_operand(val)); }
void opcode_modb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) % get_mem(parse_operand(src1))); }
void opcode_modc(const std::string& val1, const std::string& val2, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val1) % parse_operand(val2)); }

// Logic operations
void opcode_and(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) & get_mem(parse_operand(src2))); }
void opcode_anda(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) & parse_operand(val)); }
void opcode_andb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) & get_mem(parse_operand(src1))); }
void opcode_not(const std::string& src, const std::string& dst) { set_mem(parse_operand(dst), ~get_mem(parse_operand(src))); }
void opcode_nota(const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), ~parse_operand(val)); }
void opcode_nand(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), ~(get_mem(parse_operand(src1)) & get_mem(parse_operand(src2)))); }
void opcode_nanda(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), ~(get_mem(parse_operand(src1)) & parse_operand(val))); }
void opcode_nandb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), ~(parse_operand(val) & get_mem(parse_operand(src1)))); }
void opcode_or(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) | get_mem(parse_operand(src2))); }
void opcode_ora(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) | parse_operand(val)); }
void opcode_orb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) | get_mem(parse_operand(src1))); }
void opcode_nor(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), ~(get_mem(parse_operand(src1)) | get_mem(parse_operand(src2)))); }
void opcode_nora(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), ~(get_mem(parse_operand(src1)) | parse_operand(val))); }
void opcode_norb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), ~(parse_operand(val) | get_mem(parse_operand(src1)))); }
void opcode_xor(const std::string& src1, const std::string& src2, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) ^ get_mem(parse_operand(src2))); }
void opcode_xora(const std::string& src1, const std::string& val, const std::string& dst) { set_mem(parse_operand(dst), get_mem(parse_operand(src1)) ^ parse_operand(val)); }
void opcode_xorb(const std::string& val, const std::string& src1, const std::string& dst) { set_mem(parse_operand(dst), parse_operand(val) ^ get_mem(parse_operand(src1))); }

// Comparison operations
void opcode_lar(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) > get_mem(parse_operand(src2)); }
void opcode_lara(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) > parse_operand(val); }
void opcode_larb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) > get_mem(parse_operand(src1)); }
void opcode_larc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) > parse_operand(val2); }
void opcode_lss(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) < get_mem(parse_operand(src2)); }
void opcode_lssa(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) < parse_operand(val); }
void opcode_lssb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) < get_mem(parse_operand(src1)); }
void opcode_lssc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) < parse_operand(val2); }
void opcode_equ(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) == get_mem(parse_operand(src2)); }
void opcode_equa(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) == parse_operand(val); }
void opcode_equb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) == get_mem(parse_operand(src1)); }
void opcode_equc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) == parse_operand(val2); }
void opcode_nequ(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) != get_mem(parse_operand(src2)); }
void opcode_nequa(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) != parse_operand(val); }
void opcode_nequb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) != get_mem(parse_operand(src1)); }
void opcode_nequc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) != parse_operand(val2); }
void opcode_leq(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) >= get_mem(parse_operand(src2)); }
void opcode_leqa(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) >= parse_operand(val); }
void opcode_leqb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) >= get_mem(parse_operand(src1)); }
void opcode_leqc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) >= parse_operand(val2); }
void opcode_lsq(const std::string& src1, const std::string& src2) { compare_flag = get_mem(parse_operand(src1)) <= get_mem(parse_operand(src2)); }
void opcode_lsqa(const std::string& src1, const std::string& val) { compare_flag = get_mem(parse_operand(src1)) <= parse_operand(val); }
void opcode_lsqb(const std::string& val, const std::string& src1) { compare_flag = parse_operand(val) <= get_mem(parse_operand(src1)); }
void opcode_lsqc(const std::string& val1, const std::string& val2) { compare_flag = parse_operand(val1) <= parse_operand(val2); }

// Set compare flag manually (stcmp VAL)
void opcode_stcmp(const std::string& val) { compare_flag = (parse_operand(val) != 0); }
void opcode_inp(const std::string& dst) {
    char input;
    std::cin >> input;
    set_mem(parse_operand(dst), static_cast<uint8_t>(input));
}
// Input operation (supporting special keys)
void opcode_inpa(const std::string& mem) {
    uint8_t key = getch();
    if (key == 0 || key == 224) {  // Special keys (arrows, function keys)
        key = getch();
        set_mem(parse_operand(mem), key + 256);  // Store special key identifier
    } else {
        set_mem(parse_operand(mem), key);
    }
}
// Input operation (inpr MEM VAL)
void opcode_inpr(const std::string& mem, const std::string& val) {
    // Parse the starting memory address and the number of characters to input
    uint32_t address = parse_operand(mem);
    uint32_t num_chars = parse_operand(val);  // Number of characters to input

    // Read the string of characters from user input
    std::string input_str;
    std::getline(std::cin, input_str);  // Read a full line of input

    // Ensure we do not exceed the specified number of characters
    if (input_str.length() > num_chars) {
        input_str = input_str.substr(0, num_chars);  // Truncate to the maximum size
    }

    // Store each character in memory at the current address
    for (uint32_t i = 0; i < input_str.length(); ++i) {
        set_mem(address, static_cast<uint8_t>(input_str[i]));  // Store the character as byte
        address += 1;                                          // Increment address to store the next character
    }

    // If the input string is shorter than the number of characters requested,
    // fill remaining memory with zeroes
    for (uint64_t i = input_str.length(); i < num_chars; ++i) {
        set_mem(address, 0);  // Store 0 to pad the remaining memory
        address += 1;
    }
}

// Output operations
void opcode_out(const std::string& mem) { std::cout << static_cast<char>(get_mem(parse_operand(mem))); }
void opcode_outa(const std::string& val) { std::cout << static_cast<char>(parse_operand(val)); }
// Output operation (outr MEM VAL)
void opcode_outr(const std::string& mem, const std::string& val) {
    // Parse memory address and number of characters to output
    uint32_t address = parse_operand(mem);
    uint32_t num_chars = parse_operand(val) + 1;  // Number of characters to output

    // Output the characters from memory starting at 'address'
    for (uint32_t i = 0; i < num_chars; ++i) {
        std::cout << static_cast<char>(get_mem(address));
        address += 1;  // Move to the next memory location
    }
}

// Jump and jumpif
void opcode_jmp(const std::string& val) {
    uint name = parse_operand(val);
    instruction_addr = labels[name];
}

void opcode_jif(const std::string& val) {
    uint name = parse_operand(val);
    if (compare_flag) {
        instruction_addr = labels[name];
    }
}

// Labels
void opcode_label(const std::string& val) {
    uint name = parse_operand(val);
    labels[name] = instruction_addr;
}

std::vector<std::vector<std::string>> parse_operations(const std::string& input) {
    std::vector<std::vector<std::string>> operations;
    std::stringstream ss(input);
    std::string line;

    // Read each line from the input string
    while (std::getline(ss, line)) {
        // Remove leading/trailing spaces from the line
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // If the line is empty, skip it
        if (line.empty()) continue;

        std::vector<std::string> operation_parts;
        std::stringstream line_stream(line);
        std::string part;

        // Extract the opcode (first part) and operands (subsequent parts)
        while (std::getline(line_stream, part, ' ')) {
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t") + 1);
            operation_parts.push_back(part);
        }

        // If the operation has operands (i.e., more than one part)
        if (operation_parts.size() > 1) {
            std::vector<std::string> operands;
            std::string op = operation_parts[0];  // opcode
            operands.push_back(op);

            // Parse the operands, if any
            for (size_t i = 1; i < operation_parts.size(); ++i) {
                std::string operand = operation_parts[i];
                std::stringstream operand_stream(operand);
                std::string sub_operand;

                // Split operands by commas
                while (std::getline(operand_stream, sub_operand, ',')) {
                    sub_operand.erase(0, sub_operand.find_first_not_of(" \t"));
                    sub_operand.erase(sub_operand.find_last_not_of(" \t") + 1);
                    operands.push_back(sub_operand);
                }
            }

            operations.push_back(operands);  // Add parsed operation
        }
        // If the line only contains an opcode with no operands
        else if (operation_parts.size() == 1) {
            operations.push_back({operation_parts[0]});  // Add opcode with no operands
        }
    }

    return operations;
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);  // Open the file
    std::stringstream buffer;

    if (file) {
        buffer << file.rdbuf();  // Read file content into stringstream
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    return buffer.str();  // Return the file content as a string
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./main <file_name>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];  // Get file name from command-line arguments

    // Read the file content
    std::string code = read_file(filename);

    std::vector<std::vector<std::string>> instructions = parse_operations(code);
    // Loop through each instruction
    while (instruction_addr < instructions.size()) {
        const auto& instruction = instructions[instruction_addr];
        // First element is the opcode
        std::string opcode = instruction[0];

        // Remaining elements are the arguments
        std::vector<std::string> args(instruction.begin() + 1, instruction.end());
        if (opcode == "sto") {
            opcode_sto(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "db") {
            opcode_db(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "add") {
            opcode_add(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "adda") {
            opcode_adda(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "addb") {
            opcode_addb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "addc") {
            opcode_addc(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "sub") {
            opcode_sub(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "suba") {
            opcode_suba(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "subb") {
            opcode_subb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "subc") {
            opcode_subc(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "mul") {
            opcode_mul(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "mula") {
            opcode_mula(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "mulb") {
            opcode_mulb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "mulc") {
            opcode_mulc(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "div") {
            opcode_div(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "diva") {
            opcode_diva(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "divb") {
            opcode_divb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "divc") {
            opcode_divc(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "mod") {
            opcode_mod(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "moda") {
            opcode_moda(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "modb") {
            opcode_modb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "modc") {
            opcode_modc(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "and") {
            opcode_and(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "anda") {
            opcode_anda(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "andb") {
            opcode_andb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "not") {
            opcode_not(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nota") {
            opcode_nota(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nand") {
            opcode_nand(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "nanda") {
            opcode_nanda(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "nandb") {
            opcode_nandb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "or") {
            opcode_or(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "ora") {
            opcode_ora(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "orb") {
            opcode_orb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "nor") {
            opcode_nor(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "nora") {
            opcode_nora(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "norb") {
            opcode_norb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "xor") {
            opcode_xor(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "xora") {
            opcode_xora(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "xorb") {
            opcode_xorb(args[0], args[1], args[2]);
            instruction_addr += 1;
        } else if (opcode == "lar") {
            opcode_lar(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lara") {
            opcode_lara(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "larb") {
            opcode_larb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "larc") {
            opcode_larc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lss") {
            opcode_lss(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lssa") {
            opcode_lssa(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lssb") {
            opcode_lssb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lssc") {
            opcode_lssc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "equ") {
            opcode_equ(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "equa") {
            opcode_equa(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "equb") {
            opcode_equb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "equc") {
            opcode_equc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nequ") {
            opcode_nequ(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nequa") {
            opcode_nequa(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nequb") {
            opcode_nequb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "nequc") {
            opcode_nequc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "leq") {
            opcode_leq(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "leqa") {
            opcode_leqa(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "leqb") {
            opcode_leqb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "leqc") {
            opcode_leqc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lsq") {
            opcode_lsq(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lsqa") {
            opcode_lsqa(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lsqb") {
            opcode_lsqb(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "lsqc") {
            opcode_lsqc(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "stcmp") {
            opcode_stcmp(args[0]);
            instruction_addr += 1;
        } else if (opcode == "inp") {
            opcode_inp(args[0]);
            instruction_addr += 1;
        } else if (opcode == "inpa") {
            opcode_inpa(args[0]);
            instruction_addr += 1;
        } else if (opcode == "inpr") {
            opcode_inpr(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "out") {
            opcode_out(args[0]);
            instruction_addr += 1;
        } else if (opcode == "outa") {
            opcode_outa(args[0]);
            instruction_addr += 1;
        } else if (opcode == "outr") {
            opcode_outr(args[0], args[1]);
            instruction_addr += 1;
        } else if (opcode == "jmp") {
            opcode_jmp(args[0]);
            instruction_addr += 1;
        } else if (opcode == "jif") {
            opcode_jif(args[0]);
            instruction_addr += 1;
        } else if (opcode == "label") {
            opcode_label(args[0]);
            instruction_addr += 1;
        }
    }
    return 0;
}
