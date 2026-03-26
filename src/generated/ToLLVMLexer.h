
// Generated from /home/carlos/projects/cpp/tollvm/grammar/ToLLVMLexer.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  ToLLVMLexer : public antlr4::Lexer {
public:
  enum {
    NAMESPACE = 1, USE = 2, RET = 3, STRUCT = 4, UNION = 5, ENUM = 6, NULL_LIT = 7, 
    FN = 8, TYPE = 9, AS = 10, IS = 11, SIZEOF = 12, TYPEOF = 13, IF = 14, 
    ELSE = 15, FOR = 16, IN = 17, LOOP = 18, WHILE = 19, DO = 20, BREAK = 21, 
    CONTINUE = 22, SWITCH = 23, CASE = 24, DEFAULT = 25, SPAWN = 26, AWAIT = 27, 
    LOCK = 28, EXTEND = 29, TRY = 30, CATCH = 31, FINALLY = 32, THROW = 33, 
    DEFER = 34, EXTERN = 35, ARROW = 36, INCLUDE_SYS = 37, INCLUDE_LOCAL = 38, 
    NULLCOAL = 39, SPREAD = 40, RANGE_INCL = 41, RANGE = 42, INT1 = 43, 
    INT8 = 44, INT16 = 45, INT32 = 46, INT64 = 47, INT128 = 48, INTINF = 49, 
    ISIZE = 50, UINT1 = 51, UINT8 = 52, UINT16 = 53, UINT32 = 54, UINT64 = 55, 
    UINT128 = 56, USIZE = 57, FLOAT32 = 58, FLOAT64 = 59, FLOAT80 = 60, 
    FLOAT128 = 61, DOUBLE = 62, BOOL = 63, CHAR = 64, VOID = 65, STRING = 66, 
    CSTRING = 67, INT_LIT = 68, FLOAT_LIT = 69, BOOL_LIT = 70, C_STR_LIT = 71, 
    STR_LIT = 72, CHAR_LIT = 73, IDENTIFIER = 74, PLUS_ASSIGN = 75, MINUS_ASSIGN = 76, 
    STAR_ASSIGN = 77, SLASH_ASSIGN = 78, PERCENT_ASSIGN = 79, AMP_ASSIGN = 80, 
    PIPE_ASSIGN = 81, CARET_ASSIGN = 82, LSHIFT_ASSIGN = 83, RSHIFT_ASSIGN = 84, 
    SEMI = 85, COLON = 86, SCOPE = 87, COMMA = 88, DOT = 89, ASSIGN = 90, 
    LPAREN = 91, RPAREN = 92, LBRACE = 93, RBRACE = 94, LBRACKET = 95, RBRACKET = 96, 
    STAR = 97, AMPERSAND = 98, MINUS = 99, PLUS = 100, SLASH = 101, PERCENT = 102, 
    EQ = 103, NEQ = 104, LTE = 105, GTE = 106, LT = 107, GT = 108, LAND = 109, 
    LOR = 110, NOT = 111, INCR = 112, DECR = 113, LSHIFT = 114, RSHIFT = 115, 
    PIPE = 116, CARET = 117, TILDE = 118, QUESTION = 119, WS = 120, LINE_COMMENT = 121, 
    BLOCK_COMMENT = 122
  };

  explicit ToLLVMLexer(antlr4::CharStream *input);

  ~ToLLVMLexer() override;


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

