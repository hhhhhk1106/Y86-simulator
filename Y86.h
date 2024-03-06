#ifndef Y86_64_H
#define Y86_64_H

#include <iostream>
#include <fstream>
#include "wire.h"
#include "trans.h"

#define icodeValid IIOPQ
#define regValid RNONE
#define aluValid ALUCMPQ
#define jmpValid FG
#define mem_size 4096

class CPU
{
public:
    uint8_t STAT = SAOK;

private:
    uint64_t PC = 0;
    uint64_t REG[16] = {0};
    uint8_t MEM[mem_size] = {0};
    bool CC[3] = {1,0,0};
    wire one;

public:

    //tool
    uint8_t getHigh4(uint8_t);
    uint8_t getLow4(uint8_t);
    uint64_t get8(uint64_t pos);
    // reset reg[RNONE]
    void refresh();

    // 6 processing stages:
    void fetch();
    void decode();
    void execute();
    void memory();
    void writeback();
    void updatePC();

    // (fetch)
    bool insValid(); 

    // (decode)
    void setsrcA();
    void setsrcB();
    void setdstE();
    void setdstM();

    // (execute) get aluA, aluB, etc.
    uint64_t getaluA();
    uint64_t getaluB();
    uint64_t aluFun(uint8_t, uint64_t, uint64_t);
    bool cond();

    // (memory) get addr, data, etc.
    uint64_t getAddr();
    uint64_t getData();
    uint8_t setStat();
    bool readMem(uint64_t addr);
    bool writeMem(uint64_t addr, uint64_t data);

    // (updatePC)
    uint64_t setPC();

    //uint8_t reset();

    // in & out
    void inState_json(const std::string &filePath);    // read in .json
    void inState_yoed(const std::string &filePath);    // read in .yoed (processed .yo)
    void inState();
    Json outState();    // put out .json
    Json outIns();      // put out instrctions in json
    std::string getRegName(uint8_t);
    std::string getJxxName(uint8_t);
    std::string getOPName(uint8_t);
};

#endif  // Y86_64_H