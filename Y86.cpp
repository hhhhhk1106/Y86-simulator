#include "Y86.h"

// modify 'in' in HCL 
bool in(std::initializer_list<uint8_t> il) {
    bool ans = 0;
    auto code = il.begin();
    for(auto n = code + 1; n < il.end(); n ++) {
        ans = ans || (*code == *n);
    }
    return ans;
}

uint8_t CPU::getHigh4(uint8_t input) {
    return (input & 0xf0) >> 4;
}
uint8_t CPU::getLow4(uint8_t input) {
    return input & 15;
}
uint64_t CPU::get8(uint64_t pos) {
    return *(uint64_t*)(MEM + pos);
}

void CPU::refresh() {
    this->REG[RNONE] = 0;
}

void CPU::fetch() {
    one.imem_error = PC < 0 || PC > mem_size;
    if(one.imem_error) {
        one.icode = INOP;
        one.ifun = 0;
    } else {
        one.icode = getHigh4(MEM[PC]);
        one.ifun = getLow4(MEM[PC]);
        one.instr_valid = one.icode > icodeValid;
    }

    std::initializer_list<uint8_t> il_r = {one.icode, IRRMOVQ, IOPQ, IIOPQ, IPUSHQ, IPOPQ,\
                                            IIRMOVQ, IRMMOVQ, IMRMOVQ};
    bool need_regids = in(il_r);
    if(need_regids) {
        one.rA = getHigh4(MEM[PC + 1]);
        one.rB = getLow4(MEM[PC + 1]);
    }    
    std::initializer_list<uint8_t> il_vC = {one.icode, IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIOPQ};
    bool need_valC = in(il_vC);
    if(need_valC) {
        one.valC = get8(PC + 1 + need_regids);
    }
    one.valP = PC + need_regids + need_valC * 8 + 1;
}
bool CPU::insValid() {
    std::initializer_list<uint8_t> il_j = {one.icode, IJXX, IRRMOVQ};
    std::initializer_list<uint8_t> il_0 = {one.icode, IHALT, INOP, IIRMOVQ, IRMMOVQ, IMRMOVQ, ICALL, IRET, IPUSHQ, IPOPQ};
    if(one.icode > icodeValid) {
        return 1;
    } else if(one.icode == IOPQ && one.ifun > aluValid) {
        return 1;
    } else if(in(il_j) && one.ifun > jmpValid) {
        return 1;
    } else if(in(il_0) && !one.ifun) {
        return 1;
    }
    return 0;
}

void CPU::decode() {
    setsrcA();
    setsrcB();
    setdstE();
    setdstM();
    one.valA = REG[one.srcA];
    one.valB = REG[one.srcB];
}   
void CPU::setsrcA() {
    std::initializer_list<std::uint8_t> il_rA = {one.icode, IRRMOVQ, IRMMOVQ, IOPQ, IPUSHQ};
    std::initializer_list<std::uint8_t> il_rsp = {one.icode, IPOPQ, IRET};
    if(in(il_rA)) {
        one.srcA = one.rA;
    } else if(in(il_rsp)) {
        one.srcA = RRSP;
    } else {
        one.srcA = RNONE;
    }
}
void CPU::setsrcB() {
    std::initializer_list<std::uint8_t> il_rB = {one.icode, IRMMOVQ, IMRMOVQ, IOPQ, IIOPQ};
    std::initializer_list<std::uint8_t> il_rsp = {one.icode, IPUSHQ, IPOPQ, ICALL, IRET};
    if(in(il_rB)) {
        one.srcB = one.rB;
    } else if(in(il_rsp)) {
        one.srcB = RRSP;
    } else {
        one.srcB = RNONE;
    }
}
void CPU::setdstE() {
    std::initializer_list<uint8_t> il_rB = {one.icode, IRRMOVQ, IIRMOVQ, IOPQ, IIOPQ};
    std::initializer_list<uint8_t> il_rsp = {one.icode, IPUSHQ, IPOPQ, ICALL, IRET};
    if(in(il_rB)) {
        one.dstE = one.rB;
    } else if(in(il_rsp)) {
        //stat?
        one.dstE = RRSP;
    } else {
        one.dstE = RNONE;
    }
}
void CPU::setdstM() {
    std::initializer_list<uint8_t> il_rA = {one.icode, IMRMOVQ, IPOPQ};
    if(in(il_rA)) {
        one.dstM = one.rA;
    } else {
        one.dstM = RNONE;
    }
}

