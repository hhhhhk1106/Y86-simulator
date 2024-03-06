#ifndef Y86_64_CONSTVAL_H
#define Y86_64_CONSTVAL_H

#include <cstdint>

// put all constants
const uint8_t RRAX = 0;
const uint8_t RRCX = 1;
const uint8_t RRDX = 2;
const uint8_t RRBX = 3;
const uint8_t RRSP = 4;
const uint8_t RRBP = 5;
const uint8_t RRSI = 6;
const uint8_t RRDI = 7;
const uint8_t RR8 = 8;
const uint8_t RR9 = 9;
const uint8_t RR10 = 10;
const uint8_t RR11 = 11;
const uint8_t RR12 = 12;
const uint8_t RR13 = 13;
const uint8_t RR14 = 14;
const uint8_t RNONE = 15;

const uint8_t ZF = 0;
const uint8_t SF = 1;
const uint8_t OF = 2;

const uint8_t SAOK = 1;
const uint8_t SHLT = 2;
const uint8_t SADR = 3;
const uint8_t SINS = 4;

const uint8_t IHALT = 0;
const uint8_t INOP = 1;
const uint8_t IRRMOVQ = 2;
const uint8_t IIRMOVQ = 3;
const uint8_t IRMMOVQ = 4;
const uint8_t IMRMOVQ = 5;
const uint8_t IOPQ = 6;
const uint8_t IJXX = 7;
const uint8_t ICALL = 8;
const uint8_t IRET = 9;
const uint8_t IPUSHQ = 0xa;
const uint8_t IPOPQ = 0xb;
const uint8_t IIOPQ = 0xc;

const uint8_t FNONE = 0;

const uint8_t ALUADD = 0;
const uint8_t ALUSUB = 1;
const uint8_t ALUAND = 2;
const uint8_t ALUXOR = 3;
const uint8_t ALUOR = 4;
const uint8_t ALUSHL = 5;
const uint8_t ALUSHR = 6;
const uint8_t ALUSAR = 7;
const uint8_t ALUMULQ = 8;
const uint8_t ALUDIVQ = 9;
const uint8_t ALUREMQ = 0xa;
const uint8_t ALUMINQ = 0xb;
const uint8_t ALUMAXQ = 0xc;
const uint8_t ALUCMPQ = 0xd;

const uint8_t UNCON = 0;
const uint8_t FLE = 1;
const uint8_t FL = 2;
const uint8_t FE = 3;
const uint8_t FNE = 4;
const uint8_t FGE = 5;
const uint8_t FG = 6;

#endif  // Y86_64_CONSTVAL_H