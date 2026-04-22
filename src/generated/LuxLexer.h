
// Generated from /home/carlos/Projects/Cpp/Lux/grammar/LuxLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  LuxLexer : public antlr4::Lexer {
public:
  enum {
    NAMESPACE = 1, USE = 2, RET = 3, STRUCT = 4, UNION = 5, ENUM = 6, NULL_LIT = 7, 
    FN = 8, TYPE = 9, AS = 10, IS = 11, SIZEOF = 12, TYPEOF = 13, IF = 14, 
    ELSE = 15, FOR = 16, IN = 17, LOOP = 18, WHILE = 19, DO = 20, BREAK = 21, 
    CONTINUE = 22, SWITCH = 23, CASE = 24, DEFAULT = 25, SPAWN = 26, AWAIT = 27, 
    LOCK = 28, EXTEND = 29, TRY = 30, CATCH = 31, FINALLY = 32, THROW = 33, 
    DEFER = 34, EXTERN = 35, AUTO = 36, VEC = 37, MAP = 38, SET = 39, TUPLE = 40, 
    ARROW = 41, INCLUDE_SYS = 42, INCLUDE_LOCAL = 43, NULLCOAL = 44, SPREAD = 45, 
    RANGE_INCL = 46, RANGE = 47, INT1 = 48, INT8 = 49, INT16 = 50, INT32 = 51, 
    INT64 = 52, INT128 = 53, INTINF = 54, ISIZE = 55, UINT1 = 56, UINT8 = 57, 
    UINT16 = 58, UINT32 = 59, UINT64 = 60, UINT128 = 61, USIZE = 62, FLOAT32 = 63, 
    FLOAT64 = 64, FLOAT80 = 65, FLOAT128 = 66, DOUBLE = 67, BOOL = 68, CHAR = 69, 
    VOID = 70, STRING = 71, CSTRING = 72, HEX_LIT = 73, OCT_LIT = 74, BIN_LIT = 75, 
    INT_LIT = 76, FLOAT_LIT = 77, BOOL_LIT = 78, C_STR_LIT = 79, STR_LIT = 80, 
    CHAR_LIT = 81, IDENTIFIER = 82, PLUS_ASSIGN = 83, MINUS_ASSIGN = 84, 
    STAR_ASSIGN = 85, SLASH_ASSIGN = 86, PERCENT_ASSIGN = 87, AMP_ASSIGN = 88, 
    PIPE_ASSIGN = 89, CARET_ASSIGN = 90, LSHIFT_ASSIGN = 91, RSHIFT_ASSIGN = 92, 
    SEMI = 93, COLON = 94, SCOPE = 95, COMMA = 96, DOT = 97, ASSIGN = 98, 
    LPAREN = 99, RPAREN = 100, LBRACE = 101, RBRACE = 102, LBRACKET = 103, 
    RBRACKET = 104, STAR = 105, AMPERSAND = 106, MINUS = 107, PLUS = 108, 
    SLASH = 109, PERCENT = 110, EQ = 111, NEQ = 112, LTE = 113, GTE = 114, 
    LT = 115, GT = 116, LAND = 117, LOR = 118, NOT = 119, INCR = 120, DECR = 121, 
    LSHIFT = 122, PIPE = 123, CARET = 124, TILDE = 125, QUESTION = 126, 
    WS = 127, LINE_COMMENT = 128, BLOCK_COMMENT = 129
  };

  explicit LuxLexer(antlr4::CharStream *input);

  ~LuxLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