void CPU::execute() {
    uint64_t aluA = getaluA();
    uint64_t aluB = getaluB();
    uint8_t alufun = (one.icode == IOPQ || one.icode == IIOPQ )? one.ifun : ALUADD;
    //calculate
    one.valE = aluFun(alufun, aluA, aluB);
    //set cc
    bool set_cc = (one.icode == IOPQ || one.icode == IIOPQ);
    if(set_cc) {
        CC[ZF] = (one.valE == 0);
        CC[SF] = ((long long)one.valE < 0);
        CC[OF] = (((long long)aluA > 0) && ((long long)aluB > 0) && ((long long)one.valE < 0)) ||
                    (((long long)aluA < 0) && ((long long)aluB < 0) && ((long long)one.valE > 0));
    }
    //jxx
    one.Cnd = cond();
    if(one.icode == IRRMOVQ && !one.Cnd) {
        one.dstE = RNONE;
    }
}
uint64_t CPU::getaluA() {
    std::initializer_list<uint8_t> il_vA = {one.icode, IRRMOVQ, IOPQ};
    std::initializer_list<uint8_t> il_vC = {one.icode, IIRMOVQ, IRMMOVQ, IMRMOVQ, IIOPQ};
    std::initializer_list<uint8_t> il_n8 = {one.icode, ICALL, IPUSHQ};
    std::initializer_list<uint8_t> il_8 = {one.icode, IRET, IPOPQ};
    if(in(il_vA)) {
        return one.valA;
    } else if(in(il_vC)) {
        return one.valC;
    } else if(in(il_n8)) {
        return -8;
    } else if(in(il_8)) {
        return 8;
    } else {
        return 0;
    }
}
uint64_t CPU::getaluB() {
    std::initializer_list<uint8_t> il_vB = {one.icode, IOPQ, IIOPQ, IRMMOVQ, IMRMOVQ,\
                                            IPUSHQ, IPOPQ, ICALL, IRET};
    std::initializer_list<uint8_t> il_0 = {one.icode, IRRMOVQ, IRMMOVQ};
    if(in(il_vB)) {
        return one.valB;
    } else if(in(il_0)) {
        return 0;
    } else {
        return 0;
    }
}
uint64_t CPU::aluFun(uint8_t fun, uint64_t A, uint64_t B) {
    switch (fun) {
    case ALUADD:
        return B + A;
    case ALUSUB:
        return B - A;
    case ALUAND:
        return B & A;
    case ALUXOR:
        return B ^ A; 
    case ALUOR:
        return B | A;
    case ALUSHL:
        return B << A;
    case ALUSHR:
        return B >> A;
    case ALUSAR:
        return (int64_t)B >> A;
    case ALUMULQ:
        return B * A;
    case ALUDIVQ:
        return B / A;
    case ALUREMQ:
        return B - B * (B / A);
    case ALUMINQ:
        return B > A ? A : B;
    case ALUMAXQ:
        return B < A ? A : B; 
    case ALUCMPQ:
        CC[ZF] = (B = A)? 1 : 0;
        CC[SF] = (B > A)? 1 : 0;   
    default:    // OPq
        return 233;
    }
}
bool CPU::cond() {
    switch (one.ifun)
    {
    case UNCON:
        return 1;
    case FLE:
        return (CC[SF] ^ CC[OF]) || CC[ZF];
    case FL:
        return CC[SF] ^ CC[OF];
    case FE:
        return CC[ZF];
    case FNE:
        return !CC[ZF];
    case FGE:
        return !(CC[SF] ^ CC[OF]);
    case FG:
        return !(CC[SF] ^ CC[OF]) && !CC[ZF];
    default:
        return 0;
    }
}

