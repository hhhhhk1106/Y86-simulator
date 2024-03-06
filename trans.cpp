#include "trans.h"

// translate .yo to .yoed (restore all memory)
void yo2ed(std::string &filePath) {
    std::ifstream input;
    input.open(filePath, std::ios::in);
    char line[160];
    std::ofstream output;
    output.open(filePath + "ed", std::ios::out | std::ios::binary);
    int adr = 0, size = 0;
    char *val = new char[16];

    while(input.getline(line, 160)) {
        if(line[0] == '0' && line[7] != ' ') {
            adr = getAdr(line);
            size = getVal(line, val);

            output.seekp(adr, std::ios::beg);
            output.write(val, size);
        }
    }

    input.close();
    output.close();
}

// translate .yo to .json (initial)
void yo2json(std::string &filePath) {
    std::ifstream input;
    input.open(filePath, std::ios::in);
    char line[160];
    uint8_t MEM[1024] = {0};
    int adr = 0, size = 0;
    char *val = new char[16];
    while(input.getline(line, 160)) {
        if(line[0] == '0' && line[7] != ' ') {
            adr = getAdr(line);
            size = getVal(line, val);
            for(int i = 0; i < size; i ++) {
                MEM[adr + i] = val[i];
            }
        }
    }

    Json out;
    Json reg, mem, cc;
    out["PC"] = (uint64_t)0;

    reg["rax"] = (uint64_t)0;
    reg["rcx"] = (uint64_t)0;
    reg["rdx"] = (uint64_t)0;
    reg["rbx"] = (uint64_t)0;
    reg["rsp"] = (uint64_t)0;
    reg["rbp"] = (uint64_t)0;
    reg["rsi"] = (uint64_t)0;
    reg["rdi"] = (uint64_t)0;
    reg["r8"] = (uint64_t)0;
    reg["r9"] = (uint64_t)0;
    reg["r10"] = (uint64_t)0;
    reg["r11"] = (uint64_t)0;
    reg["r12"] = (uint64_t)0;
    reg["r13"] = (uint64_t)0;
    reg["r14"] = (uint64_t)0;
    out["REG"] = reg;

    for(int i = 0; i < mem_size / 8; i += 8) {
        long long content = 0;
        content = *(long long*)(MEM + i);
        if(content) {

            mem[std::to_string(i)] = content;
        }
    }
    out["MEM"] = mem;

    cc["ZF"] = 1;
    cc["SF"] = 0;
    cc["OF"] = 0;
    out["CC"] = cc;

    out["STAT"] = (uint8_t)1;

    std::ofstream output;
    output.open("initial.json", std::ios::out); //app
    output << std::setw(4) << out << std::endl;

    input.close();
    output.close();
}

int hex2dec(char ch) {
    int ans = -1;
    if(ch >= '0' && ch <= '9') {
        ans = ch - '0';
    } else if(ch >= 'a' && ch <= 'f') {
        ans = ch - 'a' + 10;
    } else if(ch >= 'A' && ch <= 'F') {
        ans = ch - 'A' + 10;
    }
    return ans;
}

int getAdr(char* line) {
    int adr = 0, temp = 0;
    for(int i = 2; line[i] != ':'; i ++) {
        temp = hex2dec(line[i]);
        if(temp == -1) {
            std::cout << "ERROR: invalid hex number" << std::endl;
        }
        adr = adr * 16 + temp;
    }
    return adr;
}

int getVal(char* line, char* val) {
    int index = 0, temp = 0;
    for(int i = 7; line[i] != ' ';) {
        temp = hex2dec(line[i]) * 16 + hex2dec(line[i+1]);
        val[index++] = temp;
        i += 2;
    }
    return index;
}