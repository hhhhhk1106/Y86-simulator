#ifndef Y86_64_WIRE_H
#define Y86_64_WIRE_H

#include "constVal.h"

struct wire
{
    uint8_t icode = 1;
    uint8_t ifun = 0;
    uint8_t rA = RNONE;
    uint8_t rB = RNONE;
    uint64_t valC = 0;
    uint64_t valP = 0;

    uint64_t valA = 0;
    uint64_t valB = 0;
    uint64_t valE = 0;
    uint64_t valM = 0;
    uint8_t srcA = RNONE;
    uint8_t srcB = RNONE;
    uint8_t dstE = RNONE;
    uint8_t dstM = RNONE;

    bool Cnd = 0;

    uint8_t stat = SAOK;
    bool instr_valid = 0;
    bool imem_error = 0;
    bool dmem_error = 0;

    uint64_t newPC = 0;
};

#endif  // Y86_64_WIRE_H