void CPU::memory() {
    uint64_t mem_addr = getAddr();
    uint64_t mem_data = getData();
    std::initializer_list<uint8_t> il_read = {one.icode, IMRMOVQ, IPOPQ, IRET};
    std::initializer_list<uint8_t> il_write = {one.icode, IRMMOVQ, IPUSHQ, ICALL};
    bool mem_read = in(il_read);
    bool mem_write = in(il_write);
    if(mem_read) {
        one.dmem_error = readMem(mem_addr);
    } else if(mem_write) {
        one.dmem_error = writeMem(mem_addr, mem_data);
    }
    one.stat = setStat();
    STAT = one.stat;
}
uint64_t CPU::getAddr() {
    std::initializer_list<uint8_t> il_vE = {one.icode, IRMMOVQ, IMRMOVQ, IPUSHQ, ICALL};
    std::initializer_list<uint8_t> il_vA = {one.icode, IPOPQ, IRET};
    if(in(il_vE)) {
        return one.valE;
    } else if(in(il_vA)) {
        return one.valA;
    } else {
        return 233;
    }
}
uint64_t CPU::getData() {
    std::initializer_list<uint8_t> il_vA = {one.icode, IRMMOVQ, IPUSHQ};
    std::initializer_list<uint8_t> il_vP = {one.icode, ICALL};
    if(in(il_vA)) {
        return one.valA;
    } else if(in(il_vP)) {
        return one.valP;
    }
    return 0;
}
uint8_t CPU::setStat() {
    if(one.instr_valid) {
        return SINS;
    } else if(one.dmem_error || one.imem_error) {
        return SADR;
    } else if(one.icode == IHALT) {
        return SHLT;
    }
    return SAOK;
}
bool CPU::readMem(uint64_t addr) {
    if(addr + 8 > mem_size) {
        return 1;
    } else {
        one.valM = *(long long*)(MEM + addr);
        return 0;
    }
}
bool CPU::writeMem(uint64_t addr, uint64_t data) {
    if(addr > mem_size) {
        return 1;
    } else {
        *(long long*)(MEM + addr) = data;
        return 0;
    }
}

void CPU::writeback() {
    if(one.dstE != RNONE) {
        REG[one.dstE] = one.valE;
    }
    if(one.dstM != RNONE) {
        REG[one.dstM] = one.valM;
    }
}

void CPU::updatePC() {
    one.newPC = setPC();
    if(STAT == SAOK) {
        PC = one.newPC;
    }
}
uint64_t CPU::setPC() {
    if(one.icode == ICALL) {
        return one.valC;
    } else if(one.icode == IJXX && one.Cnd) {
        return one.valC;
    } else if(one.icode == IRET) {
        return one.valM;
    } else {
        return one.valP;
    }
}

