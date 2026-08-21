#include "ui/Font.h"

namespace d25 {

namespace {
// A-Z
const uint8_t kA[7] = {0b01110,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001};
const uint8_t kB[7] = {0b11110,0b10001,0b10001,0b11110,0b10001,0b10001,0b11110};
const uint8_t kC[7] = {0b01110,0b10001,0b10000,0b10000,0b10000,0b10001,0b01110};
const uint8_t kD[7] = {0b11100,0b10010,0b10001,0b10001,0b10001,0b10010,0b11100};
const uint8_t kE[7] = {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b11111};
const uint8_t kF[7] = {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000};
const uint8_t kG[7] = {0b01110,0b10001,0b10000,0b10111,0b10001,0b10001,0b01111};
const uint8_t kH[7] = {0b10001,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001};
const uint8_t kI[7] = {0b01110,0b00100,0b00100,0b00100,0b00100,0b00100,0b01110};
const uint8_t kJ[7] = {0b00111,0b00010,0b00010,0b00010,0b10010,0b10010,0b01100};
const uint8_t kK[7] = {0b10001,0b10010,0b10100,0b11000,0b10100,0b10010,0b10001};
const uint8_t kL[7] = {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111};
const uint8_t kM[7] = {0b10001,0b11011,0b10101,0b10101,0b10001,0b10001,0b10001};
const uint8_t kN[7] = {0b10001,0b11001,0b10101,0b10011,0b10001,0b10001,0b10001};
const uint8_t kO[7] = {0b01110,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110};
const uint8_t kP[7] = {0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000};
const uint8_t kQ[7] = {0b01110,0b10001,0b10001,0b10001,0b10101,0b10010,0b01101};
const uint8_t kR[7] = {0b11110,0b10001,0b10001,0b11110,0b10100,0b10010,0b10001};
const uint8_t kS[7] = {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110};
const uint8_t kT[7] = {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100};
const uint8_t kU[7] = {0b10001,0b10001,0b10001,0b10001,0b10001,0b10001,0b01110};
const uint8_t kV[7] = {0b10001,0b10001,0b10001,0b10001,0b10001,0b01010,0b00100};
const uint8_t kW[7] = {0b10001,0b10001,0b10001,0b10101,0b10101,0b10101,0b01010};
const uint8_t kX[7] = {0b10001,0b10001,0b01010,0b00100,0b01010,0b10001,0b10001};
const uint8_t kY[7] = {0b10001,0b10001,0b01010,0b00100,0b00100,0b00100,0b00100};
const uint8_t kZ[7] = {0b11111,0b00001,0b00010,0b00100,0b01000,0b10000,0b11111};

// 0-9
const uint8_t k0[7] = {0b01110,0b10001,0b10011,0b10101,0b11001,0b10001,0b01110};
const uint8_t k1[7] = {0b00100,0b01100,0b00100,0b00100,0b00100,0b00100,0b01110};
const uint8_t k2[7] = {0b01110,0b10001,0b00001,0b00010,0b00100,0b01000,0b11111};
const uint8_t k3[7] = {0b11110,0b00001,0b00001,0b01110,0b00001,0b00001,0b11110};
const uint8_t k4[7] = {0b00010,0b00110,0b01010,0b10010,0b11111,0b00010,0b00010};
const uint8_t k5[7] = {0b11111,0b10000,0b11110,0b00001,0b00001,0b10001,0b01110};
const uint8_t k6[7] = {0b00110,0b01000,0b10000,0b11110,0b10001,0b10001,0b01110};
const uint8_t k7[7] = {0b11111,0b00001,0b00010,0b00100,0b01000,0b01000,0b01000};
const uint8_t k8[7] = {0b01110,0b10001,0b10001,0b01110,0b10001,0b10001,0b01110};
const uint8_t k9[7] = {0b01110,0b10001,0b10001,0b01111,0b00001,0b00010,0b01100};

// 标点
const uint8_t kSpace  [7] = {0,0,0,0,0,0,0};
const uint8_t kBang   [7] = {0b00100,0b00100,0b00100,0b00100,0b00000,0b00000,0b00100};
const uint8_t kDash   [7] = {0b00000,0b00000,0b00000,0b11111,0b00000,0b00000,0b00000};
const uint8_t kDot    [7] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b00110,0b00110};
const uint8_t kSlash  [7] = {0b00001,0b00010,0b00010,0b00100,0b01000,0b01000,0b10000};
const uint8_t kColon  [7] = {0b00000,0b00110,0b00110,0b00000,0b00110,0b00110,0b00000};

const uint8_t kQuestion[7] = {0b01110,0b10001,0b00001,0b00110,0b00100,0b00000,0b00100};

const uint8_t* kLetters[26] = {
    kA,kB,kC,kD,kE,kF,kG,kH,kI,kJ,kK,kL,kM,
    kN,kO,kP,kQ,kR,kS,kT,kU,kV,kW,kX,kY,kZ
};
const uint8_t* kDigits[10] = {k0,k1,k2,k3,k4,k5,k6,k7,k8,k9};
} // namespace

char Font::normalize(char c) {
    if (c >= 'a' && c <= 'z') return char(c - 'a' + 'A');
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) return c;
    if (c == ' ' || c == '!' || c == '-' || c == '.' || c == '/' || c == ':') return c;
    return '?';
}

const uint8_t* Font::glyph(char c) {
    c = normalize(c);
    if (c >= 'A' && c <= 'Z') return kLetters[c - 'A'];
    if (c >= '0' && c <= '9') return kDigits[c - '0'];
    switch (c) {
        case ' ': return kSpace;
        case '!': return kBang;
        case '-': return kDash;
        case '.': return kDot;
        case '/': return kSlash;
        case ':': return kColon;
        default:  return kQuestion;
    }
}

} // namespace d25
