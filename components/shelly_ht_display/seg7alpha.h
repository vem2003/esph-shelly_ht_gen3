#pragma once
// =============================================================================
// seg7alpha — 7-Segment Alphabet (MIT-licensed alternative font)
//
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 <YOUR NAME HERE>
//
// An unambiguous alpha-numeric 7-segment character set.
// Every letter has a UNIQUE pattern that does not collide with any digit.
//
// Design origin:
//   Digits 0-9 and letters A,B,C,D,E,F,H,L,N,O,P,R,T,Y use universal
//   industry-standard encodings (hex displays, calculators, since 1960s).
//   Letters I,J,M,Q,S,U are own designs. Letters G,K,V,W,X,Z are
//   constrained-choice (very few viable alternatives given 7 segments
//   and 10 reserved digit patterns).
//   Inspired by the Siekoo alphabet (A. Fakoo, fakoo.de/siekoo),
//   but independently designed with own choices for non-standard chars.
//
// Encoding: bit6=A(top), bit5=B(top-right), bit4=C(bot-right),
//           bit3=D(bottom), bit2=E(bot-left), bit1=F(top-left), bit0=G(middle)
// =============================================================================

namespace esphome {
namespace shelly_htg3 {

// ─── Digits ─────────────────────────────────────────────────────
//  (standard 7-segment, identical across all fonts)

static const uint8_t SEG7_0 = 0x7E;  // ABCDEF      ─┐  ┌─
static const uint8_t SEG7_1 = 0x30;  // BC            │  │
static const uint8_t SEG7_2 = 0x6D;  // ABDEG        ─┤  ├─
static const uint8_t SEG7_3 = 0x79;  // ABCDG         │  │
static const uint8_t SEG7_4 = 0x33;  // BCFG         ─┘  └─
static const uint8_t SEG7_5 = 0x5B;  // ACDFG
static const uint8_t SEG7_6 = 0x5F;  // ACDEFG
static const uint8_t SEG7_7 = 0x70;  // ABC
static const uint8_t SEG7_8 = 0x7F;  // ABCDEFG
static const uint8_t SEG7_9 = 0x7B;  // ABCDFG

// ─── Letters ────────────────────────────────────────────────────
//  Each letter differs from every digit and from every other letter.
//  [standard] = industry convention, [own] = original design,
//  [constrained] = very few viable alternatives exist.

static const uint8_t SEG7_A = 0x77;  // ABCEFG       [standard]
static const uint8_t SEG7_B = 0x1F;  // CDEFG        [standard] lowercase b
static const uint8_t SEG7_C = 0x4E;  // ADEF         [standard]
static const uint8_t SEG7_D = 0x3D;  // BCDEG        [standard] lowercase d
static const uint8_t SEG7_E = 0x4F;  // ADEFG        [standard]
static const uint8_t SEG7_F = 0x47;  // AEFG         [standard]
static const uint8_t SEG7_G = 0x5E;  // ACDEF        [constrained] 6 w/o middle bar
static const uint8_t SEG7_H = 0x37;  // BCEFG        [standard]
static const uint8_t SEG7_I = 0x06;  // EF           [own] left vertical pair
static const uint8_t SEG7_J = 0x38;  // BCD          [own] right hook
static const uint8_t SEG7_K = 0x57;  // ACEFG        [constrained] H + top, no B
static const uint8_t SEG7_L = 0x0E;  // DEF          [standard]
static const uint8_t SEG7_M = 0x66;  // ABEF         [own] upper pillars
static const uint8_t SEG7_N = 0x15;  // CEG          [standard] lowercase n
static const uint8_t SEG7_O = 0x1D;  // CDEG         [standard] lowercase o, != 0
static const uint8_t SEG7_P = 0x67;  // ABEFG        [standard]
static const uint8_t SEG7_Q = 0x7A;  // ABCDF        [own]
static const uint8_t SEG7_R = 0x05;  // EG           [standard] lowercase r
static const uint8_t SEG7_S = 0x4B;  // ADFG         [own]
static const uint8_t SEG7_T = 0x0F;  // DEFG         [standard] lowercase t
static const uint8_t SEG7_U = 0x3C;  // BCDE         [own] wider than std CDE
static const uint8_t SEG7_V = 0x2A;  // BDF          [constrained]
static const uint8_t SEG7_W = 0x2B;  // BDFG         [constrained] V + middle bar
static const uint8_t SEG7_X = 0x14;  // CE           [constrained] diagonal pair
static const uint8_t SEG7_Y = 0x3B;  // BCDFG        [standard] lowercase y
static const uint8_t SEG7_Z = 0x6C;  // ABDE         [constrained] zigzag

// ─── Special Characters ─────────────────────────────────────────

static const uint8_t SEG7_DOT       = 0x04;  // E           .
static const uint8_t SEG7_COMMA     = 0x18;  // CD          ,
static const uint8_t SEG7_SEMICOLON = 0x28;  // BD          ;
static const uint8_t SEG7_COLON     = 0x48;  // AD          :
static const uint8_t SEG7_EXCLAIM   = 0x69;  // ABDG        !
static const uint8_t SEG7_QUESTION  = 0x61;  // ABG         ?
static const uint8_t SEG7_PLUS      = 0x31;  // BCG         +
static const uint8_t SEG7_MINUS     = 0x01;  // G           -
static const uint8_t SEG7_EQUAL     = 0x09;  // DG          =
static const uint8_t SEG7_ASTERISK  = 0x49;  // ADG         *
static const uint8_t SEG7_HASH      = 0x36;  // BCEF        #
static const uint8_t SEG7_SLASH     = 0x25;  // BEG         /
static const uint8_t SEG7_BACKSLASH = 0x13;  // CFG         \.
static const uint8_t SEG7_OVERLINE  = 0x40;  // A           ¯
static const uint8_t SEG7_UNDERSCR  = 0x08;  // D           _
static const uint8_t SEG7_LPAREN    = 0x4E;  // ADEF        (   (= C, intentional)
static const uint8_t SEG7_RPAREN    = 0x78;  // ABCD        )
static const uint8_t SEG7_SQUOTE    = 0x02;  // F           '
static const uint8_t SEG7_DQUOTE    = 0x22;  // BF          "
static const uint8_t SEG7_DEGREE    = 0x63;  // ABFG        °
static const uint8_t SEG7_LESS      = 0x42;  // AF          <
static const uint8_t SEG7_GREATER   = 0x60;  // AB          >
static const uint8_t SEG7_PERCENT   = 0x12;  // CF          %
static const uint8_t SEG7_AT        = 0x74;  // ABCE        @
static const uint8_t SEG7_BLANK     = 0x00;  //             (space)

// ─── Lookup tables ──────────────────────────────────────────────

static const uint8_t SEG7_DIGITS[] = {
  SEG7_0, SEG7_1, SEG7_2, SEG7_3, SEG7_4, SEG7_5, SEG7_6, SEG7_7, SEG7_8, SEG7_9
};

static const uint8_t SEG7_LETTERS[] = {
  SEG7_A, SEG7_B, SEG7_C, SEG7_D, SEG7_E, SEG7_F, SEG7_G, SEG7_H, SEG7_I, SEG7_J,
  SEG7_K, SEG7_L, SEG7_M, SEG7_N, SEG7_O, SEG7_P, SEG7_Q, SEG7_R, SEG7_S, SEG7_T,
  SEG7_U, SEG7_V, SEG7_W, SEG7_X, SEG7_Y, SEG7_Z
};

/// Convert ASCII char to seg7alpha 7-segment code
/// Returns SEG7_BLANK for unsupported characters
inline uint8_t seg7_encode(char c) {
  if (c >= '0' && c <= '9') return SEG7_DIGITS[c - '0'];
  if (c >= 'A' && c <= 'Z') return SEG7_LETTERS[c - 'A'];
  if (c >= 'a' && c <= 'z') return SEG7_LETTERS[c - 'a'];
  switch (c) {
    case ' ': return SEG7_BLANK;
    case '.': return SEG7_DOT;
    case ',': return SEG7_COMMA;
    case ';': return SEG7_SEMICOLON;
    case ':': return SEG7_COLON;
    case '!': return SEG7_EXCLAIM;
    case '?': return SEG7_QUESTION;
    case '+': return SEG7_PLUS;
    case '-': return SEG7_MINUS;
    case '=': return SEG7_EQUAL;
    case '*': return SEG7_ASTERISK;
    case '#': return SEG7_HASH;
    case '/': return SEG7_SLASH;
    case '\\': return SEG7_BACKSLASH;
    case '_': return SEG7_UNDERSCR;
    case '(': return SEG7_LPAREN;
    case ')': return SEG7_RPAREN;
    case '\'': return SEG7_SQUOTE;
    case '"': return SEG7_DQUOTE;
    case '<': return SEG7_LESS;
    case '>': return SEG7_GREATER;
    case '%': return SEG7_PERCENT;
    case '@': return SEG7_AT;
    default:  return SEG7_BLANK;
  }
}

}  // namespace shelly_htg3
}  // namespace esphome