void CPU::inState_json(const std::string &filePath) {
    std::ifstream input_j;
    input_j.open(filePath, std::ios::in);
    if(!input_j.is_open()) {
        return;
    }
    Json initial;
    input_j >> initial;
    if(initial.is_array()) {
        initial = initial.at(0).get<Json>();
    }
    if(initial.contains("PC")) {
        this->PC = initial["PC"].get<uint64_t>();
    }
    if(initial.contains("REG")) {
        Json reg = initial["REG"];
        this->REG[RRAX] = (uint64_t)reg["rax"].get<long long>();
        this->REG[RRCX] = (uint64_t)reg["rcx"].get<long long>();
        this->REG[RRDX] = (uint64_t)reg["rdx"].get<long long>();
        this->REG[RRBX] = (uint64_t)reg["rbx"].get<long long>();
        this->REG[RRSP] = (uint64_t)reg["rsp"].get<long long>();
        this->REG[RRBP] = (uint64_t)reg["rbp"].get<long long>();
        this->REG[RRSI] = (uint64_t)reg["rsi"].get<long long>();
        this->REG[RRDI] = (uint64_t)reg["rdi"].get<long long>();
        this->REG[RR8] = (uint64_t)reg["r8"].get<long long>();
        this->REG[RR9] = (uint64_t)reg["r9"].get<long long>();
        this->REG[RR10] = (uint64_t)reg["r10"].get<long long>();
        this->REG[RR11] = (uint64_t)reg["r11"].get<long long>();
        this->REG[RR12] = (uint64_t)reg["r12"].get<long long>();
        this->REG[RR13] = (uint64_t)reg["r13"].get<long long>();
        this->REG[RR14] = (uint64_t)reg["r14"].get<long long>();
        this->REG[RNONE] = 0;
    }
    if(initial.contains("MEM")) {
        Json mem = initial["MEM"];
        for(int i = 0; i < mem_size / 8; i += 8) {
            std::string inx = std::to_string(i);
            if(mem.contains(inx)) {
                *(long long*)(this->MEM + i) = mem[inx].get<long long>();
            }
        }
    }
    if(initial.contains("CC")) {
        Json cc = initial["CC"];
        this->CC[ZF] = (bool) cc["ZF"].get<int>();
        this->CC[SF] = (bool) cc["SF"].get<int>();
        this->CC[OF] = (bool) cc["OF"].get<int>();
    }
    if(initial.contains("STAT")) {
        this->STAT = initial["STAT"].get<uint8_t>();
    }
    input_j.close();
}
void CPU::inState_yoed(const std::string &filePath) {
    std::ifstream input_yo;
    input_yo.open(filePath, std::ios::in | std::ios::binary);
    if(!input_yo.is_open()) {
        return;
    }
    char ch;
    int i = 0;
    while (input_yo.get(ch)) {
        this->MEM[i ++] = (uint8_t)ch;
    }

    input_yo.close();
}
void CPU::inState() {
    char line[160];

    while(std::cin.getline(line, 160)) {
        if(line[0] == '0' && hex2dec(line[7]) != -1) {
            int adr = getAdr(line);
            char val[16];
            int index = 0, temp = 0;
            for(int i = 7; hex2dec(line[i]) != -1;) {
                temp = hex2dec(line[i]) * 16 + hex2dec(line[i+1]);
                val[index++] = temp;
                i += 2;
            }           
            for(int i = 0; i < index; i ++) {
                this->MEM[adr + i] = val[i];
            }
        }
    }

}
Json CPU::outState() {
    Json out;
    Json reg, mem, cc;
    out["PC"] = (uint64_t)PC;

    reg["rax"] = (long long)REG[RRAX];
    reg["rcx"] = (long long)REG[RRCX];
    reg["rdx"] = (long long)REG[RRDX];
    reg["rbx"] = (long long)REG[RRBX];
    reg["rsp"] = (long long)REG[RRSP];
    reg["rbp"] = (long long)REG[RRBP];
    reg["rsi"] = (long long)REG[RRSI];
    reg["rdi"] = (long long)REG[RRDI];
    reg["r8"] = (long long)REG[RR8];
    reg["r9"] = (long long)REG[RR9];
    reg["r10"] = (long long)REG[RR10];
    reg["r11"] = (long long)REG[RR11];
    reg["r12"] = (long long)REG[RR12];
    reg["r13"] = (long long)REG[RR13];
    reg["r14"] = (long long)REG[RR14];
    out["REG"] = reg;

    for(int i = 0; i < mem_size / 8; i += 8) {
        long long content = 0;
        content = *(long long*)(MEM + i);
        if(content) {
            mem[std::to_string(i)] = content;
        }
    }
    out["MEM"] = mem;

    cc["ZF"] = (int)CC[ZF];
    cc["SF"] = (int)CC[SF];
    cc["OF"] = (int)CC[OF];
    out["CC"] = cc;

    out["STAT"] = (uint8_t)STAT;
    return out; 
}
Json CPU::outIns() {
    Json insOut;
    std::string whole;
    std::string ope, jmp, ra, rb;
    switch (one.icode)
    {
    case IHALT:
        whole = "halt";
        break;
    case INOP:
        whole = "nop";
        break;
    case IRRMOVQ: {
        if(one.ifun == 0) {
            ope = "rrmovq";
        } else {
            jmp = getJxxName(one.ifun);
            ope = "cmov" + jmp;
        }
        ra = getRegName(one.rA);
        rb = getRegName(one.rB);
        whole = ope + " " + ra + ", " + rb;
        break;
    }
    case IIRMOVQ: {
        ope = "irmovq";
        long long Val = one.valC;
        rb = getRegName(one.rB);
        whole = ope + " $" + std::to_string(Val) + ", " + rb;
        break;
    }
    case IRMMOVQ: {
        ope = "rmmovq";
        long long offset = one.valC;
        ra = getRegName(one.rA);
        rb = getRegName(one.rB);
        if(offset) {
            whole = ope + " " + ra + ", " + std::to_string(offset) + "(" + rb + ")";
        } else {
            whole = ope + " " + ra + ", (" + rb + ")";
        }    
        break;
    }
    case IMRMOVQ: {
        ope = "mrmovq";
        long long offset = one.valC;
        ra = getRegName(one.rA);
        rb = getRegName(one.rB);
        if(offset) {
            whole = ope + " " + std::to_string(offset) + "(" + rb + ")" + ", " + ra;
        } else {
            whole = ope + " (" + rb + ")" + ", " + ra;
        }
        break;
    }
    case IOPQ: {
        ope = getOPName(one.ifun);
        ra = getRegName(one.rA);
        rb = getRegName(one.rB);
        whole = ope + " " + ra + ", " + rb;
        break;
    }
    case IJXX: {
        if(one.ifun == 0) {
            ope = "jmp";
        } else {
            jmp = getJxxName(one.ifun);
            ope = "j" + jmp;
        }
        long long dest = one.valC;
        whole = ope + " " + std::to_string(dest);
        break;
    }
    case ICALL: {
        ope = "call";
        long long dest = one.valC;
        whole = ope + " " + std::to_string(dest);
        break;
    }
    case IRET:
        whole = "ret";
        break;
    case IPUSHQ: {
        ope = "pushq";
        ra = getRegName(one.rA);
        whole = ope + " " + ra;
        break;
    }
    case IPOPQ: {
        ope = "popq";
        ra = getRegName(one.rA);
        whole = ope + " " + ra;
        break;
    }
    case IIOPQ: {
        ope = getOPName(one.ifun);
        long long Val = one.valC;
        rb = getRegName(one.rB);
        whole = "i" + ope + " $" + std::to_string(Val) + ", " + rb;
        break;
    }    
    default:
        break;  // error
    }
    insOut.push_back(whole);
    return insOut;
}
std::string CPU::getRegName(uint8_t in) {
    switch (in) {
    case RRAX:
        return "%rax";
    case RRCX:
        return "%rcx";
    case RRDX:
        return "%rdx";
    case RRBX:
        return "%rbx";
    case RRSP:
        return "%rsp";
    case RRBP:
        return "%rbp";
    case RRSI:
        return "%rsi";
    case RRDI:
        return "%rdi";
    case RR8:
        return "%r8";
    case RR9:
        return "%r9";
    case RR10:
        return "%r10";
    case RR11:
        return "%r11";
    case RR12:
        return "%r12";
    case RR13:
        return "%r13";
    case RR14:
        return "%r14";
    default:
        return "";  //error
    }
}
std::string CPU::getJxxName(uint8_t in) {
    switch (in) {
    case FLE:
        return "le";
    case FL:
        return "l";
    case FE:
        return "e";
    case FNE:
        return "ne";
    case FGE:
        return "ge";
    case FG:
        return "g";
    default:
        return "";  //error
    }
}
std::string CPU::getOPName(uint8_t in) {
    switch (in) {
    case ALUADD:
        return "addq";
    case ALUSUB:
        return "subq";
    case ALUAND:
        return "andq";
    case ALUXOR:
        return "xorq";
    default:
        return "";  //error
    }
}


