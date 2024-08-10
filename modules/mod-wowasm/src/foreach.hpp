#pragma once

#define _VA_FIRST_(FIRST, ...) FIRST
#define _VA_SKIP0_(FIRST, ...) __VA_ARGS__

// -- INDEXED --

#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)

#define PP_ARG_N(                          \
  _01,_02,_03,_04,_05,_06,_07,_08,_09,_10, \
  _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
  _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
  _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
  _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
  _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
  _61,_62,_63,N,...) N

#define PP_RSEQ_N()              \
                    63,62,61,60, \
  59,58,57,56,55,54,53,52,51,50, \
  49,48,47,46,45,44,43,42,41,40, \
  39,38,37,36,35,34,33,32,31,30, \
  29,28,27,26,25,24,23,22,21,20, \
  19,18,17,16,15,14,13,12,11,10, \
  09,08,07,06,05,04,03,02,01,00

#define MACRO_CONCAT_(a, b)  a ## b
#define MACRO_CONCAT(a, b)   MACRO_CONCAT_(a, b)
#define MACRO_SELECT(X, ...) MACRO_CONCAT(X, PP_NARG(__VA_ARGS__))

#define EMPTY
#define COMMA ,

#define APPLY_01(X,_01) \
  X(_01,1,EMPTY)
#define APPLY_02(X,_01,_02) \
  X(_01,1,COMMA) X(_02,2,EMPTY)
#define APPLY_03(X,_01,_02,_03) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,EMPTY)
#define APPLY_04(X,_01,_02,_03,_04) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,EMPTY)
#define APPLY_05(X,_01,_02,_03,_04,_05) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,EMPTY)
#define APPLY_06(X,_01,_02,_03,_04,_05,_06) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,EMPTY)
#define APPLY_07(X,_01,_02,_03,_04,_05,_06,_07) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,EMPTY)
#define APPLY_08(X,_01,_02,_03,_04,_05,_06,_07,_08) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,EMPTY)
#define APPLY_09(X,_01,_02,_03,_04,_05,_06,_07,_08,_09) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,EMPTY)
#define APPLY_10(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,EMPTY)
#define APPLY_11(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,EMPTY)
#define APPLY_12(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11,_12) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,COMMA) X(_12,12,EMPTY)
#define APPLY_13(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11,_12,_13) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,COMMA) X(_12,12,COMMA) X(_13,13,EMPTY)
#define APPLY_14(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11,_12,_13,_14) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,COMMA) X(_12,12,COMMA) X(_13,13,COMMA) X(_14,14,EMPTY)
#define APPLY_15(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11,_12,_13,_14,_15) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,COMMA) X(_12,12,COMMA) X(_13,13,COMMA) X(_14,14,COMMA) X(_15,15,EMPTY)
#define APPLY_16(X,_01,_02,_03,_04,_05,_06,_07,_08,_09,_10,_11,_12,_13,_14,_15,_16) \
  X(_01,1,COMMA) X(_02,2,COMMA) X(_03,3,COMMA) X(_04,4,COMMA) X(_05,5,COMMA) X(_06,6,COMMA) X(_07,7,COMMA) X(_08,8,COMMA) X(_09,9,COMMA) X(_10,10,COMMA) X(_11,11,COMMA) X(_12,12,COMMA) X(_13,13,COMMA) X(_14,14,COMMA) X(_15,15,COMMA) X(_16,16,EMPTY)

// #define X(VALUE, INDEX, COMMA)
#define FOR_EACH_INDEXED(X, ...) __VA_OPT__(MACRO_SELECT(APPLY_, __VA_ARGS__)(X, __VA_ARGS__))

// -- INFINITE (NOT INDEXED) --

#define PARENS ()

#define EXPAND(...)  EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

// define X(VALUE, COMMA)
#define FOR_EACH(X, ...)            __VA_OPT__(EXPAND(FOR_EACH_HELPER(X, __VA_ARGS__)))
#define FOR_EACH_HELPER(X, A1, ...) X(A1, __VA_OPT__(COMMA)) __VA_OPT__(FOR_EACH_AGAIN PARENS (X, __VA_ARGS__))
#define FOR_EACH_AGAIN()            FOR_EACH_HELPER
