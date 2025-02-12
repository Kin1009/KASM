#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#elif __APPLE__ || __linux__
#include <ncurses.h>
#else
#error "Unsupported platform"
#endif

std::vector<std::vector<std::string>> parse_operations(const std::string &input) {
    std::vector<std::vector<std::string>> operations;
    std::stringstream ss(input);
    std::string line;

    while (std::getline(ss, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty()) continue;

        std::vector<std::string> operation_parts;
        std::stringstream line_stream(line);
        std::string part;

        while (std::getline(line_stream, part, ' ')) {
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t") + 1);
            operation_parts.push_back(part);
        }

        if (!operation_parts.empty()) {
            operations.push_back(operation_parts);
        }
    }
    return operations;
}
std::string formatPath(const std::string& path) {
    // Check if the path is absolute
    if (std::filesystem::path(path).is_absolute()) {
        return path;
    }

    // Check if it already starts with "./"
    if (path.rfind("./", 0) == 0) {
        return path;
    }

    // Otherwise, prepend "./"
    return "./" + path;
}
uint32_t parse_operand(const std::string &operand) {
    if (operand.empty()) return 0;
    if (operand[0] == '%') return std::stoi(operand.substr(1), nullptr, 2);
    if (operand[0] == '$') return std::stoi(operand.substr(1), nullptr, 16);
    if (operand[0] == '*') return static_cast<uint8_t>(operand[1]);
    if (operand[0] == '#') return std::stoi(operand.substr(1), nullptr, 10);
    return std::stoi(operand, nullptr, 10);
}

std::string read_file(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::ios_base::failure("Failed to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string remove_extension(const std::string &filename) {
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == std::string::npos) return filename;
    return filename.substr(0, last_dot);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./" + std::filesystem::path(__FILE__).stem().string() + " -i <input_file> [-o <output_file>]\n";
        return 1;
    }

    std::string input_file;
    std::string output_file;
    bool output_specified = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i" && i + 1 < argc) {
            input_file = argv[++i];
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
            output_specified = true;
        } else if (input_file.empty()) {
            input_file = arg;  // Assume first argument is input if no `-i`
        }
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified.\n";
        return 1;
    }

    if (!output_specified) {
        output_file = remove_extension(input_file);
        #ifdef _WIN32
                output_file += ".exe";
        #endif
    }
    std::string code = read_file(input_file);
    std::string template_ = R"( 
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <fstream>

#ifdef _WIN32
    #include <conio.h>
#elif __APPLE__ || __linux__
    #include <ncurses.h>
#else
    #error "Unsupported platform"
#endif

std::unordered_map<uint32_t, uint8_t> memory;
std::string input_str;
std::string a;
std::string b;
int c;
bool d;
bool compare_flag = false;

uint8_t get_mem(uint32_t addr) { return memory[addr]; }
void set_mem(uint32_t addr, uint8_t value) { memory[addr] = value; }

