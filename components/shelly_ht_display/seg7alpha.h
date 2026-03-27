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

static const uint8_t S7_0 = 0x7E;  // ABCDEF      ─┐  ┌─
static const uint8_t S7_1 = 0x30;  // BC            │  │
static const uint8_t S7_2 = 0x6D;  // ABDEG        ─┤  ├─
static const uint8_t S7_3 = 0x79;  // ABCDG         │  │
static const uint8_t S7_4 = 0x33;  // BCFG         ─┘  └─
static const uint8_t S7_5 = 0x5B;  // ACDFG
static const uint8_t S7_6 = 0x5F;  // ACDEFG
static const uint8_t S7_7 = 0x70;  // ABC
static const uint8_t S7_8 = 0x7F;  // ABCDEFG
static const uint8_t S7_9 = 0x7B;  // ABCDFG

// ─── Letters ────────────────────────────────────────────────────
//  Each letter differs from every digit and from every other letter.
//  [standard] = industry convention, [own] = original design,
//  [constrained] = very few viable alternatives exist.

static const uint8_t S7_A = 0x77;  // ABCEFG       [standard]
static const uint8_t S7_B = 0x1F;  // CDEFG        [standard] lowercase b
static const uint8_t S7_C = 0x4E;  // ADEF         [standard]
static const uint8_t S7_D = 0x3D;  // BCDEG        [standard] lowercase d
static const uint8_t S7_E = 0x4F;  // ADEFG        [standard]
static const uint8_t S7_F = 0x47;  // AEFG         [standard]
static const uint8_t S7_G = 0x5E;  // ACDEF        [constrained] 6 w/o middle bar
static const uint8_t S7_H = 0x37;  // BCEFG        [standard]
static const uint8_t S7_I = 0x06;  // EF           [own] left vertical pair
static const uint8_t S7_J = 0x38;  // BCD          [own] right hook
static const uint8_t S7_K = 0x57;  // ACEFG        [constrained] H + top, no B
static const uint8_t S7_L = 0x0E;  // DEF          [standard]
static const uint8_t S7_M = 0x66;  // ABEF         [own] upper pillars
static const uint8_t S7_N = 0x15;  // CEG          [standard] lowercase n
static const uint8_t S7_O = 0x1D;  // CDEG         [standard] lowercase o, != 0
static const uint8_t S7_P = 0x67;  // ABEFG        [standard]
static const uint8_t S7_Q = 0x7A;  // ABCDF        [own]
static const uint8_t S7_R = 0x05;  // EG           [standard] lowercase r
static const uint8_t S7_S = 0x4B;  // ADFG         [own]
static const uint8_t S7_T = 0x0F;  // DEFG         [standard] lowercase t
static const uint8_t S7_U = 0x3C;  // BCDE         [own] wider than std CDE
static const uint8_t S7_V = 0x2A;  // BDF          [constrained]
static const uint8_t S7_W = 0x2B;  // BDFG         [constrained] V + middle bar
static const uint8_t S7_X = 0x14;  // CE           [constrained] diagonal pair
static const uint8_t S7_Y = 0x3B;  // BCDFG        [standard] lowercase y
static const uint8_t S7_Z = 0x6C;  // ABDE         [constrained] zigzag

// ─── Special Characters ─────────────────────────────────────────

static const uint8_t S7_DOT       = 0x04;  // E           .
static const uint8_t S7_COMMA     = 0x18;  // CD          ,
static const uint8_t S7_SEMICOLON = 0x28;  // BD          ;
static const uint8_t S7_COLON     = 0x48;  // AD          :
static const uint8_t S7_EXCLAIM   = 0x69;  // ABDG        !
static const uint8_t S7_QUESTION  = 0x61;  // ABG         ?
static const uint8_t S7_PLUS      = 0x31;  // BCG         +
static const uint8_t S7_MINUS     = 0x01;  // G           -
static const uint8_t S7_EQUAL     = 0x09;  // DG          =
static const uint8_t S7_ASTERISK  = 0x49;  // ADG         *
static const uint8_t S7_HASH      = 0x36;  // BCEF        #
static const uint8_t S7_SLASH     = 0x25;  // BEG         /
static const uint8_t S7_BACKSLASH = 0x13;  // CFG         \.
static const uint8_t S7_OVERLINE  = 0x40;  // A           ¯
static const uint8_t S7_UNDERSCR  = 0x08;  // D           _
static const uint8_t S7_LPAREN    = 0x4E;  // ADEF        (   (= C, intentional)
static const uint8_t S7_RPAREN    = 0x78;  // ABCD        )
static const uint8_t S7_SQUOTE    = 0x02;  // F           '
static const uint8_t S7_DQUOTE    = 0x22;  // BF          "
static const uint8_t S7_DEGREE    = 0x63;  // ABFG        °
static const uint8_t S7_LESS      = 0x42;  // AF          <
static const uint8_t S7_GREATER   = 0x60;  // AB          >
static const uint8_t S7_PERCENT   = 0x12;  // CF          %
static const uint8_t S7_AT        = 0x74;  // ABCE        @
static const uint8_t S7_BLANK     = 0x00;  //             (space)

// ─── Lookup tables ──────────────────────────────────────────────

static const uint8_t S7_DIGITS[] = {
  S7_0, S7_1, S7_2, S7_3, S7_4, S7_5, S7_6, S7_7, S7_8, S7_9
};

static const uint8_t S7_LETTERS[] = {
  S7_A, S7_B, S7_C, S7_D, S7_E, S7_F, S7_G, S7_H, S7_I, S7_J,
  S7_K, S7_L, S7_M, S7_N, S7_O, S7_P, S7_Q, S7_R, S7_S, S7_T,
  S7_U, S7_V, S7_W, S7_X, S7_Y, S7_Z
};

/// Convert ASCII char to seg7alpha 7-segment code
/// Returns S7_BLANK for unsupported characters
inline uint8_t seg7_encode(char c) {
  if (c >= '0' && c <= '9') return S7_DIGITS[c - '0'];
  if (c >= 'A' && c <= 'Z') return S7_LETTERS[c - 'A'];
  if (c >= 'a' && c <= 'z') return S7_LETTERS[c - 'a'];
  switch (c) {
    case ' ': return S7_BLANK;
    case '.': return S7_DOT;
    case ',': return S7_COMMA;
    case ';': return S7_SEMICOLON;
    case ':': return S7_COLON;
    case '!': return S7_EXCLAIM;
    case '?': return S7_QUESTION;
    case '+': return S7_PLUS;
    case '-': return S7_MINUS;
    case '=': return S7_EQUAL;
    case '*': return S7_ASTERISK;
    case '#': return S7_HASH;
    case '/': return S7_SLASH;
    case '\\': return S7_BACKSLASH;
    case '_': return S7_UNDERSCR;
    case '(': return S7_LPAREN;
    case ')': return S7_RPAREN;
    case '\'': return S7_SQUOTE;
    case '"': return S7_DQUOTE;
    case '<': return S7_LESS;
    case '>': return S7_GREATER;
    case '%': return S7_PERCENT;
    case '@': return S7_AT;
    default:  return S7_BLANK;
  }
}

}  // namespace shelly_htg3
}  // namespace esphome