int main() {
)";

    std::vector<std::vector<std::string>> instructions = parse_operations(code);
    std::string temp = template_;

    for (const auto &instruction : instructions) {
        if (instruction.empty()) continue;
        std::string opcode = instruction[0];
        std::vector<std::string> args(instruction.begin() + 1, instruction.end());

        if (opcode == "label") {
            temp += "   label" + std::to_string(parse_operand(args[0])) + ":\n";
        } else if (opcode == "inpr") {
            temp +=
                "    input_str = \"\";\n    std::getline(std::cin, input_str);\n"
                "    if (input_str.length() > " +
                std::to_string(parse_operand(args[1])) +
                ") {\n"
                "        input_str = input_str.substr(0, " +
                std::to_string(parse_operand(args[1])) +
                ");\n"
                "    }\n"
                "    for (uint32_t i = 0; i < input_str.length(); ++i) {\n"
                "        set_mem(" +
                std::to_string(parse_operand(args[0])) +
                " + i, static_cast<uint8_t>(input_str[i]));\n"
                "    }\n"
                "    for (uint64_t i = input_str.length(); i < " +
                std::to_string(parse_operand(args[1])) +
                "; ++i) {\n"
                "        set_mem(" +
                std::to_string(parse_operand(args[0])) +
                " + i, 0);\n"
                "    }\n";
        } else if (opcode == "sto") {
            temp += "    set_mem(" + std::to_string(parse_operand(args[0])) + ", " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "outr") {
            temp += "    for (uint32_t i = 0; i < " + std::to_string(parse_operand(args[1]) + 1) + "; ++i) { std::cout << static_cast<char>(get_mem(" + std::to_string(parse_operand(args[0])) +
                    " + i)); }\n";
        } else if (opcode == "jmp") {
            temp += "    goto label" + std::to_string(parse_operand(args[0])) + ";\n";
        } else if (opcode == "add") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") + get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "adda") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") + " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "addb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " + get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "addc") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " + " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "sub") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") - get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "suba") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") - " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "subb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " - get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "subc") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " - " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "mul") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") * get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "mula") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") * " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "mulb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " * get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "mulc") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " * " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "div") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") / get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "diva") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") / " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "divb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " / get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "divc") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " / " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "mod") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") % get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "moda") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") % " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "modb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " % get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "modc") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " % " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "and") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") & get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "anda") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") & " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "andb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " & get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "not") {
            temp += "set_mem(" + std::to_string(parse_operand(args[1])) + ", ~get_mem(" + std::to_string(parse_operand(args[0])) + "));\n";
        } else if (opcode == "nota") {
            temp += "set_mem(" + std::to_string(parse_operand(args[1])) + ", ~" + std::to_string(parse_operand(args[0])) + ");\n";
        } else if (opcode == "nand") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(get_mem(" + std::to_string(parse_operand(args[0])) + ") & get_mem(" + std::to_string(parse_operand(args[1])) + ")));\n";
        } else if (opcode == "nanda") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(get_mem(" + std::to_string(parse_operand(args[0])) + ") & " + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "nandb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(" + std::to_string(parse_operand(args[0])) + " & get_mem(" + std::to_string(parse_operand(args[1])) + ")));\n";
        } else if (opcode == "or") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") | get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "ora") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") | " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "orb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " | get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "nor") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(get_mem(" + std::to_string(parse_operand(args[0])) + ") | get_mem(" + std::to_string(parse_operand(args[1])) + ")));\n";
        } else if (opcode == "nora") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(get_mem(" + std::to_string(parse_operand(args[0])) + ") | " + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "norb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", ~(" + std::to_string(parse_operand(args[0])) + " | get_mem(" + std::to_string(parse_operand(args[1])) + ")));\n";
        } else if (opcode == "xor") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") ^ get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "xora") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", get_mem(" + std::to_string(parse_operand(args[0])) + ") ^ " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "xorb") {
            temp += "set_mem(" + std::to_string(parse_operand(args[2])) + ", " + std::to_string(parse_operand(args[0])) + " ^ get_mem(" + std::to_string(parse_operand(args[1])) + "));\n";
        } else if (opcode == "stcmp") {
            temp += "compare_flag = " + std::string(std::to_string(parse_operand(args[0])) != "0" ? "true" : "false") + ";\n";
        } else if (opcode == "inp") {
            temp += "std::cin >> a;\n";
            temp += "set_mem(" + std::to_string(parse_operand(args[0])) + ", static_cast<uint8_t>(a[0]));\n";
        } else if (opcode == "inpa") {
            temp += "c = getch();\n";
            temp += "c = (c == 0 || c == 224) ? (getch() + 256) : c;\n";
            temp += "set_mem(" + std::to_string(parse_operand(args[0])) + ", c);\n";
        } else if (opcode == "out") {
            temp += "std::cout << static_cast<char>(getmem(" + std::to_string(parse_operand(args[0])) + "));\n";
        } else if (opcode == "outa") {
            temp += "std::cout << static_cast<char>(" + std::to_string(parse_operand(args[0])) + ");\n";
        } else if (opcode == "jif") {
            temp += "if (compare_flag) { goto label" + std::to_string(parse_operand(args[0])) + ";}\n";
        } else if (opcode == "db") {
            temp += "a = " + std::to_string(parse_operand(args[1])) + ";\n";
            temp += "c = " + std::to_string(parse_operand(args[0])) + ";\n";
            temp += "    for (size_t i = 0; i < a.length(); i += 2) { set_mem(c++, static_cast<uint8_t>(std::stoi(a.substr(i, 2), nullptr, 16))); }\n";
        } else if (opcode == "lar") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") > get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lara") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") > " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "larb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " > get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "larc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " > " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lss") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") < get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lssa") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") < " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lssb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " < get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lssc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " < " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "equ") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") == get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "equa") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") == " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "equb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " == get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "equc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " == " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "nequ") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") != get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "nequa") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") != " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "nequb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " != get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "nequc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " != " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "leq") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") >= get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "leqa") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") >= " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "leqb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " >= get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "leqc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " >= " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lsq") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") <= get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lsqa") {
            temp += "compare_flag = get_mem(" + std::to_string(parse_operand(args[0])) + ") <= " + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lsqb") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " <= get_mem(" + std::to_string(parse_operand(args[1])) + ");\n";
        } else if (opcode == "lsqc") {
            temp += "compare_flag = " + std::to_string(parse_operand(args[0])) + " <= " + std::to_string(parse_operand(args[1])) + ");\n";
        }
    }
    temp += "   return 0;\n}\n";

    std::ofstream result(output_file + ".cpp");
    if (!result) {
        throw std::ios_base::failure("Failed to write output file");
    }
    result << temp;
    result.flush();
    result.close();
    output_file = formatPath(output_file);
    std::string compile_command = "g++ " + output_file + ".cpp -o " + output_file;
    int status = system(compile_command.c_str());
    std::filesystem::remove(output_file + ".cpp");
    return status;
}
