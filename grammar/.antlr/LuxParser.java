// Generated from /home/carlos/Projects/Cpp/Lux/grammar/LuxParser.g4 by ANTLR 4.13.1
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.misc.*;
import org.antlr.v4.runtime.tree.*;
import java.util.List;
import java.util.Iterator;
import java.util.ArrayList;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast", "CheckReturnValue"})
public class LuxParser extends Parser {
	static { RuntimeMetaData.checkVersion("4.13.1", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		NAMESPACE=1, USE=2, RET=3, STRUCT=4, UNION=5, ENUM=6, NULL_LIT=7, FN=8, 
		TYPE=9, AS=10, IS=11, SIZEOF=12, TYPEOF=13, IF=14, ELSE=15, FOR=16, IN=17, 
		LOOP=18, WHILE=19, DO=20, BREAK=21, CONTINUE=22, SWITCH=23, CASE=24, DEFAULT=25, 
		SPAWN=26, AWAIT=27, LOCK=28, EXTEND=29, TRY=30, CATCH=31, FINALLY=32, 
		THROW=33, DEFER=34, EXTERN=35, AUTO=36, VEC=37, MAP=38, SET=39, TUPLE=40, 
		ARROW=41, INCLUDE_SYS=42, INCLUDE_LOCAL=43, NULLCOAL=44, SPREAD=45, RANGE_INCL=46, 
		RANGE=47, INT1=48, INT8=49, INT16=50, INT32=51, INT64=52, INT128=53, INTINF=54, 
		ISIZE=55, UINT1=56, UINT8=57, UINT16=58, UINT32=59, UINT64=60, UINT128=61, 
		USIZE=62, FLOAT32=63, FLOAT64=64, FLOAT80=65, FLOAT128=66, DOUBLE=67, 
		BOOL=68, CHAR=69, VOID=70, STRING=71, CSTRING=72, HEX_LIT=73, OCT_LIT=74, 
		BIN_LIT=75, INT_LIT=76, FLOAT_LIT=77, BOOL_LIT=78, C_STR_LIT=79, STR_LIT=80, 
		CHAR_LIT=81, IDENTIFIER=82, PLUS_ASSIGN=83, MINUS_ASSIGN=84, STAR_ASSIGN=85, 
		SLASH_ASSIGN=86, PERCENT_ASSIGN=87, AMP_ASSIGN=88, PIPE_ASSIGN=89, CARET_ASSIGN=90, 
		LSHIFT_ASSIGN=91, RSHIFT_ASSIGN=92, SEMI=93, COLON=94, SCOPE=95, COMMA=96, 
		DOT=97, ASSIGN=98, LPAREN=99, RPAREN=100, LBRACE=101, RBRACE=102, LBRACKET=103, 
		RBRACKET=104, STAR=105, AMPERSAND=106, MINUS=107, PLUS=108, SLASH=109, 
		PERCENT=110, EQ=111, NEQ=112, LTE=113, GTE=114, LT=115, GT=116, LAND=117, 
		LOR=118, NOT=119, INCR=120, DECR=121, LSHIFT=122, PIPE=123, CARET=124, 
		TILDE=125, QUESTION=126, WS=127, LINE_COMMENT=128, BLOCK_COMMENT=129;
	public static final int
		RULE_program = 0, RULE_preambleDecl = 1, RULE_namespaceDecl = 2, RULE_useDecl = 3, 
		RULE_modulePath = 4, RULE_includeDecl = 5, RULE_topLevelDecl = 6, RULE_typeAliasDecl = 7, 
		RULE_enumDecl = 8, RULE_structDecl = 9, RULE_structField = 10, RULE_unionDecl = 11, 
		RULE_unionField = 12, RULE_externDecl = 13, RULE_externParamList = 14, 
		RULE_externParam = 15, RULE_functionDecl = 16, RULE_extendDecl = 17, RULE_typeParamList = 18, 
		RULE_typeParam = 19, RULE_extendMethod = 20, RULE_paramList = 21, RULE_param = 22, 
		RULE_block = 23, RULE_statement = 24, RULE_deferStmt = 25, RULE_exprStmt = 26, 
		RULE_varDeclStmt = 27, RULE_assignStmt = 28, RULE_compoundAssignStmt = 29, 
		RULE_fieldAssignStmt = 30, RULE_indexFieldAssignStmt = 31, RULE_derefAssignStmt = 32, 
		RULE_arrowAssignStmt = 33, RULE_arrowCompoundAssignStmt = 34, RULE_callStmt = 35, 
		RULE_argList = 36, RULE_returnStmt = 37, RULE_ifStmt = 38, RULE_elseIfClause = 39, 
		RULE_elseClause = 40, RULE_forStmt = 41, RULE_breakStmt = 42, RULE_continueStmt = 43, 
		RULE_loopStmt = 44, RULE_whileStmt = 45, RULE_doWhileStmt = 46, RULE_lockStmt = 47, 
		RULE_tryCatchStmt = 48, RULE_catchClause = 49, RULE_finallyClause = 50, 
		RULE_throwStmt = 51, RULE_switchStmt = 52, RULE_caseClause = 53, RULE_defaultClause = 54, 
		RULE_expression = 55, RULE_typeSpec = 56, RULE_fnTypeSpec = 57, RULE_primitiveType = 58;
	private static String[] makeRuleNames() {
		return new String[] {
			"program", "preambleDecl", "namespaceDecl", "useDecl", "modulePath", 
			"includeDecl", "topLevelDecl", "typeAliasDecl", "enumDecl", "structDecl", 
			"structField", "unionDecl", "unionField", "externDecl", "externParamList", 
			"externParam", "functionDecl", "extendDecl", "typeParamList", "typeParam", 
			"extendMethod", "paramList", "param", "block", "statement", "deferStmt", 
			"exprStmt", "varDeclStmt", "assignStmt", "compoundAssignStmt", "fieldAssignStmt", 
			"indexFieldAssignStmt", "derefAssignStmt", "arrowAssignStmt", "arrowCompoundAssignStmt", 
			"callStmt", "argList", "returnStmt", "ifStmt", "elseIfClause", "elseClause", 
			"forStmt", "breakStmt", "continueStmt", "loopStmt", "whileStmt", "doWhileStmt", 
			"lockStmt", "tryCatchStmt", "catchClause", "finallyClause", "throwStmt", 
			"switchStmt", "caseClause", "defaultClause", "expression", "typeSpec", 
			"fnTypeSpec", "primitiveType"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "'namespace'", "'use'", "'ret'", "'struct'", "'union'", "'enum'", 
			"'null'", "'fn'", "'type'", "'as'", "'is'", "'sizeof'", "'typeof'", "'if'", 
			"'else'", "'for'", "'in'", "'loop'", "'while'", "'do'", "'break'", "'continue'", 
			"'switch'", "'case'", "'default'", "'spawn'", "'await'", "'lock'", "'extend'", 
			"'try'", "'catch'", "'finally'", "'throw'", "'defer'", "'extern'", "'auto'", 
			"'vec'", "'map'", "'set'", "'tuple'", "'->'", null, null, "'??'", "'...'", 
			"'..='", "'..'", "'int1'", "'int8'", "'int16'", "'int32'", "'int64'", 
			"'int128'", "'intinf'", "'isize'", "'uint1'", "'uint8'", "'uint16'", 
			"'uint32'", "'uint64'", "'uint128'", "'usize'", "'float32'", "'float64'", 
			"'float80'", "'float128'", "'double'", "'bool'", "'char'", "'void'", 
			"'string'", "'cstring'", null, null, null, null, null, null, null, null, 
			null, null, "'+='", "'-='", "'*='", "'/='", "'%='", "'&='", "'|='", "'^='", 
			"'<<='", "'>>='", "';'", "':'", "'::'", "','", "'.'", "'='", "'('", "')'", 
			"'{'", "'}'", "'['", "']'", "'*'", "'&'", "'-'", "'+'", "'/'", "'%'", 
			"'=='", "'!='", "'<='", "'>='", "'<'", "'>'", "'&&'", "'||'", "'!'", 
			"'++'", "'--'", "'<<'", "'|'", "'^'", "'~'", "'?'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, "NAMESPACE", "USE", "RET", "STRUCT", "UNION", "ENUM", "NULL_LIT", 
			"FN", "TYPE", "AS", "IS", "SIZEOF", "TYPEOF", "IF", "ELSE", "FOR", "IN", 
			"LOOP", "WHILE", "DO", "BREAK", "CONTINUE", "SWITCH", "CASE", "DEFAULT", 
			"SPAWN", "AWAIT", "LOCK", "EXTEND", "TRY", "CATCH", "FINALLY", "THROW", 
			"DEFER", "EXTERN", "AUTO", "VEC", "MAP", "SET", "TUPLE", "ARROW", "INCLUDE_SYS", 
			"INCLUDE_LOCAL", "NULLCOAL", "SPREAD", "RANGE_INCL", "RANGE", "INT1", 
			"INT8", "INT16", "INT32", "INT64", "INT128", "INTINF", "ISIZE", "UINT1", 
			"UINT8", "UINT16", "UINT32", "UINT64", "UINT128", "USIZE", "FLOAT32", 
			"FLOAT64", "FLOAT80", "FLOAT128", "DOUBLE", "BOOL", "CHAR", "VOID", "STRING", 
			"CSTRING", "HEX_LIT", "OCT_LIT", "BIN_LIT", "INT_LIT", "FLOAT_LIT", "BOOL_LIT", 
			"C_STR_LIT", "STR_LIT", "CHAR_LIT", "IDENTIFIER", "PLUS_ASSIGN", "MINUS_ASSIGN", 
			"STAR_ASSIGN", "SLASH_ASSIGN", "PERCENT_ASSIGN", "AMP_ASSIGN", "PIPE_ASSIGN", 
			"CARET_ASSIGN", "LSHIFT_ASSIGN", "RSHIFT_ASSIGN", "SEMI", "COLON", "SCOPE", 
			"COMMA", "DOT", "ASSIGN", "LPAREN", "RPAREN", "LBRACE", "RBRACE", "LBRACKET", 
			"RBRACKET", "STAR", "AMPERSAND", "MINUS", "PLUS", "SLASH", "PERCENT", 
			"EQ", "NEQ", "LTE", "GTE", "LT", "GT", "LAND", "LOR", "NOT", "INCR", 
			"DECR", "LSHIFT", "PIPE", "CARET", "TILDE", "QUESTION", "WS", "LINE_COMMENT", 
			"BLOCK_COMMENT"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}

	@Override
	public String getGrammarFileName() { return "LuxParser.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public ATN getATN() { return _ATN; }

	public LuxParser(TokenStream input) {
		super(input);
		_interp = new ParserATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ProgramContext extends ParserRuleContext {
		public TerminalNode EOF() { return getToken(LuxParser.EOF, 0); }
		public NamespaceDeclContext namespaceDecl() {
			return getRuleContext(NamespaceDeclContext.class,0);
		}
		public List<PreambleDeclContext> preambleDecl() {
			return getRuleContexts(PreambleDeclContext.class);
		}
		public PreambleDeclContext preambleDecl(int i) {
			return getRuleContext(PreambleDeclContext.class,i);
		}
		public List<TopLevelDeclContext> topLevelDecl() {
			return getRuleContexts(TopLevelDeclContext.class);
		}
		public TopLevelDeclContext topLevelDecl(int i) {
			return getRuleContext(TopLevelDeclContext.class,i);
		}
		public ProgramContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_program; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterProgram(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitProgram(this);
		}
	}

	public final ProgramContext program() throws RecognitionException {
		ProgramContext _localctx = new ProgramContext(_ctx, getState());
		enterRule(_localctx, 0, RULE_program);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(119);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==NAMESPACE) {
				{
				setState(118);
				namespaceDecl();
				}
			}

			setState(124);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & 13194139533316L) != 0)) {
				{
				{
				setState(121);
				preambleDecl();
				}
				}
				setState(126);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(130);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279309776321680L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				{
				setState(127);
				topLevelDecl();
				}
				}
				setState(132);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(133);
			match(EOF);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PreambleDeclContext extends ParserRuleContext {
		public UseDeclContext useDecl() {
			return getRuleContext(UseDeclContext.class,0);
		}
		public IncludeDeclContext includeDecl() {
			return getRuleContext(IncludeDeclContext.class,0);
		}
		public PreambleDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_preambleDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPreambleDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPreambleDecl(this);
		}
	}

	public final PreambleDeclContext preambleDecl() throws RecognitionException {
		PreambleDeclContext _localctx = new PreambleDeclContext(_ctx, getState());
		enterRule(_localctx, 2, RULE_preambleDecl);
		try {
			setState(137);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case USE:
				enterOuterAlt(_localctx, 1);
				{
				setState(135);
				useDecl();
				}
				break;
			case INCLUDE_SYS:
			case INCLUDE_LOCAL:
				enterOuterAlt(_localctx, 2);
				{
				setState(136);
				includeDecl();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class NamespaceDeclContext extends ParserRuleContext {
		public TerminalNode NAMESPACE() { return getToken(LuxParser.NAMESPACE, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public NamespaceDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_namespaceDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterNamespaceDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitNamespaceDecl(this);
		}
	}

	public final NamespaceDeclContext namespaceDecl() throws RecognitionException {
		NamespaceDeclContext _localctx = new NamespaceDeclContext(_ctx, getState());
		enterRule(_localctx, 4, RULE_namespaceDecl);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(139);
			match(NAMESPACE);
			setState(140);
			match(IDENTIFIER);
			setState(141);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class UseDeclContext extends ParserRuleContext {
		public UseDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_useDecl; }
	 
		public UseDeclContext() { }
		public void copyFrom(UseDeclContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class UseItemContext extends UseDeclContext {
		public TerminalNode USE() { return getToken(LuxParser.USE, 0); }
		public ModulePathContext modulePath() {
			return getRuleContext(ModulePathContext.class,0);
		}
		public TerminalNode SCOPE() { return getToken(LuxParser.SCOPE, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public UseItemContext(UseDeclContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterUseItem(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitUseItem(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class UseGroupContext extends UseDeclContext {
		public TerminalNode USE() { return getToken(LuxParser.USE, 0); }
		public ModulePathContext modulePath() {
			return getRuleContext(ModulePathContext.class,0);
		}
		public TerminalNode SCOPE() { return getToken(LuxParser.SCOPE, 0); }
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public UseGroupContext(UseDeclContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterUseGroup(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitUseGroup(this);
		}
	}

	public final UseDeclContext useDecl() throws RecognitionException {
		UseDeclContext _localctx = new UseDeclContext(_ctx, getState());
		enterRule(_localctx, 6, RULE_useDecl);
		int _la;
		try {
			setState(164);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,5,_ctx) ) {
			case 1:
				_localctx = new UseItemContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(143);
				match(USE);
				setState(144);
				modulePath();
				setState(145);
				match(SCOPE);
				setState(146);
				match(IDENTIFIER);
				setState(147);
				match(SEMI);
				}
				break;
			case 2:
				_localctx = new UseGroupContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(149);
				match(USE);
				setState(150);
				modulePath();
				setState(151);
				match(SCOPE);
				setState(152);
				match(LBRACE);
				setState(153);
				match(IDENTIFIER);
				setState(158);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(154);
					match(COMMA);
					setState(155);
					match(IDENTIFIER);
					}
					}
					setState(160);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(161);
				match(RBRACE);
				setState(162);
				match(SEMI);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ModulePathContext extends ParserRuleContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public List<TerminalNode> SCOPE() { return getTokens(LuxParser.SCOPE); }
		public TerminalNode SCOPE(int i) {
			return getToken(LuxParser.SCOPE, i);
		}
		public ModulePathContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_modulePath; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterModulePath(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitModulePath(this);
		}
	}

	public final ModulePathContext modulePath() throws RecognitionException {
		ModulePathContext _localctx = new ModulePathContext(_ctx, getState());
		enterRule(_localctx, 8, RULE_modulePath);
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(166);
			match(IDENTIFIER);
			setState(171);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,6,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					{
					{
					setState(167);
					match(SCOPE);
					setState(168);
					match(IDENTIFIER);
					}
					} 
				}
				setState(173);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,6,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class IncludeDeclContext extends ParserRuleContext {
		public TerminalNode INCLUDE_SYS() { return getToken(LuxParser.INCLUDE_SYS, 0); }
		public TerminalNode INCLUDE_LOCAL() { return getToken(LuxParser.INCLUDE_LOCAL, 0); }
		public IncludeDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_includeDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIncludeDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIncludeDecl(this);
		}
	}

	public final IncludeDeclContext includeDecl() throws RecognitionException {
		IncludeDeclContext _localctx = new IncludeDeclContext(_ctx, getState());
		enterRule(_localctx, 10, RULE_includeDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(174);
			_la = _input.LA(1);
			if ( !(_la==INCLUDE_SYS || _la==INCLUDE_LOCAL) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TopLevelDeclContext extends ParserRuleContext {
		public TypeAliasDeclContext typeAliasDecl() {
			return getRuleContext(TypeAliasDeclContext.class,0);
		}
		public StructDeclContext structDecl() {
			return getRuleContext(StructDeclContext.class,0);
		}
		public UnionDeclContext unionDecl() {
			return getRuleContext(UnionDeclContext.class,0);
		}
		public EnumDeclContext enumDecl() {
			return getRuleContext(EnumDeclContext.class,0);
		}
		public ExtendDeclContext extendDecl() {
			return getRuleContext(ExtendDeclContext.class,0);
		}
		public ExternDeclContext externDecl() {
			return getRuleContext(ExternDeclContext.class,0);
		}
		public FunctionDeclContext functionDecl() {
			return getRuleContext(FunctionDeclContext.class,0);
		}
		public TopLevelDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_topLevelDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTopLevelDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTopLevelDecl(this);
		}
	}

	public final TopLevelDeclContext topLevelDecl() throws RecognitionException {
		TopLevelDeclContext _localctx = new TopLevelDeclContext(_ctx, getState());
		enterRule(_localctx, 12, RULE_topLevelDecl);
		try {
			setState(183);
			_errHandler.sync(this);
			switch (_input.LA(1)) {
			case TYPE:
				enterOuterAlt(_localctx, 1);
				{
				setState(176);
				typeAliasDecl();
				}
				break;
			case STRUCT:
				enterOuterAlt(_localctx, 2);
				{
				setState(177);
				structDecl();
				}
				break;
			case UNION:
				enterOuterAlt(_localctx, 3);
				{
				setState(178);
				unionDecl();
				}
				break;
			case ENUM:
				enterOuterAlt(_localctx, 4);
				{
				setState(179);
				enumDecl();
				}
				break;
			case EXTEND:
				enterOuterAlt(_localctx, 5);
				{
				setState(180);
				extendDecl();
				}
				break;
			case EXTERN:
				enterOuterAlt(_localctx, 6);
				{
				setState(181);
				externDecl();
				}
				break;
			case FN:
			case AUTO:
			case VEC:
			case MAP:
			case SET:
			case TUPLE:
			case INT1:
			case INT8:
			case INT16:
			case INT32:
			case INT64:
			case INT128:
			case INTINF:
			case ISIZE:
			case UINT1:
			case UINT8:
			case UINT16:
			case UINT32:
			case UINT64:
			case UINT128:
			case USIZE:
			case FLOAT32:
			case FLOAT64:
			case FLOAT80:
			case FLOAT128:
			case DOUBLE:
			case BOOL:
			case CHAR:
			case VOID:
			case STRING:
			case CSTRING:
			case IDENTIFIER:
			case LBRACKET:
			case STAR:
				enterOuterAlt(_localctx, 7);
				{
				setState(182);
				functionDecl();
				}
				break;
			default:
				throw new NoViableAltException(this);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TypeAliasDeclContext extends ParserRuleContext {
		public TerminalNode TYPE() { return getToken(LuxParser.TYPE, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public TypeAliasDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_typeAliasDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTypeAliasDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTypeAliasDecl(this);
		}
	}

	public final TypeAliasDeclContext typeAliasDecl() throws RecognitionException {
		TypeAliasDeclContext _localctx = new TypeAliasDeclContext(_ctx, getState());
		enterRule(_localctx, 14, RULE_typeAliasDecl);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(185);
			match(TYPE);
			setState(186);
			match(IDENTIFIER);
			setState(187);
			match(ASSIGN);
			setState(188);
			typeSpec(0);
			setState(189);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class EnumDeclContext extends ParserRuleContext {
		public TerminalNode ENUM() { return getToken(LuxParser.ENUM, 0); }
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public EnumDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_enumDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterEnumDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitEnumDecl(this);
		}
	}

	public final EnumDeclContext enumDecl() throws RecognitionException {
		EnumDeclContext _localctx = new EnumDeclContext(_ctx, getState());
		enterRule(_localctx, 16, RULE_enumDecl);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(191);
			match(ENUM);
			setState(192);
			match(IDENTIFIER);
			setState(193);
			match(LBRACE);
			setState(194);
			match(IDENTIFIER);
			setState(199);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,8,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					{
					{
					setState(195);
					match(COMMA);
					setState(196);
					match(IDENTIFIER);
					}
					} 
				}
				setState(201);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,8,_ctx);
			}
			setState(203);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==COMMA) {
				{
				setState(202);
				match(COMMA);
				}
			}

			setState(205);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class StructDeclContext extends ParserRuleContext {
		public TerminalNode STRUCT() { return getToken(LuxParser.STRUCT, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public TypeParamListContext typeParamList() {
			return getRuleContext(TypeParamListContext.class,0);
		}
		public List<StructFieldContext> structField() {
			return getRuleContexts(StructFieldContext.class);
		}
		public StructFieldContext structField(int i) {
			return getRuleContext(StructFieldContext.class,i);
		}
		public StructDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_structDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStructDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStructDecl(this);
		}
	}

	public final StructDeclContext structDecl() throws RecognitionException {
		StructDeclContext _localctx = new StructDeclContext(_ctx, getState());
		enterRule(_localctx, 18, RULE_structDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(207);
			match(STRUCT);
			setState(208);
			match(IDENTIFIER);
			setState(210);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==LT) {
				{
				setState(209);
				typeParamList();
				}
			}

			setState(212);
			match(LBRACE);
			setState(216);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				{
				setState(213);
				structField();
				}
				}
				setState(218);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(219);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class StructFieldContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public StructFieldContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_structField; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStructField(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStructField(this);
		}
	}

	public final StructFieldContext structField() throws RecognitionException {
		StructFieldContext _localctx = new StructFieldContext(_ctx, getState());
		enterRule(_localctx, 20, RULE_structField);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(221);
			typeSpec(0);
			setState(222);
			match(IDENTIFIER);
			setState(223);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class UnionDeclContext extends ParserRuleContext {
		public TerminalNode UNION() { return getToken(LuxParser.UNION, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public List<UnionFieldContext> unionField() {
			return getRuleContexts(UnionFieldContext.class);
		}
		public UnionFieldContext unionField(int i) {
			return getRuleContext(UnionFieldContext.class,i);
		}
		public UnionDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_unionDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterUnionDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitUnionDecl(this);
		}
	}

	public final UnionDeclContext unionDecl() throws RecognitionException {
		UnionDeclContext _localctx = new UnionDeclContext(_ctx, getState());
		enterRule(_localctx, 22, RULE_unionDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(225);
			match(UNION);
			setState(226);
			match(IDENTIFIER);
			setState(227);
			match(LBRACE);
			setState(231);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				{
				setState(228);
				unionField();
				}
				}
				setState(233);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(234);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class UnionFieldContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public UnionFieldContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_unionField; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterUnionField(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitUnionField(this);
		}
	}

	public final UnionFieldContext unionField() throws RecognitionException {
		UnionFieldContext _localctx = new UnionFieldContext(_ctx, getState());
		enterRule(_localctx, 24, RULE_unionField);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(236);
			typeSpec(0);
			setState(237);
			match(IDENTIFIER);
			setState(238);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExternDeclContext extends ParserRuleContext {
		public TerminalNode EXTERN() { return getToken(LuxParser.EXTERN, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ExternParamListContext externParamList() {
			return getRuleContext(ExternParamListContext.class,0);
		}
		public TerminalNode COMMA() { return getToken(LuxParser.COMMA, 0); }
		public TerminalNode SPREAD() { return getToken(LuxParser.SPREAD, 0); }
		public ExternDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_externDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExternDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExternDecl(this);
		}
	}

	public final ExternDeclContext externDecl() throws RecognitionException {
		ExternDeclContext _localctx = new ExternDeclContext(_ctx, getState());
		enterRule(_localctx, 26, RULE_externDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(240);
			match(EXTERN);
			setState(241);
			typeSpec(0);
			setState(242);
			match(IDENTIFIER);
			setState(243);
			match(LPAREN);
			setState(245);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				setState(244);
				externParamList();
				}
			}

			setState(249);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==COMMA) {
				{
				setState(247);
				match(COMMA);
				setState(248);
				match(SPREAD);
				}
			}

			setState(251);
			match(RPAREN);
			setState(252);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExternParamListContext extends ParserRuleContext {
		public List<ExternParamContext> externParam() {
			return getRuleContexts(ExternParamContext.class);
		}
		public ExternParamContext externParam(int i) {
			return getRuleContext(ExternParamContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public ExternParamListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_externParamList; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExternParamList(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExternParamList(this);
		}
	}

	public final ExternParamListContext externParamList() throws RecognitionException {
		ExternParamListContext _localctx = new ExternParamListContext(_ctx, getState());
		enterRule(_localctx, 28, RULE_externParamList);
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(254);
			externParam();
			setState(259);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,15,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					{
					{
					setState(255);
					match(COMMA);
					setState(256);
					externParam();
					}
					} 
				}
				setState(261);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,15,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExternParamContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public ExternParamContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_externParam; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExternParam(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExternParam(this);
		}
	}

	public final ExternParamContext externParam() throws RecognitionException {
		ExternParamContext _localctx = new ExternParamContext(_ctx, getState());
		enterRule(_localctx, 30, RULE_externParam);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(262);
			typeSpec(0);
			setState(264);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==IDENTIFIER) {
				{
				setState(263);
				match(IDENTIFIER);
				}
			}

			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FunctionDeclContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public TypeParamListContext typeParamList() {
			return getRuleContext(TypeParamListContext.class,0);
		}
		public ParamListContext paramList() {
			return getRuleContext(ParamListContext.class,0);
		}
		public FunctionDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_functionDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFunctionDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFunctionDecl(this);
		}
	}

	public final FunctionDeclContext functionDecl() throws RecognitionException {
		FunctionDeclContext _localctx = new FunctionDeclContext(_ctx, getState());
		enterRule(_localctx, 32, RULE_functionDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(266);
			typeSpec(0);
			setState(267);
			match(IDENTIFIER);
			setState(269);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==LT) {
				{
				setState(268);
				typeParamList();
				}
			}

			setState(271);
			match(LPAREN);
			setState(273);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				setState(272);
				paramList();
				}
			}

			setState(275);
			match(RPAREN);
			setState(276);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExtendDeclContext extends ParserRuleContext {
		public TerminalNode EXTEND() { return getToken(LuxParser.EXTEND, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public TypeParamListContext typeParamList() {
			return getRuleContext(TypeParamListContext.class,0);
		}
		public List<ExtendMethodContext> extendMethod() {
			return getRuleContexts(ExtendMethodContext.class);
		}
		public ExtendMethodContext extendMethod(int i) {
			return getRuleContext(ExtendMethodContext.class,i);
		}
		public ExtendDeclContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_extendDecl; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExtendDecl(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExtendDecl(this);
		}
	}

	public final ExtendDeclContext extendDecl() throws RecognitionException {
		ExtendDeclContext _localctx = new ExtendDeclContext(_ctx, getState());
		enterRule(_localctx, 34, RULE_extendDecl);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(278);
			match(EXTEND);
			setState(279);
			match(IDENTIFIER);
			setState(281);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==LT) {
				{
				setState(280);
				typeParamList();
				}
			}

			setState(283);
			match(LBRACE);
			setState(287);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				{
				setState(284);
				extendMethod();
				}
				}
				setState(289);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(290);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TypeParamListContext extends ParserRuleContext {
		public TerminalNode LT() { return getToken(LuxParser.LT, 0); }
		public List<TypeParamContext> typeParam() {
			return getRuleContexts(TypeParamContext.class);
		}
		public TypeParamContext typeParam(int i) {
			return getRuleContext(TypeParamContext.class,i);
		}
		public TerminalNode GT() { return getToken(LuxParser.GT, 0); }
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public TypeParamListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_typeParamList; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTypeParamList(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTypeParamList(this);
		}
	}

	public final TypeParamListContext typeParamList() throws RecognitionException {
		TypeParamListContext _localctx = new TypeParamListContext(_ctx, getState());
		enterRule(_localctx, 36, RULE_typeParamList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(292);
			match(LT);
			setState(293);
			typeParam();
			setState(298);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(294);
				match(COMMA);
				setState(295);
				typeParam();
				}
				}
				setState(300);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(301);
			match(GT);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TypeParamContext extends ParserRuleContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode COLON() { return getToken(LuxParser.COLON, 0); }
		public TypeParamContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_typeParam; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTypeParam(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTypeParam(this);
		}
	}

	public final TypeParamContext typeParam() throws RecognitionException {
		TypeParamContext _localctx = new TypeParamContext(_ctx, getState());
		enterRule(_localctx, 38, RULE_typeParam);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(303);
			match(IDENTIFIER);
			setState(306);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if (_la==COLON) {
				{
				setState(304);
				match(COLON);
				setState(305);
				match(IDENTIFIER);
				}
			}

			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExtendMethodContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode AMPERSAND() { return getToken(LuxParser.AMPERSAND, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public List<ParamContext> param() {
			return getRuleContexts(ParamContext.class);
		}
		public ParamContext param(int i) {
			return getRuleContext(ParamContext.class,i);
		}
		public ParamListContext paramList() {
			return getRuleContext(ParamListContext.class,0);
		}
		public ExtendMethodContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_extendMethod; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExtendMethod(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExtendMethod(this);
		}
	}

	public final ExtendMethodContext extendMethod() throws RecognitionException {
		ExtendMethodContext _localctx = new ExtendMethodContext(_ctx, getState());
		enterRule(_localctx, 40, RULE_extendMethod);
		int _la;
		try {
			setState(332);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,25,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(308);
				typeSpec(0);
				setState(309);
				match(IDENTIFIER);
				setState(310);
				match(LPAREN);
				setState(311);
				match(AMPERSAND);
				setState(312);
				match(IDENTIFIER);
				setState(317);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(313);
					match(COMMA);
					setState(314);
					param();
					}
					}
					setState(319);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(320);
				match(RPAREN);
				setState(321);
				block();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(323);
				typeSpec(0);
				setState(324);
				match(IDENTIFIER);
				setState(325);
				match(LPAREN);
				setState(327);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
					{
					setState(326);
					paramList();
					}
				}

				setState(329);
				match(RPAREN);
				setState(330);
				block();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParamListContext extends ParserRuleContext {
		public List<ParamContext> param() {
			return getRuleContexts(ParamContext.class);
		}
		public ParamContext param(int i) {
			return getRuleContext(ParamContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public ParamListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_paramList; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterParamList(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitParamList(this);
		}
	}

	public final ParamListContext paramList() throws RecognitionException {
		ParamListContext _localctx = new ParamListContext(_ctx, getState());
		enterRule(_localctx, 42, RULE_paramList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(334);
			param();
			setState(339);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(335);
				match(COMMA);
				setState(336);
				param();
				}
				}
				setState(341);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ParamContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode SPREAD() { return getToken(LuxParser.SPREAD, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public ParamContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_param; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterParam(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitParam(this);
		}
	}

	public final ParamContext param() throws RecognitionException {
		ParamContext _localctx = new ParamContext(_ctx, getState());
		enterRule(_localctx, 44, RULE_param);
		try {
			setState(349);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,27,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(342);
				typeSpec(0);
				setState(343);
				match(SPREAD);
				setState(344);
				match(IDENTIFIER);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(346);
				typeSpec(0);
				setState(347);
				match(IDENTIFIER);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class BlockContext extends ParserRuleContext {
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public List<StatementContext> statement() {
			return getRuleContexts(StatementContext.class);
		}
		public StatementContext statement(int i) {
			return getRuleContext(StatementContext.class,i);
		}
		public BlockContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_block; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBlock(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBlock(this);
		}
	}

	public final BlockContext block() throws RecognitionException {
		BlockContext _localctx = new BlockContext(_ctx, getState());
		enterRule(_localctx, 46, RULE_block);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(351);
			match(LBRACE);
			setState(355);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while ((((_la) & ~0x3f) == 0 && ((1L << _la) & -244132970925688L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2558060565625307135L) != 0)) {
				{
				{
				setState(352);
				statement();
				}
				}
				setState(357);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(358);
			match(RBRACE);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class StatementContext extends ParserRuleContext {
		public VarDeclStmtContext varDeclStmt() {
			return getRuleContext(VarDeclStmtContext.class,0);
		}
		public AssignStmtContext assignStmt() {
			return getRuleContext(AssignStmtContext.class,0);
		}
		public CompoundAssignStmtContext compoundAssignStmt() {
			return getRuleContext(CompoundAssignStmtContext.class,0);
		}
		public DerefAssignStmtContext derefAssignStmt() {
			return getRuleContext(DerefAssignStmtContext.class,0);
		}
		public IndexFieldAssignStmtContext indexFieldAssignStmt() {
			return getRuleContext(IndexFieldAssignStmtContext.class,0);
		}
		public FieldAssignStmtContext fieldAssignStmt() {
			return getRuleContext(FieldAssignStmtContext.class,0);
		}
		public ArrowAssignStmtContext arrowAssignStmt() {
			return getRuleContext(ArrowAssignStmtContext.class,0);
		}
		public ArrowCompoundAssignStmtContext arrowCompoundAssignStmt() {
			return getRuleContext(ArrowCompoundAssignStmtContext.class,0);
		}
		public CallStmtContext callStmt() {
			return getRuleContext(CallStmtContext.class,0);
		}
		public ExprStmtContext exprStmt() {
			return getRuleContext(ExprStmtContext.class,0);
		}
		public ReturnStmtContext returnStmt() {
			return getRuleContext(ReturnStmtContext.class,0);
		}
		public IfStmtContext ifStmt() {
			return getRuleContext(IfStmtContext.class,0);
		}
		public ForStmtContext forStmt() {
			return getRuleContext(ForStmtContext.class,0);
		}
		public LoopStmtContext loopStmt() {
			return getRuleContext(LoopStmtContext.class,0);
		}
		public WhileStmtContext whileStmt() {
			return getRuleContext(WhileStmtContext.class,0);
		}
		public DoWhileStmtContext doWhileStmt() {
			return getRuleContext(DoWhileStmtContext.class,0);
		}
		public BreakStmtContext breakStmt() {
			return getRuleContext(BreakStmtContext.class,0);
		}
		public ContinueStmtContext continueStmt() {
			return getRuleContext(ContinueStmtContext.class,0);
		}
		public SwitchStmtContext switchStmt() {
			return getRuleContext(SwitchStmtContext.class,0);
		}
		public LockStmtContext lockStmt() {
			return getRuleContext(LockStmtContext.class,0);
		}
		public TryCatchStmtContext tryCatchStmt() {
			return getRuleContext(TryCatchStmtContext.class,0);
		}
		public ThrowStmtContext throwStmt() {
			return getRuleContext(ThrowStmtContext.class,0);
		}
		public DeferStmtContext deferStmt() {
			return getRuleContext(DeferStmtContext.class,0);
		}
		public StatementContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_statement; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStatement(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStatement(this);
		}
	}

	public final StatementContext statement() throws RecognitionException {
		StatementContext _localctx = new StatementContext(_ctx, getState());
		enterRule(_localctx, 48, RULE_statement);
		try {
			setState(383);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,29,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(360);
				varDeclStmt();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(361);
				assignStmt();
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(362);
				compoundAssignStmt();
				}
				break;
			case 4:
				enterOuterAlt(_localctx, 4);
				{
				setState(363);
				derefAssignStmt();
				}
				break;
			case 5:
				enterOuterAlt(_localctx, 5);
				{
				setState(364);
				indexFieldAssignStmt();
				}
				break;
			case 6:
				enterOuterAlt(_localctx, 6);
				{
				setState(365);
				fieldAssignStmt();
				}
				break;
			case 7:
				enterOuterAlt(_localctx, 7);
				{
				setState(366);
				arrowAssignStmt();
				}
				break;
			case 8:
				enterOuterAlt(_localctx, 8);
				{
				setState(367);
				arrowCompoundAssignStmt();
				}
				break;
			case 9:
				enterOuterAlt(_localctx, 9);
				{
				setState(368);
				callStmt();
				}
				break;
			case 10:
				enterOuterAlt(_localctx, 10);
				{
				setState(369);
				exprStmt();
				}
				break;
			case 11:
				enterOuterAlt(_localctx, 11);
				{
				setState(370);
				returnStmt();
				}
				break;
			case 12:
				enterOuterAlt(_localctx, 12);
				{
				setState(371);
				ifStmt();
				}
				break;
			case 13:
				enterOuterAlt(_localctx, 13);
				{
				setState(372);
				forStmt();
				}
				break;
			case 14:
				enterOuterAlt(_localctx, 14);
				{
				setState(373);
				loopStmt();
				}
				break;
			case 15:
				enterOuterAlt(_localctx, 15);
				{
				setState(374);
				whileStmt();
				}
				break;
			case 16:
				enterOuterAlt(_localctx, 16);
				{
				setState(375);
				doWhileStmt();
				}
				break;
			case 17:
				enterOuterAlt(_localctx, 17);
				{
				setState(376);
				breakStmt();
				}
				break;
			case 18:
				enterOuterAlt(_localctx, 18);
				{
				setState(377);
				continueStmt();
				}
				break;
			case 19:
				enterOuterAlt(_localctx, 19);
				{
				setState(378);
				switchStmt();
				}
				break;
			case 20:
				enterOuterAlt(_localctx, 20);
				{
				setState(379);
				lockStmt();
				}
				break;
			case 21:
				enterOuterAlt(_localctx, 21);
				{
				setState(380);
				tryCatchStmt();
				}
				break;
			case 22:
				enterOuterAlt(_localctx, 22);
				{
				setState(381);
				throwStmt();
				}
				break;
			case 23:
				enterOuterAlt(_localctx, 23);
				{
				setState(382);
				deferStmt();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class DeferStmtContext extends ParserRuleContext {
		public TerminalNode DEFER() { return getToken(LuxParser.DEFER, 0); }
		public CallStmtContext callStmt() {
			return getRuleContext(CallStmtContext.class,0);
		}
		public ExprStmtContext exprStmt() {
			return getRuleContext(ExprStmtContext.class,0);
		}
		public DeferStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_deferStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterDeferStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitDeferStmt(this);
		}
	}

	public final DeferStmtContext deferStmt() throws RecognitionException {
		DeferStmtContext _localctx = new DeferStmtContext(_ctx, getState());
		enterRule(_localctx, 50, RULE_deferStmt);
		try {
			setState(389);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,30,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(385);
				match(DEFER);
				setState(386);
				callStmt();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(387);
				match(DEFER);
				setState(388);
				exprStmt();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExprStmtContext extends ParserRuleContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ExprStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_exprStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterExprStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitExprStmt(this);
		}
	}

	public final ExprStmtContext exprStmt() throws RecognitionException {
		ExprStmtContext _localctx = new ExprStmtContext(_ctx, getState());
		enterRule(_localctx, 52, RULE_exprStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(391);
			expression(0);
			setState(392);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class VarDeclStmtContext extends ParserRuleContext {
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public VarDeclStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_varDeclStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterVarDeclStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitVarDeclStmt(this);
		}
	}

	public final VarDeclStmtContext varDeclStmt() throws RecognitionException {
		VarDeclStmtContext _localctx = new VarDeclStmtContext(_ctx, getState());
		enterRule(_localctx, 54, RULE_varDeclStmt);
		int _la;
		try {
			setState(419);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,32,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(394);
				typeSpec(0);
				setState(395);
				match(LPAREN);
				setState(396);
				match(IDENTIFIER);
				setState(401);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(397);
					match(COMMA);
					setState(398);
					match(IDENTIFIER);
					}
					}
					setState(403);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(404);
				match(RPAREN);
				setState(405);
				match(ASSIGN);
				setState(406);
				expression(0);
				setState(407);
				match(SEMI);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(409);
				typeSpec(0);
				setState(410);
				match(IDENTIFIER);
				setState(411);
				match(ASSIGN);
				setState(412);
				expression(0);
				setState(413);
				match(SEMI);
				}
				break;
			case 3:
				enterOuterAlt(_localctx, 3);
				{
				setState(415);
				typeSpec(0);
				setState(416);
				match(IDENTIFIER);
				setState(417);
				match(SEMI);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class AssignStmtContext extends ParserRuleContext {
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public List<TerminalNode> LBRACKET() { return getTokens(LuxParser.LBRACKET); }
		public TerminalNode LBRACKET(int i) {
			return getToken(LuxParser.LBRACKET, i);
		}
		public List<TerminalNode> RBRACKET() { return getTokens(LuxParser.RBRACKET); }
		public TerminalNode RBRACKET(int i) {
			return getToken(LuxParser.RBRACKET, i);
		}
		public AssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_assignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitAssignStmt(this);
		}
	}

	public final AssignStmtContext assignStmt() throws RecognitionException {
		AssignStmtContext _localctx = new AssignStmtContext(_ctx, getState());
		enterRule(_localctx, 56, RULE_assignStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(421);
			match(IDENTIFIER);
			setState(428);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==LBRACKET) {
				{
				{
				setState(422);
				match(LBRACKET);
				setState(423);
				expression(0);
				setState(424);
				match(RBRACKET);
				}
				}
				setState(430);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(431);
			match(ASSIGN);
			setState(432);
			expression(0);
			setState(433);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class CompoundAssignStmtContext extends ParserRuleContext {
		public Token op;
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public TerminalNode PLUS_ASSIGN() { return getToken(LuxParser.PLUS_ASSIGN, 0); }
		public TerminalNode MINUS_ASSIGN() { return getToken(LuxParser.MINUS_ASSIGN, 0); }
		public TerminalNode STAR_ASSIGN() { return getToken(LuxParser.STAR_ASSIGN, 0); }
		public TerminalNode SLASH_ASSIGN() { return getToken(LuxParser.SLASH_ASSIGN, 0); }
		public TerminalNode PERCENT_ASSIGN() { return getToken(LuxParser.PERCENT_ASSIGN, 0); }
		public TerminalNode AMP_ASSIGN() { return getToken(LuxParser.AMP_ASSIGN, 0); }
		public TerminalNode PIPE_ASSIGN() { return getToken(LuxParser.PIPE_ASSIGN, 0); }
		public TerminalNode CARET_ASSIGN() { return getToken(LuxParser.CARET_ASSIGN, 0); }
		public TerminalNode LSHIFT_ASSIGN() { return getToken(LuxParser.LSHIFT_ASSIGN, 0); }
		public TerminalNode RSHIFT_ASSIGN() { return getToken(LuxParser.RSHIFT_ASSIGN, 0); }
		public CompoundAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_compoundAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCompoundAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCompoundAssignStmt(this);
		}
	}

	public final CompoundAssignStmtContext compoundAssignStmt() throws RecognitionException {
		CompoundAssignStmtContext _localctx = new CompoundAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 58, RULE_compoundAssignStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(435);
			match(IDENTIFIER);
			setState(436);
			((CompoundAssignStmtContext)_localctx).op = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 83)) & ~0x3f) == 0 && ((1L << (_la - 83)) & 1023L) != 0)) ) {
				((CompoundAssignStmtContext)_localctx).op = (Token)_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			setState(437);
			expression(0);
			setState(438);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FieldAssignStmtContext extends ParserRuleContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public List<TerminalNode> DOT() { return getTokens(LuxParser.DOT); }
		public TerminalNode DOT(int i) {
			return getToken(LuxParser.DOT, i);
		}
		public FieldAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_fieldAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFieldAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFieldAssignStmt(this);
		}
	}

	public final FieldAssignStmtContext fieldAssignStmt() throws RecognitionException {
		FieldAssignStmtContext _localctx = new FieldAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 60, RULE_fieldAssignStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(440);
			match(IDENTIFIER);
			setState(443); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(441);
				match(DOT);
				setState(442);
				match(IDENTIFIER);
				}
				}
				setState(445); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==DOT );
			setState(447);
			match(ASSIGN);
			setState(448);
			expression(0);
			setState(449);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class IndexFieldAssignStmtContext extends ParserRuleContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public List<TerminalNode> LBRACKET() { return getTokens(LuxParser.LBRACKET); }
		public TerminalNode LBRACKET(int i) {
			return getToken(LuxParser.LBRACKET, i);
		}
		public List<TerminalNode> RBRACKET() { return getTokens(LuxParser.RBRACKET); }
		public TerminalNode RBRACKET(int i) {
			return getToken(LuxParser.RBRACKET, i);
		}
		public List<TerminalNode> DOT() { return getTokens(LuxParser.DOT); }
		public TerminalNode DOT(int i) {
			return getToken(LuxParser.DOT, i);
		}
		public IndexFieldAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_indexFieldAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIndexFieldAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIndexFieldAssignStmt(this);
		}
	}

	public final IndexFieldAssignStmtContext indexFieldAssignStmt() throws RecognitionException {
		IndexFieldAssignStmtContext _localctx = new IndexFieldAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 62, RULE_indexFieldAssignStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(451);
			match(IDENTIFIER);
			setState(456); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(452);
				match(LBRACKET);
				setState(453);
				expression(0);
				setState(454);
				match(RBRACKET);
				}
				}
				setState(458); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==LBRACKET );
			setState(462); 
			_errHandler.sync(this);
			_la = _input.LA(1);
			do {
				{
				{
				setState(460);
				match(DOT);
				setState(461);
				match(IDENTIFIER);
				}
				}
				setState(464); 
				_errHandler.sync(this);
				_la = _input.LA(1);
			} while ( _la==DOT );
			setState(466);
			match(ASSIGN);
			setState(467);
			expression(0);
			setState(468);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class DerefAssignStmtContext extends ParserRuleContext {
		public TerminalNode STAR() { return getToken(LuxParser.STAR, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public DerefAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_derefAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterDerefAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitDerefAssignStmt(this);
		}
	}

	public final DerefAssignStmtContext derefAssignStmt() throws RecognitionException {
		DerefAssignStmtContext _localctx = new DerefAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 64, RULE_derefAssignStmt);
		try {
			setState(484);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,37,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(470);
				match(STAR);
				setState(471);
				match(IDENTIFIER);
				setState(472);
				match(ASSIGN);
				setState(473);
				expression(0);
				setState(474);
				match(SEMI);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(476);
				match(STAR);
				setState(477);
				match(LPAREN);
				setState(478);
				expression(0);
				setState(479);
				match(RPAREN);
				setState(480);
				match(ASSIGN);
				setState(481);
				expression(0);
				setState(482);
				match(SEMI);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArrowAssignStmtContext extends ParserRuleContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ArrowAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrowAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArrowAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArrowAssignStmt(this);
		}
	}

	public final ArrowAssignStmtContext arrowAssignStmt() throws RecognitionException {
		ArrowAssignStmtContext _localctx = new ArrowAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 66, RULE_arrowAssignStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(486);
			match(IDENTIFIER);
			setState(487);
			match(ARROW);
			setState(488);
			match(IDENTIFIER);
			setState(489);
			match(ASSIGN);
			setState(490);
			expression(0);
			setState(491);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArrowCompoundAssignStmtContext extends ParserRuleContext {
		public Token op;
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public TerminalNode PLUS_ASSIGN() { return getToken(LuxParser.PLUS_ASSIGN, 0); }
		public TerminalNode MINUS_ASSIGN() { return getToken(LuxParser.MINUS_ASSIGN, 0); }
		public TerminalNode STAR_ASSIGN() { return getToken(LuxParser.STAR_ASSIGN, 0); }
		public TerminalNode SLASH_ASSIGN() { return getToken(LuxParser.SLASH_ASSIGN, 0); }
		public TerminalNode PERCENT_ASSIGN() { return getToken(LuxParser.PERCENT_ASSIGN, 0); }
		public TerminalNode AMP_ASSIGN() { return getToken(LuxParser.AMP_ASSIGN, 0); }
		public TerminalNode PIPE_ASSIGN() { return getToken(LuxParser.PIPE_ASSIGN, 0); }
		public TerminalNode CARET_ASSIGN() { return getToken(LuxParser.CARET_ASSIGN, 0); }
		public TerminalNode LSHIFT_ASSIGN() { return getToken(LuxParser.LSHIFT_ASSIGN, 0); }
		public TerminalNode RSHIFT_ASSIGN() { return getToken(LuxParser.RSHIFT_ASSIGN, 0); }
		public ArrowCompoundAssignStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_arrowCompoundAssignStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArrowCompoundAssignStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArrowCompoundAssignStmt(this);
		}
	}

	public final ArrowCompoundAssignStmtContext arrowCompoundAssignStmt() throws RecognitionException {
		ArrowCompoundAssignStmtContext _localctx = new ArrowCompoundAssignStmtContext(_ctx, getState());
		enterRule(_localctx, 68, RULE_arrowCompoundAssignStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(493);
			match(IDENTIFIER);
			setState(494);
			match(ARROW);
			setState(495);
			match(IDENTIFIER);
			setState(496);
			((ArrowCompoundAssignStmtContext)_localctx).op = _input.LT(1);
			_la = _input.LA(1);
			if ( !(((((_la - 83)) & ~0x3f) == 0 && ((1L << (_la - 83)) & 1023L) != 0)) ) {
				((ArrowCompoundAssignStmtContext)_localctx).op = (Token)_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			setState(497);
			expression(0);
			setState(498);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class CallStmtContext extends ParserRuleContext {
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ArgListContext argList() {
			return getRuleContext(ArgListContext.class,0);
		}
		public CallStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_callStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCallStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCallStmt(this);
		}
	}

	public final CallStmtContext callStmt() throws RecognitionException {
		CallStmtContext _localctx = new CallStmtContext(_ctx, getState());
		enterRule(_localctx, 70, RULE_callStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(500);
			match(IDENTIFIER);
			setState(501);
			match(LPAREN);
			setState(503);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
				{
				setState(502);
				argList();
				}
			}

			setState(505);
			match(RPAREN);
			setState(506);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ArgListContext extends ParserRuleContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public ArgListContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_argList; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArgList(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArgList(this);
		}
	}

	public final ArgListContext argList() throws RecognitionException {
		ArgListContext _localctx = new ArgListContext(_ctx, getState());
		enterRule(_localctx, 72, RULE_argList);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(508);
			expression(0);
			setState(513);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(509);
				match(COMMA);
				setState(510);
				expression(0);
				}
				}
				setState(515);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ReturnStmtContext extends ParserRuleContext {
		public TerminalNode RET() { return getToken(LuxParser.RET, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public ReturnStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_returnStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterReturnStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitReturnStmt(this);
		}
	}

	public final ReturnStmtContext returnStmt() throws RecognitionException {
		ReturnStmtContext _localctx = new ReturnStmtContext(_ctx, getState());
		enterRule(_localctx, 74, RULE_returnStmt);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(516);
			match(RET);
			setState(518);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
				{
				setState(517);
				expression(0);
				}
			}

			setState(520);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class IfStmtContext extends ParserRuleContext {
		public TerminalNode IF() { return getToken(LuxParser.IF, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public List<ElseIfClauseContext> elseIfClause() {
			return getRuleContexts(ElseIfClauseContext.class);
		}
		public ElseIfClauseContext elseIfClause(int i) {
			return getRuleContext(ElseIfClauseContext.class,i);
		}
		public ElseClauseContext elseClause() {
			return getRuleContext(ElseClauseContext.class,0);
		}
		public IfStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_ifStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIfStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIfStmt(this);
		}
	}

	public final IfStmtContext ifStmt() throws RecognitionException {
		IfStmtContext _localctx = new IfStmtContext(_ctx, getState());
		enterRule(_localctx, 76, RULE_ifStmt);
		int _la;
		try {
			int _alt;
			setState(548);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,45,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(522);
				match(IF);
				setState(523);
				match(LPAREN);
				setState(524);
				expression(0);
				setState(525);
				match(RPAREN);
				setState(526);
				block();
				setState(530);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,41,_ctx);
				while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
					if ( _alt==1 ) {
						{
						{
						setState(527);
						elseIfClause();
						}
						} 
					}
					setState(532);
					_errHandler.sync(this);
					_alt = getInterpreter().adaptivePredict(_input,41,_ctx);
				}
				setState(534);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==ELSE) {
					{
					setState(533);
					elseClause();
					}
				}

				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(536);
				match(IF);
				setState(537);
				expression(0);
				setState(538);
				block();
				setState(542);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,43,_ctx);
				while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
					if ( _alt==1 ) {
						{
						{
						setState(539);
						elseIfClause();
						}
						} 
					}
					setState(544);
					_errHandler.sync(this);
					_alt = getInterpreter().adaptivePredict(_input,43,_ctx);
				}
				setState(546);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==ELSE) {
					{
					setState(545);
					elseClause();
					}
				}

				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ElseIfClauseContext extends ParserRuleContext {
		public TerminalNode ELSE() { return getToken(LuxParser.ELSE, 0); }
		public TerminalNode IF() { return getToken(LuxParser.IF, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public ElseIfClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_elseIfClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterElseIfClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitElseIfClause(this);
		}
	}

	public final ElseIfClauseContext elseIfClause() throws RecognitionException {
		ElseIfClauseContext _localctx = new ElseIfClauseContext(_ctx, getState());
		enterRule(_localctx, 78, RULE_elseIfClause);
		try {
			setState(562);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,46,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(550);
				match(ELSE);
				setState(551);
				match(IF);
				setState(552);
				match(LPAREN);
				setState(553);
				expression(0);
				setState(554);
				match(RPAREN);
				setState(555);
				block();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(557);
				match(ELSE);
				setState(558);
				match(IF);
				setState(559);
				expression(0);
				setState(560);
				block();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ElseClauseContext extends ParserRuleContext {
		public TerminalNode ELSE() { return getToken(LuxParser.ELSE, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public ElseClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_elseClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterElseClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitElseClause(this);
		}
	}

	public final ElseClauseContext elseClause() throws RecognitionException {
		ElseClauseContext _localctx = new ElseClauseContext(_ctx, getState());
		enterRule(_localctx, 80, RULE_elseClause);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(564);
			match(ELSE);
			setState(565);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ForStmtContext extends ParserRuleContext {
		public ForStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_forStmt; }
	 
		public ForStmtContext() { }
		public void copyFrom(ForStmtContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ForInStmtContext extends ForStmtContext {
		public TerminalNode FOR() { return getToken(LuxParser.FOR, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode IN() { return getToken(LuxParser.IN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public ForInStmtContext(ForStmtContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterForInStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitForInStmt(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ForClassicStmtContext extends ForStmtContext {
		public TerminalNode FOR() { return getToken(LuxParser.FOR, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode ASSIGN() { return getToken(LuxParser.ASSIGN, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> SEMI() { return getTokens(LuxParser.SEMI); }
		public TerminalNode SEMI(int i) {
			return getToken(LuxParser.SEMI, i);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public ForClassicStmtContext(ForStmtContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterForClassicStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitForClassicStmt(this);
		}
	}

	public final ForStmtContext forStmt() throws RecognitionException {
		ForStmtContext _localctx = new ForStmtContext(_ctx, getState());
		enterRule(_localctx, 82, RULE_forStmt);
		try {
			setState(585);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,47,_ctx) ) {
			case 1:
				_localctx = new ForInStmtContext(_localctx);
				enterOuterAlt(_localctx, 1);
				{
				setState(567);
				match(FOR);
				setState(568);
				typeSpec(0);
				setState(569);
				match(IDENTIFIER);
				setState(570);
				match(IN);
				setState(571);
				expression(0);
				setState(572);
				block();
				}
				break;
			case 2:
				_localctx = new ForClassicStmtContext(_localctx);
				enterOuterAlt(_localctx, 2);
				{
				setState(574);
				match(FOR);
				setState(575);
				typeSpec(0);
				setState(576);
				match(IDENTIFIER);
				setState(577);
				match(ASSIGN);
				setState(578);
				expression(0);
				setState(579);
				match(SEMI);
				setState(580);
				expression(0);
				setState(581);
				match(SEMI);
				setState(582);
				expression(0);
				setState(583);
				block();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class BreakStmtContext extends ParserRuleContext {
		public TerminalNode BREAK() { return getToken(LuxParser.BREAK, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public BreakStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_breakStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBreakStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBreakStmt(this);
		}
	}

	public final BreakStmtContext breakStmt() throws RecognitionException {
		BreakStmtContext _localctx = new BreakStmtContext(_ctx, getState());
		enterRule(_localctx, 84, RULE_breakStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(587);
			match(BREAK);
			setState(588);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ContinueStmtContext extends ParserRuleContext {
		public TerminalNode CONTINUE() { return getToken(LuxParser.CONTINUE, 0); }
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ContinueStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_continueStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterContinueStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitContinueStmt(this);
		}
	}

	public final ContinueStmtContext continueStmt() throws RecognitionException {
		ContinueStmtContext _localctx = new ContinueStmtContext(_ctx, getState());
		enterRule(_localctx, 86, RULE_continueStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(590);
			match(CONTINUE);
			setState(591);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class LoopStmtContext extends ParserRuleContext {
		public TerminalNode LOOP() { return getToken(LuxParser.LOOP, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public LoopStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_loopStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLoopStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLoopStmt(this);
		}
	}

	public final LoopStmtContext loopStmt() throws RecognitionException {
		LoopStmtContext _localctx = new LoopStmtContext(_ctx, getState());
		enterRule(_localctx, 88, RULE_loopStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(593);
			match(LOOP);
			setState(594);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class WhileStmtContext extends ParserRuleContext {
		public TerminalNode WHILE() { return getToken(LuxParser.WHILE, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public WhileStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_whileStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterWhileStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitWhileStmt(this);
		}
	}

	public final WhileStmtContext whileStmt() throws RecognitionException {
		WhileStmtContext _localctx = new WhileStmtContext(_ctx, getState());
		enterRule(_localctx, 90, RULE_whileStmt);
		try {
			setState(606);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,48,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(596);
				match(WHILE);
				setState(597);
				expression(0);
				setState(598);
				block();
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(600);
				match(WHILE);
				setState(601);
				match(LPAREN);
				setState(602);
				expression(0);
				setState(603);
				match(RPAREN);
				setState(604);
				block();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class DoWhileStmtContext extends ParserRuleContext {
		public TerminalNode DO() { return getToken(LuxParser.DO, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public TerminalNode WHILE() { return getToken(LuxParser.WHILE, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public DoWhileStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_doWhileStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterDoWhileStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitDoWhileStmt(this);
		}
	}

	public final DoWhileStmtContext doWhileStmt() throws RecognitionException {
		DoWhileStmtContext _localctx = new DoWhileStmtContext(_ctx, getState());
		enterRule(_localctx, 92, RULE_doWhileStmt);
		try {
			setState(622);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,49,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(608);
				match(DO);
				setState(609);
				block();
				setState(610);
				match(WHILE);
				setState(611);
				expression(0);
				setState(612);
				match(SEMI);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(614);
				match(DO);
				setState(615);
				block();
				setState(616);
				match(WHILE);
				setState(617);
				match(LPAREN);
				setState(618);
				expression(0);
				setState(619);
				match(RPAREN);
				setState(620);
				match(SEMI);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class LockStmtContext extends ParserRuleContext {
		public TerminalNode LOCK() { return getToken(LuxParser.LOCK, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public LockStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_lockStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLockStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLockStmt(this);
		}
	}

	public final LockStmtContext lockStmt() throws RecognitionException {
		LockStmtContext _localctx = new LockStmtContext(_ctx, getState());
		enterRule(_localctx, 94, RULE_lockStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(624);
			match(LOCK);
			setState(625);
			match(LPAREN);
			setState(626);
			expression(0);
			setState(627);
			match(RPAREN);
			setState(628);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TryCatchStmtContext extends ParserRuleContext {
		public TerminalNode TRY() { return getToken(LuxParser.TRY, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public List<CatchClauseContext> catchClause() {
			return getRuleContexts(CatchClauseContext.class);
		}
		public CatchClauseContext catchClause(int i) {
			return getRuleContext(CatchClauseContext.class,i);
		}
		public FinallyClauseContext finallyClause() {
			return getRuleContext(FinallyClauseContext.class,0);
		}
		public TryCatchStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_tryCatchStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTryCatchStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTryCatchStmt(this);
		}
	}

	public final TryCatchStmtContext tryCatchStmt() throws RecognitionException {
		TryCatchStmtContext _localctx = new TryCatchStmtContext(_ctx, getState());
		enterRule(_localctx, 96, RULE_tryCatchStmt);
		int _la;
		try {
			setState(644);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,52,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(630);
				match(TRY);
				setState(631);
				block();
				setState(633); 
				_errHandler.sync(this);
				_la = _input.LA(1);
				do {
					{
					{
					setState(632);
					catchClause();
					}
					}
					setState(635); 
					_errHandler.sync(this);
					_la = _input.LA(1);
				} while ( _la==CATCH );
				setState(638);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==FINALLY) {
					{
					setState(637);
					finallyClause();
					}
				}

				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(640);
				match(TRY);
				setState(641);
				block();
				setState(642);
				finallyClause();
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class CatchClauseContext extends ParserRuleContext {
		public TerminalNode CATCH() { return getToken(LuxParser.CATCH, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public CatchClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_catchClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCatchClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCatchClause(this);
		}
	}

	public final CatchClauseContext catchClause() throws RecognitionException {
		CatchClauseContext _localctx = new CatchClauseContext(_ctx, getState());
		enterRule(_localctx, 98, RULE_catchClause);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(646);
			match(CATCH);
			setState(647);
			match(LPAREN);
			setState(648);
			typeSpec(0);
			setState(649);
			match(IDENTIFIER);
			setState(650);
			match(RPAREN);
			setState(651);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FinallyClauseContext extends ParserRuleContext {
		public TerminalNode FINALLY() { return getToken(LuxParser.FINALLY, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public FinallyClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_finallyClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFinallyClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFinallyClause(this);
		}
	}

	public final FinallyClauseContext finallyClause() throws RecognitionException {
		FinallyClauseContext _localctx = new FinallyClauseContext(_ctx, getState());
		enterRule(_localctx, 100, RULE_finallyClause);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(653);
			match(FINALLY);
			setState(654);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ThrowStmtContext extends ParserRuleContext {
		public TerminalNode THROW() { return getToken(LuxParser.THROW, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode SEMI() { return getToken(LuxParser.SEMI, 0); }
		public ThrowStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_throwStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterThrowStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitThrowStmt(this);
		}
	}

	public final ThrowStmtContext throwStmt() throws RecognitionException {
		ThrowStmtContext _localctx = new ThrowStmtContext(_ctx, getState());
		enterRule(_localctx, 102, RULE_throwStmt);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(656);
			match(THROW);
			setState(657);
			expression(0);
			setState(658);
			match(SEMI);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class SwitchStmtContext extends ParserRuleContext {
		public TerminalNode SWITCH() { return getToken(LuxParser.SWITCH, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public List<CaseClauseContext> caseClause() {
			return getRuleContexts(CaseClauseContext.class);
		}
		public CaseClauseContext caseClause(int i) {
			return getRuleContext(CaseClauseContext.class,i);
		}
		public DefaultClauseContext defaultClause() {
			return getRuleContext(DefaultClauseContext.class,0);
		}
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public SwitchStmtContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_switchStmt; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterSwitchStmt(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitSwitchStmt(this);
		}
	}

	public final SwitchStmtContext switchStmt() throws RecognitionException {
		SwitchStmtContext _localctx = new SwitchStmtContext(_ctx, getState());
		enterRule(_localctx, 104, RULE_switchStmt);
		int _la;
		try {
			setState(690);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,57,_ctx) ) {
			case 1:
				enterOuterAlt(_localctx, 1);
				{
				setState(660);
				match(SWITCH);
				setState(661);
				expression(0);
				setState(662);
				match(LBRACE);
				setState(666);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==CASE) {
					{
					{
					setState(663);
					caseClause();
					}
					}
					setState(668);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(670);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==DEFAULT) {
					{
					setState(669);
					defaultClause();
					}
				}

				setState(672);
				match(RBRACE);
				}
				break;
			case 2:
				enterOuterAlt(_localctx, 2);
				{
				setState(674);
				match(SWITCH);
				setState(675);
				match(LPAREN);
				setState(676);
				expression(0);
				setState(677);
				match(RPAREN);
				setState(678);
				match(LBRACE);
				setState(682);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==CASE) {
					{
					{
					setState(679);
					caseClause();
					}
					}
					setState(684);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(686);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==DEFAULT) {
					{
					setState(685);
					defaultClause();
					}
				}

				setState(688);
				match(RBRACE);
				}
				break;
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class CaseClauseContext extends ParserRuleContext {
		public TerminalNode CASE() { return getToken(LuxParser.CASE, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public CaseClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_caseClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCaseClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCaseClause(this);
		}
	}

	public final CaseClauseContext caseClause() throws RecognitionException {
		CaseClauseContext _localctx = new CaseClauseContext(_ctx, getState());
		enterRule(_localctx, 106, RULE_caseClause);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(692);
			match(CASE);
			setState(693);
			expression(0);
			setState(698);
			_errHandler.sync(this);
			_la = _input.LA(1);
			while (_la==COMMA) {
				{
				{
				setState(694);
				match(COMMA);
				setState(695);
				expression(0);
				}
				}
				setState(700);
				_errHandler.sync(this);
				_la = _input.LA(1);
			}
			setState(701);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class DefaultClauseContext extends ParserRuleContext {
		public TerminalNode DEFAULT() { return getToken(LuxParser.DEFAULT, 0); }
		public BlockContext block() {
			return getRuleContext(BlockContext.class,0);
		}
		public DefaultClauseContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_defaultClause; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterDefaultClause(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitDefaultClause(this);
		}
	}

	public final DefaultClauseContext defaultClause() throws RecognitionException {
		DefaultClauseContext _localctx = new DefaultClauseContext(_ctx, getState());
		enterRule(_localctx, 108, RULE_defaultClause);
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(703);
			match(DEFAULT);
			setState(704);
			block();
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class ExpressionContext extends ParserRuleContext {
		public ExpressionContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_expression; }
	 
		public ExpressionContext() { }
		public void copyFrom(ExpressionContext ctx) {
			super.copyFrom(ctx);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class FieldAccessExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode DOT() { return getToken(LuxParser.DOT, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public FieldAccessExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFieldAccessExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFieldAccessExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TypeofExprContext extends ExpressionContext {
		public TerminalNode TYPEOF() { return getToken(LuxParser.TYPEOF, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TypeofExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTypeofExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTypeofExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RshiftExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> GT() { return getTokens(LuxParser.GT); }
		public TerminalNode GT(int i) {
			return getToken(LuxParser.GT, i);
		}
		public RshiftExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterRshiftExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitRshiftExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrowMethodCallExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public ArgListContext argList() {
			return getRuleContext(ArgListContext.class,0);
		}
		public ArrowMethodCallExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArrowMethodCallExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArrowMethodCallExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class OctLitExprContext extends ExpressionContext {
		public TerminalNode OCT_LIT() { return getToken(LuxParser.OCT_LIT, 0); }
		public OctLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterOctLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitOctLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BitXorExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode CARET() { return getToken(LuxParser.CARET, 0); }
		public BitXorExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBitXorExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBitXorExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LogicalNotExprContext extends ExpressionContext {
		public TerminalNode NOT() { return getToken(LuxParser.NOT, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public LogicalNotExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLogicalNotExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLogicalNotExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IdentExprContext extends ExpressionContext {
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public IdentExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIdentExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIdentExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class PreIncrExprContext extends ExpressionContext {
		public TerminalNode INCR() { return getToken(LuxParser.INCR, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public PreIncrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPreIncrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPreIncrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TernaryExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode QUESTION() { return getToken(LuxParser.QUESTION, 0); }
		public TerminalNode COLON() { return getToken(LuxParser.COLON, 0); }
		public TernaryExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTernaryExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTernaryExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ChainedTupleIndexExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode DOT() { return getToken(LuxParser.DOT, 0); }
		public TerminalNode FLOAT_LIT() { return getToken(LuxParser.FLOAT_LIT, 0); }
		public ChainedTupleIndexExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterChainedTupleIndexExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitChainedTupleIndexExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class NullLitExprContext extends ExpressionContext {
		public TerminalNode NULL_LIT() { return getToken(LuxParser.NULL_LIT, 0); }
		public NullLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterNullLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitNullLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class MulExprContext extends ExpressionContext {
		public Token op;
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode STAR() { return getToken(LuxParser.STAR, 0); }
		public TerminalNode SLASH() { return getToken(LuxParser.SLASH, 0); }
		public TerminalNode PERCENT() { return getToken(LuxParser.PERCENT, 0); }
		public MulExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterMulExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitMulExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BitAndExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode AMPERSAND() { return getToken(LuxParser.AMPERSAND, 0); }
		public BitAndExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBitAndExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBitAndExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IsExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode IS() { return getToken(LuxParser.IS, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public IsExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIsExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIsExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LshiftExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode LSHIFT() { return getToken(LuxParser.LSHIFT, 0); }
		public LshiftExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLshiftExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLshiftExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TupleLitExprContext extends ExpressionContext {
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TupleLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTupleLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTupleLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class AddSubExprContext extends ExpressionContext {
		public Token op;
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode PLUS() { return getToken(LuxParser.PLUS, 0); }
		public TerminalNode MINUS() { return getToken(LuxParser.MINUS, 0); }
		public AddSubExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterAddSubExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitAddSubExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IntLitExprContext extends ExpressionContext {
		public TerminalNode INT_LIT() { return getToken(LuxParser.INT_LIT, 0); }
		public IntLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIntLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIntLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class AddrOfExprContext extends ExpressionContext {
		public TerminalNode AMPERSAND() { return getToken(LuxParser.AMPERSAND, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public AddrOfExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterAddrOfExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitAddrOfExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TupleIndexExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode DOT() { return getToken(LuxParser.DOT, 0); }
		public TerminalNode INT_LIT() { return getToken(LuxParser.INT_LIT, 0); }
		public TupleIndexExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTupleIndexExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTupleIndexExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class FloatLitExprContext extends ExpressionContext {
		public TerminalNode FLOAT_LIT() { return getToken(LuxParser.FLOAT_LIT, 0); }
		public FloatLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFloatLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFloatLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SpawnExprContext extends ExpressionContext {
		public TerminalNode SPAWN() { return getToken(LuxParser.SPAWN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public SpawnExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterSpawnExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitSpawnExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrowAccessExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public ArrowAccessExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArrowAccessExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArrowAccessExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ListCompExprContext extends ExpressionContext {
		public TerminalNode LBRACKET() { return getToken(LuxParser.LBRACKET, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode PIPE() { return getToken(LuxParser.PIPE, 0); }
		public TerminalNode FOR() { return getToken(LuxParser.FOR, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode IN() { return getToken(LuxParser.IN, 0); }
		public TerminalNode RBRACKET() { return getToken(LuxParser.RBRACKET, 0); }
		public TerminalNode IF() { return getToken(LuxParser.IF, 0); }
		public ListCompExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterListCompExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitListCompExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class IndexExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode LBRACKET() { return getToken(LuxParser.LBRACKET, 0); }
		public TerminalNode RBRACKET() { return getToken(LuxParser.RBRACKET, 0); }
		public IndexExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterIndexExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitIndexExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class NegExprContext extends ExpressionContext {
		public TerminalNode MINUS() { return getToken(LuxParser.MINUS, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public NegExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterNegExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitNegExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class DerefExprContext extends ExpressionContext {
		public TerminalNode STAR() { return getToken(LuxParser.STAR, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public DerefExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterDerefExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitDerefExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class PreDecrExprContext extends ExpressionContext {
		public TerminalNode DECR() { return getToken(LuxParser.DECR, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public PreDecrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPreDecrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPreDecrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SpreadExprContext extends ExpressionContext {
		public TerminalNode SPREAD() { return getToken(LuxParser.SPREAD, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public SpreadExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterSpreadExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitSpreadExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class StaticMethodCallExprContext extends ExpressionContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode SCOPE() { return getToken(LuxParser.SCOPE, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public ArgListContext argList() {
			return getRuleContext(ArgListContext.class,0);
		}
		public StaticMethodCallExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStaticMethodCallExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStaticMethodCallExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class NullCoalExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode NULLCOAL() { return getToken(LuxParser.NULLCOAL, 0); }
		public NullCoalExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterNullCoalExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitNullCoalExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class CastExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode AS() { return getToken(LuxParser.AS, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public CastExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCastExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCastExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class EnumAccessExprContext extends ExpressionContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode SCOPE() { return getToken(LuxParser.SCOPE, 0); }
		public EnumAccessExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterEnumAccessExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitEnumAccessExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ParenExprContext extends ExpressionContext {
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public ParenExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterParenExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitParenExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BitNotExprContext extends ExpressionContext {
		public TerminalNode TILDE() { return getToken(LuxParser.TILDE, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public BitNotExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBitNotExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBitNotExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ArrayLitExprContext extends ExpressionContext {
		public TerminalNode LBRACKET() { return getToken(LuxParser.LBRACKET, 0); }
		public TerminalNode RBRACKET() { return getToken(LuxParser.RBRACKET, 0); }
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public ArrayLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterArrayLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitArrayLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class MethodCallExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode DOT() { return getToken(LuxParser.DOT, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public ArgListContext argList() {
			return getRuleContext(ArgListContext.class,0);
		}
		public MethodCallExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterMethodCallExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitMethodCallExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class StructLitExprContext extends ExpressionContext {
		public List<TerminalNode> IDENTIFIER() { return getTokens(LuxParser.IDENTIFIER); }
		public TerminalNode IDENTIFIER(int i) {
			return getToken(LuxParser.IDENTIFIER, i);
		}
		public TerminalNode LBRACE() { return getToken(LuxParser.LBRACE, 0); }
		public TerminalNode RBRACE() { return getToken(LuxParser.RBRACE, 0); }
		public List<TerminalNode> COLON() { return getTokens(LuxParser.COLON); }
		public TerminalNode COLON(int i) {
			return getToken(LuxParser.COLON, i);
		}
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public StructLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStructLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStructLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class PostDecrExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode DECR() { return getToken(LuxParser.DECR, 0); }
		public PostDecrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPostDecrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPostDecrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RelExprContext extends ExpressionContext {
		public Token op;
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode LT() { return getToken(LuxParser.LT, 0); }
		public TerminalNode GT() { return getToken(LuxParser.GT, 0); }
		public TerminalNode LTE() { return getToken(LuxParser.LTE, 0); }
		public TerminalNode GTE() { return getToken(LuxParser.GTE, 0); }
		public RelExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterRelExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitRelExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BinLitExprContext extends ExpressionContext {
		public TerminalNode BIN_LIT() { return getToken(LuxParser.BIN_LIT, 0); }
		public BinLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBinLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBinLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RangeInclExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode RANGE_INCL() { return getToken(LuxParser.RANGE_INCL, 0); }
		public RangeInclExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterRangeInclExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitRangeInclExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TupleArrowIndexExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public TerminalNode INT_LIT() { return getToken(LuxParser.INT_LIT, 0); }
		public TupleArrowIndexExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTupleArrowIndexExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTupleArrowIndexExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LogicalAndExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode LAND() { return getToken(LuxParser.LAND, 0); }
		public LogicalAndExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLogicalAndExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLogicalAndExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class StrLitExprContext extends ExpressionContext {
		public TerminalNode STR_LIT() { return getToken(LuxParser.STR_LIT, 0); }
		public StrLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterStrLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitStrLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class AwaitExprContext extends ExpressionContext {
		public TerminalNode AWAIT() { return getToken(LuxParser.AWAIT, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public AwaitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterAwaitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitAwaitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class CStrLitExprContext extends ExpressionContext {
		public TerminalNode C_STR_LIT() { return getToken(LuxParser.C_STR_LIT, 0); }
		public CStrLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCStrLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCStrLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class FnCallExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public ArgListContext argList() {
			return getRuleContext(ArgListContext.class,0);
		}
		public FnCallExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFnCallExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFnCallExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class LogicalOrExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode LOR() { return getToken(LuxParser.LOR, 0); }
		public LogicalOrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterLogicalOrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitLogicalOrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class SizeofExprContext extends ExpressionContext {
		public TerminalNode SIZEOF() { return getToken(LuxParser.SIZEOF, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TypeSpecContext typeSpec() {
			return getRuleContext(TypeSpecContext.class,0);
		}
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public SizeofExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterSizeofExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitSizeofExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class EqExprContext extends ExpressionContext {
		public Token op;
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode EQ() { return getToken(LuxParser.EQ, 0); }
		public TerminalNode NEQ() { return getToken(LuxParser.NEQ, 0); }
		public EqExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterEqExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitEqExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BitOrExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode PIPE() { return getToken(LuxParser.PIPE, 0); }
		public BitOrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBitOrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBitOrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class CharLitExprContext extends ExpressionContext {
		public TerminalNode CHAR_LIT() { return getToken(LuxParser.CHAR_LIT, 0); }
		public CharLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterCharLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitCharLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class TryExprContext extends ExpressionContext {
		public TerminalNode TRY() { return getToken(LuxParser.TRY, 0); }
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TryExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTryExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTryExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class PostIncrExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode INCR() { return getToken(LuxParser.INCR, 0); }
		public PostIncrExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPostIncrExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPostIncrExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class BoolLitExprContext extends ExpressionContext {
		public TerminalNode BOOL_LIT() { return getToken(LuxParser.BOOL_LIT, 0); }
		public BoolLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterBoolLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitBoolLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class HexLitExprContext extends ExpressionContext {
		public TerminalNode HEX_LIT() { return getToken(LuxParser.HEX_LIT, 0); }
		public HexLitExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterHexLitExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitHexLitExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class ChainedTupleArrowIndexExprContext extends ExpressionContext {
		public ExpressionContext expression() {
			return getRuleContext(ExpressionContext.class,0);
		}
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public TerminalNode FLOAT_LIT() { return getToken(LuxParser.FLOAT_LIT, 0); }
		public ChainedTupleArrowIndexExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterChainedTupleArrowIndexExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitChainedTupleArrowIndexExpr(this);
		}
	}
	@SuppressWarnings("CheckReturnValue")
	public static class RangeExprContext extends ExpressionContext {
		public List<ExpressionContext> expression() {
			return getRuleContexts(ExpressionContext.class);
		}
		public ExpressionContext expression(int i) {
			return getRuleContext(ExpressionContext.class,i);
		}
		public TerminalNode RANGE() { return getToken(LuxParser.RANGE, 0); }
		public RangeExprContext(ExpressionContext ctx) { copyFrom(ctx); }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterRangeExpr(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitRangeExpr(this);
		}
	}

	public final ExpressionContext expression() throws RecognitionException {
		return expression(0);
	}

	private ExpressionContext expression(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		ExpressionContext _localctx = new ExpressionContext(_ctx, _parentState);
		ExpressionContext _prevctx = _localctx;
		int _startState = 110;
		enterRecursionRule(_localctx, 110, RULE_expression, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(821);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,66,_ctx) ) {
			case 1:
				{
				_localctx = new StructLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;

				setState(707);
				match(IDENTIFIER);
				setState(708);
				match(LBRACE);
				setState(721);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==IDENTIFIER) {
					{
					setState(709);
					match(IDENTIFIER);
					setState(710);
					match(COLON);
					setState(711);
					expression(0);
					setState(718);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(712);
						match(COMMA);
						setState(713);
						match(IDENTIFIER);
						setState(714);
						match(COLON);
						setState(715);
						expression(0);
						}
						}
						setState(720);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(723);
				match(RBRACE);
				}
				break;
			case 2:
				{
				_localctx = new StaticMethodCallExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(724);
				match(IDENTIFIER);
				setState(725);
				match(SCOPE);
				setState(726);
				match(IDENTIFIER);
				setState(727);
				match(LPAREN);
				setState(729);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
					{
					setState(728);
					argList();
					}
				}

				setState(731);
				match(RPAREN);
				}
				break;
			case 3:
				{
				_localctx = new EnumAccessExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(732);
				match(IDENTIFIER);
				setState(733);
				match(SCOPE);
				setState(734);
				match(IDENTIFIER);
				}
				break;
			case 4:
				{
				_localctx = new DerefExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(735);
				match(STAR);
				setState(736);
				expression(43);
				}
				break;
			case 5:
				{
				_localctx = new AddrOfExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(737);
				match(AMPERSAND);
				setState(738);
				expression(42);
				}
				break;
			case 6:
				{
				_localctx = new NegExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(739);
				match(MINUS);
				setState(740);
				expression(41);
				}
				break;
			case 7:
				{
				_localctx = new SpawnExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(741);
				match(SPAWN);
				setState(742);
				expression(40);
				}
				break;
			case 8:
				{
				_localctx = new AwaitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(743);
				match(AWAIT);
				setState(744);
				expression(39);
				}
				break;
			case 9:
				{
				_localctx = new LogicalNotExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(745);
				match(NOT);
				setState(746);
				expression(38);
				}
				break;
			case 10:
				{
				_localctx = new BitNotExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(747);
				match(TILDE);
				setState(748);
				expression(37);
				}
				break;
			case 11:
				{
				_localctx = new PreIncrExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(749);
				match(INCR);
				setState(750);
				expression(36);
				}
				break;
			case 12:
				{
				_localctx = new PreDecrExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(751);
				match(DECR);
				setState(752);
				expression(35);
				}
				break;
			case 13:
				{
				_localctx = new SizeofExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(753);
				match(SIZEOF);
				setState(754);
				match(LPAREN);
				setState(755);
				typeSpec(0);
				setState(756);
				match(RPAREN);
				}
				break;
			case 14:
				{
				_localctx = new TypeofExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(758);
				match(TYPEOF);
				setState(759);
				match(LPAREN);
				setState(760);
				expression(0);
				setState(761);
				match(RPAREN);
				}
				break;
			case 15:
				{
				_localctx = new TryExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(763);
				match(TRY);
				setState(764);
				expression(17);
				}
				break;
			case 16:
				{
				_localctx = new TupleLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(765);
				match(LPAREN);
				setState(766);
				expression(0);
				setState(767);
				match(COMMA);
				setState(768);
				expression(0);
				setState(773);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(769);
					match(COMMA);
					setState(770);
					expression(0);
					}
					}
					setState(775);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(776);
				match(RPAREN);
				}
				break;
			case 17:
				{
				_localctx = new ParenExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(778);
				match(LPAREN);
				setState(779);
				expression(0);
				setState(780);
				match(RPAREN);
				}
				break;
			case 18:
				{
				_localctx = new SpreadExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(782);
				match(SPREAD);
				setState(783);
				expression(14);
				}
				break;
			case 19:
				{
				_localctx = new ListCompExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(784);
				match(LBRACKET);
				setState(785);
				expression(0);
				setState(786);
				match(PIPE);
				setState(787);
				match(FOR);
				setState(788);
				typeSpec(0);
				setState(789);
				match(IDENTIFIER);
				setState(790);
				match(IN);
				setState(791);
				expression(0);
				setState(794);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if (_la==IF) {
					{
					setState(792);
					match(IF);
					setState(793);
					expression(0);
					}
				}

				setState(796);
				match(RBRACKET);
				}
				break;
			case 20:
				{
				_localctx = new ArrayLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(798);
				match(LBRACKET);
				setState(807);
				_errHandler.sync(this);
				_la = _input.LA(1);
				if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
					{
					setState(799);
					expression(0);
					setState(804);
					_errHandler.sync(this);
					_la = _input.LA(1);
					while (_la==COMMA) {
						{
						{
						setState(800);
						match(COMMA);
						setState(801);
						expression(0);
						}
						}
						setState(806);
						_errHandler.sync(this);
						_la = _input.LA(1);
					}
					}
				}

				setState(809);
				match(RBRACKET);
				}
				break;
			case 21:
				{
				_localctx = new NullLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(810);
				match(NULL_LIT);
				}
				break;
			case 22:
				{
				_localctx = new IntLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(811);
				match(INT_LIT);
				}
				break;
			case 23:
				{
				_localctx = new HexLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(812);
				match(HEX_LIT);
				}
				break;
			case 24:
				{
				_localctx = new OctLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(813);
				match(OCT_LIT);
				}
				break;
			case 25:
				{
				_localctx = new BinLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(814);
				match(BIN_LIT);
				}
				break;
			case 26:
				{
				_localctx = new FloatLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(815);
				match(FLOAT_LIT);
				}
				break;
			case 27:
				{
				_localctx = new BoolLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(816);
				match(BOOL_LIT);
				}
				break;
			case 28:
				{
				_localctx = new CharLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(817);
				match(CHAR_LIT);
				}
				break;
			case 29:
				{
				_localctx = new StrLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(818);
				match(STR_LIT);
				}
				break;
			case 30:
				{
				_localctx = new CStrLitExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(819);
				match(C_STR_LIT);
				}
				break;
			case 31:
				{
				_localctx = new IdentExprContext(_localctx);
				_ctx = _localctx;
				_prevctx = _localctx;
				setState(820);
				match(IDENTIFIER);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(929);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,71,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					setState(927);
					_errHandler.sync(this);
					switch ( getInterpreter().adaptivePredict(_input,70,_ctx) ) {
					case 1:
						{
						_localctx = new MulExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(823);
						if (!(precpred(_ctx, 32))) throw new FailedPredicateException(this, "precpred(_ctx, 32)");
						setState(824);
						((MulExprContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(((((_la - 105)) & ~0x3f) == 0 && ((1L << (_la - 105)) & 49L) != 0)) ) {
							((MulExprContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(825);
						expression(33);
						}
						break;
					case 2:
						{
						_localctx = new AddSubExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(826);
						if (!(precpred(_ctx, 31))) throw new FailedPredicateException(this, "precpred(_ctx, 31)");
						setState(827);
						((AddSubExprContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==MINUS || _la==PLUS) ) {
							((AddSubExprContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(828);
						expression(32);
						}
						break;
					case 3:
						{
						_localctx = new LshiftExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(829);
						if (!(precpred(_ctx, 30))) throw new FailedPredicateException(this, "precpred(_ctx, 30)");
						setState(830);
						match(LSHIFT);
						setState(831);
						expression(31);
						}
						break;
					case 4:
						{
						_localctx = new RshiftExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(832);
						if (!(precpred(_ctx, 29))) throw new FailedPredicateException(this, "precpred(_ctx, 29)");
						setState(833);
						match(GT);
						setState(834);
						match(GT);
						setState(835);
						expression(30);
						}
						break;
					case 5:
						{
						_localctx = new RelExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(836);
						if (!(precpred(_ctx, 28))) throw new FailedPredicateException(this, "precpred(_ctx, 28)");
						setState(837);
						((RelExprContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(((((_la - 113)) & ~0x3f) == 0 && ((1L << (_la - 113)) & 15L) != 0)) ) {
							((RelExprContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(838);
						expression(29);
						}
						break;
					case 6:
						{
						_localctx = new EqExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(839);
						if (!(precpred(_ctx, 27))) throw new FailedPredicateException(this, "precpred(_ctx, 27)");
						setState(840);
						((EqExprContext)_localctx).op = _input.LT(1);
						_la = _input.LA(1);
						if ( !(_la==EQ || _la==NEQ) ) {
							((EqExprContext)_localctx).op = (Token)_errHandler.recoverInline(this);
						}
						else {
							if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
							_errHandler.reportMatch(this);
							consume();
						}
						setState(841);
						expression(28);
						}
						break;
					case 7:
						{
						_localctx = new BitAndExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(842);
						if (!(precpred(_ctx, 26))) throw new FailedPredicateException(this, "precpred(_ctx, 26)");
						setState(843);
						match(AMPERSAND);
						setState(844);
						expression(27);
						}
						break;
					case 8:
						{
						_localctx = new BitXorExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(845);
						if (!(precpred(_ctx, 25))) throw new FailedPredicateException(this, "precpred(_ctx, 25)");
						setState(846);
						match(CARET);
						setState(847);
						expression(26);
						}
						break;
					case 9:
						{
						_localctx = new BitOrExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(848);
						if (!(precpred(_ctx, 24))) throw new FailedPredicateException(this, "precpred(_ctx, 24)");
						setState(849);
						match(PIPE);
						setState(850);
						expression(25);
						}
						break;
					case 10:
						{
						_localctx = new LogicalAndExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(851);
						if (!(precpred(_ctx, 23))) throw new FailedPredicateException(this, "precpred(_ctx, 23)");
						setState(852);
						match(LAND);
						setState(853);
						expression(24);
						}
						break;
					case 11:
						{
						_localctx = new LogicalOrExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(854);
						if (!(precpred(_ctx, 22))) throw new FailedPredicateException(this, "precpred(_ctx, 22)");
						setState(855);
						match(LOR);
						setState(856);
						expression(23);
						}
						break;
					case 12:
						{
						_localctx = new NullCoalExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(857);
						if (!(precpred(_ctx, 21))) throw new FailedPredicateException(this, "precpred(_ctx, 21)");
						setState(858);
						match(NULLCOAL);
						setState(859);
						expression(22);
						}
						break;
					case 13:
						{
						_localctx = new RangeExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(860);
						if (!(precpred(_ctx, 20))) throw new FailedPredicateException(this, "precpred(_ctx, 20)");
						setState(861);
						match(RANGE);
						setState(862);
						expression(21);
						}
						break;
					case 14:
						{
						_localctx = new RangeInclExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(863);
						if (!(precpred(_ctx, 19))) throw new FailedPredicateException(this, "precpred(_ctx, 19)");
						setState(864);
						match(RANGE_INCL);
						setState(865);
						expression(20);
						}
						break;
					case 15:
						{
						_localctx = new TernaryExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(866);
						if (!(precpred(_ctx, 18))) throw new FailedPredicateException(this, "precpred(_ctx, 18)");
						setState(867);
						match(QUESTION);
						setState(868);
						expression(0);
						setState(869);
						match(COLON);
						setState(870);
						expression(18);
						}
						break;
					case 16:
						{
						_localctx = new MethodCallExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(872);
						if (!(precpred(_ctx, 60))) throw new FailedPredicateException(this, "precpred(_ctx, 60)");
						setState(873);
						match(DOT);
						setState(874);
						match(IDENTIFIER);
						setState(875);
						match(LPAREN);
						setState(877);
						_errHandler.sync(this);
						_la = _input.LA(1);
						if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
							{
							setState(876);
							argList();
							}
						}

						setState(879);
						match(RPAREN);
						}
						break;
					case 17:
						{
						_localctx = new FnCallExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(880);
						if (!(precpred(_ctx, 59))) throw new FailedPredicateException(this, "precpred(_ctx, 59)");
						setState(881);
						match(LPAREN);
						setState(883);
						_errHandler.sync(this);
						_la = _input.LA(1);
						if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
							{
							setState(882);
							argList();
							}
						}

						setState(885);
						match(RPAREN);
						}
						break;
					case 18:
						{
						_localctx = new FieldAccessExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(886);
						if (!(precpred(_ctx, 58))) throw new FailedPredicateException(this, "precpred(_ctx, 58)");
						setState(887);
						match(DOT);
						setState(888);
						match(IDENTIFIER);
						}
						break;
					case 19:
						{
						_localctx = new TupleIndexExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(889);
						if (!(precpred(_ctx, 57))) throw new FailedPredicateException(this, "precpred(_ctx, 57)");
						setState(890);
						match(DOT);
						setState(891);
						match(INT_LIT);
						}
						break;
					case 20:
						{
						_localctx = new ChainedTupleIndexExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(892);
						if (!(precpred(_ctx, 56))) throw new FailedPredicateException(this, "precpred(_ctx, 56)");
						setState(893);
						match(DOT);
						setState(894);
						match(FLOAT_LIT);
						}
						break;
					case 21:
						{
						_localctx = new ArrowMethodCallExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(895);
						if (!(precpred(_ctx, 55))) throw new FailedPredicateException(this, "precpred(_ctx, 55)");
						setState(896);
						match(ARROW);
						setState(897);
						match(IDENTIFIER);
						setState(898);
						match(LPAREN);
						setState(900);
						_errHandler.sync(this);
						_la = _input.LA(1);
						if ((((_la) & ~0x3f) == 0 && ((1L << _la) & 35185647169664L) != 0) || ((((_la - 73)) & ~0x3f) == 0 && ((1L << (_la - 73)) & 4996212042236927L) != 0)) {
							{
							setState(899);
							argList();
							}
						}

						setState(902);
						match(RPAREN);
						}
						break;
					case 22:
						{
						_localctx = new ArrowAccessExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(903);
						if (!(precpred(_ctx, 54))) throw new FailedPredicateException(this, "precpred(_ctx, 54)");
						setState(904);
						match(ARROW);
						setState(905);
						match(IDENTIFIER);
						}
						break;
					case 23:
						{
						_localctx = new TupleArrowIndexExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(906);
						if (!(precpred(_ctx, 53))) throw new FailedPredicateException(this, "precpred(_ctx, 53)");
						setState(907);
						match(ARROW);
						setState(908);
						match(INT_LIT);
						}
						break;
					case 24:
						{
						_localctx = new ChainedTupleArrowIndexExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(909);
						if (!(precpred(_ctx, 52))) throw new FailedPredicateException(this, "precpred(_ctx, 52)");
						setState(910);
						match(ARROW);
						setState(911);
						match(FLOAT_LIT);
						}
						break;
					case 25:
						{
						_localctx = new IndexExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(912);
						if (!(precpred(_ctx, 51))) throw new FailedPredicateException(this, "precpred(_ctx, 51)");
						setState(913);
						match(LBRACKET);
						setState(914);
						expression(0);
						setState(915);
						match(RBRACKET);
						}
						break;
					case 26:
						{
						_localctx = new CastExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(917);
						if (!(precpred(_ctx, 50))) throw new FailedPredicateException(this, "precpred(_ctx, 50)");
						setState(918);
						match(AS);
						setState(919);
						typeSpec(0);
						}
						break;
					case 27:
						{
						_localctx = new IsExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(920);
						if (!(precpred(_ctx, 49))) throw new FailedPredicateException(this, "precpred(_ctx, 49)");
						setState(921);
						match(IS);
						setState(922);
						typeSpec(0);
						}
						break;
					case 28:
						{
						_localctx = new PostIncrExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(923);
						if (!(precpred(_ctx, 48))) throw new FailedPredicateException(this, "precpred(_ctx, 48)");
						setState(924);
						match(INCR);
						}
						break;
					case 29:
						{
						_localctx = new PostDecrExprContext(new ExpressionContext(_parentctx, _parentState));
						pushNewRecursionContext(_localctx, _startState, RULE_expression);
						setState(925);
						if (!(precpred(_ctx, 47))) throw new FailedPredicateException(this, "precpred(_ctx, 47)");
						setState(926);
						match(DECR);
						}
						break;
					}
					} 
				}
				setState(931);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,71,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class TypeSpecContext extends ParserRuleContext {
		public TerminalNode STAR() { return getToken(LuxParser.STAR, 0); }
		public List<TypeSpecContext> typeSpec() {
			return getRuleContexts(TypeSpecContext.class);
		}
		public TypeSpecContext typeSpec(int i) {
			return getRuleContext(TypeSpecContext.class,i);
		}
		public TerminalNode LBRACKET() { return getToken(LuxParser.LBRACKET, 0); }
		public TerminalNode INT_LIT() { return getToken(LuxParser.INT_LIT, 0); }
		public TerminalNode RBRACKET() { return getToken(LuxParser.RBRACKET, 0); }
		public FnTypeSpecContext fnTypeSpec() {
			return getRuleContext(FnTypeSpecContext.class,0);
		}
		public TerminalNode VEC() { return getToken(LuxParser.VEC, 0); }
		public TerminalNode LT() { return getToken(LuxParser.LT, 0); }
		public TerminalNode GT() { return getToken(LuxParser.GT, 0); }
		public TerminalNode MAP() { return getToken(LuxParser.MAP, 0); }
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public TerminalNode SET() { return getToken(LuxParser.SET, 0); }
		public TerminalNode TUPLE() { return getToken(LuxParser.TUPLE, 0); }
		public TerminalNode IDENTIFIER() { return getToken(LuxParser.IDENTIFIER, 0); }
		public PrimitiveTypeContext primitiveType() {
			return getRuleContext(PrimitiveTypeContext.class,0);
		}
		public TerminalNode AUTO() { return getToken(LuxParser.AUTO, 0); }
		public TypeSpecContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_typeSpec; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterTypeSpec(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitTypeSpec(this);
		}
	}

	public final TypeSpecContext typeSpec() throws RecognitionException {
		return typeSpec(0);
	}

	private TypeSpecContext typeSpec(int _p) throws RecognitionException {
		ParserRuleContext _parentctx = _ctx;
		int _parentState = getState();
		TypeSpecContext _localctx = new TypeSpecContext(_ctx, _parentState);
		TypeSpecContext _prevctx = _localctx;
		int _startState = 112;
		enterRecursionRule(_localctx, 112, RULE_typeSpec, _p);
		int _la;
		try {
			int _alt;
			enterOuterAlt(_localctx, 1);
			{
			setState(986);
			_errHandler.sync(this);
			switch ( getInterpreter().adaptivePredict(_input,74,_ctx) ) {
			case 1:
				{
				setState(933);
				match(STAR);
				setState(934);
				typeSpec(13);
				}
				break;
			case 2:
				{
				setState(935);
				match(LBRACKET);
				setState(936);
				match(INT_LIT);
				setState(937);
				match(RBRACKET);
				setState(938);
				typeSpec(11);
				}
				break;
			case 3:
				{
				setState(939);
				match(LBRACKET);
				setState(940);
				match(RBRACKET);
				setState(941);
				typeSpec(10);
				}
				break;
			case 4:
				{
				setState(942);
				fnTypeSpec();
				}
				break;
			case 5:
				{
				setState(943);
				match(VEC);
				setState(944);
				match(LT);
				setState(945);
				typeSpec(0);
				setState(946);
				match(GT);
				}
				break;
			case 6:
				{
				setState(948);
				match(MAP);
				setState(949);
				match(LT);
				setState(950);
				typeSpec(0);
				setState(951);
				match(COMMA);
				setState(952);
				typeSpec(0);
				setState(953);
				match(GT);
				}
				break;
			case 7:
				{
				setState(955);
				match(SET);
				setState(956);
				match(LT);
				setState(957);
				typeSpec(0);
				setState(958);
				match(GT);
				}
				break;
			case 8:
				{
				setState(960);
				match(TUPLE);
				setState(961);
				match(LT);
				setState(962);
				typeSpec(0);
				setState(965); 
				_errHandler.sync(this);
				_la = _input.LA(1);
				do {
					{
					{
					setState(963);
					match(COMMA);
					setState(964);
					typeSpec(0);
					}
					}
					setState(967); 
					_errHandler.sync(this);
					_la = _input.LA(1);
				} while ( _la==COMMA );
				setState(969);
				match(GT);
				}
				break;
			case 9:
				{
				setState(971);
				match(IDENTIFIER);
				setState(972);
				match(LT);
				setState(973);
				typeSpec(0);
				setState(978);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(974);
					match(COMMA);
					setState(975);
					typeSpec(0);
					}
					}
					setState(980);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				setState(981);
				match(GT);
				}
				break;
			case 10:
				{
				setState(983);
				primitiveType();
				}
				break;
			case 11:
				{
				setState(984);
				match(AUTO);
				}
				break;
			case 12:
				{
				setState(985);
				match(IDENTIFIER);
				}
				break;
			}
			_ctx.stop = _input.LT(-1);
			setState(992);
			_errHandler.sync(this);
			_alt = getInterpreter().adaptivePredict(_input,75,_ctx);
			while ( _alt!=2 && _alt!=org.antlr.v4.runtime.atn.ATN.INVALID_ALT_NUMBER ) {
				if ( _alt==1 ) {
					if ( _parseListeners!=null ) triggerExitRuleEvent();
					_prevctx = _localctx;
					{
					{
					_localctx = new TypeSpecContext(_parentctx, _parentState);
					pushNewRecursionContext(_localctx, _startState, RULE_typeSpec);
					setState(988);
					if (!(precpred(_ctx, 12))) throw new FailedPredicateException(this, "precpred(_ctx, 12)");
					setState(989);
					match(STAR);
					}
					} 
				}
				setState(994);
				_errHandler.sync(this);
				_alt = getInterpreter().adaptivePredict(_input,75,_ctx);
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			unrollRecursionContexts(_parentctx);
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class FnTypeSpecContext extends ParserRuleContext {
		public TerminalNode FN() { return getToken(LuxParser.FN, 0); }
		public TerminalNode LPAREN() { return getToken(LuxParser.LPAREN, 0); }
		public TerminalNode RPAREN() { return getToken(LuxParser.RPAREN, 0); }
		public TerminalNode ARROW() { return getToken(LuxParser.ARROW, 0); }
		public List<TypeSpecContext> typeSpec() {
			return getRuleContexts(TypeSpecContext.class);
		}
		public TypeSpecContext typeSpec(int i) {
			return getRuleContext(TypeSpecContext.class,i);
		}
		public List<TerminalNode> COMMA() { return getTokens(LuxParser.COMMA); }
		public TerminalNode COMMA(int i) {
			return getToken(LuxParser.COMMA, i);
		}
		public FnTypeSpecContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_fnTypeSpec; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterFnTypeSpec(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitFnTypeSpec(this);
		}
	}

	public final FnTypeSpecContext fnTypeSpec() throws RecognitionException {
		FnTypeSpecContext _localctx = new FnTypeSpecContext(_ctx, getState());
		enterRule(_localctx, 114, RULE_fnTypeSpec);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(995);
			match(FN);
			setState(996);
			match(LPAREN);
			setState(1005);
			_errHandler.sync(this);
			_la = _input.LA(1);
			if ((((_la) & ~0x3f) == 0 && ((1L << _la) & -279344672931584L) != 0) || ((((_la - 64)) & ~0x3f) == 0 && ((1L << (_la - 64)) & 2748779332095L) != 0)) {
				{
				setState(997);
				typeSpec(0);
				setState(1002);
				_errHandler.sync(this);
				_la = _input.LA(1);
				while (_la==COMMA) {
					{
					{
					setState(998);
					match(COMMA);
					setState(999);
					typeSpec(0);
					}
					}
					setState(1004);
					_errHandler.sync(this);
					_la = _input.LA(1);
				}
				}
			}

			setState(1007);
			match(RPAREN);
			setState(1008);
			match(ARROW);
			setState(1009);
			typeSpec(0);
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	@SuppressWarnings("CheckReturnValue")
	public static class PrimitiveTypeContext extends ParserRuleContext {
		public TerminalNode INT1() { return getToken(LuxParser.INT1, 0); }
		public TerminalNode INT8() { return getToken(LuxParser.INT8, 0); }
		public TerminalNode INT16() { return getToken(LuxParser.INT16, 0); }
		public TerminalNode INT32() { return getToken(LuxParser.INT32, 0); }
		public TerminalNode INT64() { return getToken(LuxParser.INT64, 0); }
		public TerminalNode INT128() { return getToken(LuxParser.INT128, 0); }
		public TerminalNode INTINF() { return getToken(LuxParser.INTINF, 0); }
		public TerminalNode ISIZE() { return getToken(LuxParser.ISIZE, 0); }
		public TerminalNode UINT1() { return getToken(LuxParser.UINT1, 0); }
		public TerminalNode UINT8() { return getToken(LuxParser.UINT8, 0); }
		public TerminalNode UINT16() { return getToken(LuxParser.UINT16, 0); }
		public TerminalNode UINT32() { return getToken(LuxParser.UINT32, 0); }
		public TerminalNode UINT64() { return getToken(LuxParser.UINT64, 0); }
		public TerminalNode UINT128() { return getToken(LuxParser.UINT128, 0); }
		public TerminalNode USIZE() { return getToken(LuxParser.USIZE, 0); }
		public TerminalNode FLOAT32() { return getToken(LuxParser.FLOAT32, 0); }
		public TerminalNode FLOAT64() { return getToken(LuxParser.FLOAT64, 0); }
		public TerminalNode FLOAT80() { return getToken(LuxParser.FLOAT80, 0); }
		public TerminalNode FLOAT128() { return getToken(LuxParser.FLOAT128, 0); }
		public TerminalNode DOUBLE() { return getToken(LuxParser.DOUBLE, 0); }
		public TerminalNode BOOL() { return getToken(LuxParser.BOOL, 0); }
		public TerminalNode CHAR() { return getToken(LuxParser.CHAR, 0); }
		public TerminalNode VOID() { return getToken(LuxParser.VOID, 0); }
		public TerminalNode STRING() { return getToken(LuxParser.STRING, 0); }
		public TerminalNode CSTRING() { return getToken(LuxParser.CSTRING, 0); }
		public PrimitiveTypeContext(ParserRuleContext parent, int invokingState) {
			super(parent, invokingState);
		}
		@Override public int getRuleIndex() { return RULE_primitiveType; }
		@Override
		public void enterRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).enterPrimitiveType(this);
		}
		@Override
		public void exitRule(ParseTreeListener listener) {
			if ( listener instanceof LuxParserListener ) ((LuxParserListener)listener).exitPrimitiveType(this);
		}
	}

	public final PrimitiveTypeContext primitiveType() throws RecognitionException {
		PrimitiveTypeContext _localctx = new PrimitiveTypeContext(_ctx, getState());
		enterRule(_localctx, 116, RULE_primitiveType);
		int _la;
		try {
			enterOuterAlt(_localctx, 1);
			{
			setState(1011);
			_la = _input.LA(1);
			if ( !(((((_la - 48)) & ~0x3f) == 0 && ((1L << (_la - 48)) & 33554431L) != 0)) ) {
			_errHandler.recoverInline(this);
			}
			else {
				if ( _input.LA(1)==Token.EOF ) matchedEOF = true;
				_errHandler.reportMatch(this);
				consume();
			}
			}
		}
		catch (RecognitionException re) {
			_localctx.exception = re;
			_errHandler.reportError(this, re);
			_errHandler.recover(this, re);
		}
		finally {
			exitRule();
		}
		return _localctx;
	}

	public boolean sempred(RuleContext _localctx, int ruleIndex, int predIndex) {
		switch (ruleIndex) {
		case 55:
			return expression_sempred((ExpressionContext)_localctx, predIndex);
		case 56:
			return typeSpec_sempred((TypeSpecContext)_localctx, predIndex);
		}
		return true;
	}
	private boolean expression_sempred(ExpressionContext _localctx, int predIndex) {
		switch (predIndex) {
		case 0:
			return precpred(_ctx, 32);
		case 1:
			return precpred(_ctx, 31);
		case 2:
			return precpred(_ctx, 30);
		case 3:
			return precpred(_ctx, 29);
		case 4:
			return precpred(_ctx, 28);
		case 5:
			return precpred(_ctx, 27);
		case 6:
			return precpred(_ctx, 26);
		case 7:
			return precpred(_ctx, 25);
		case 8:
			return precpred(_ctx, 24);
		case 9:
			return precpred(_ctx, 23);
		case 10:
			return precpred(_ctx, 22);
		case 11:
			return precpred(_ctx, 21);
		case 12:
			return precpred(_ctx, 20);
		case 13:
			return precpred(_ctx, 19);
		case 14:
			return precpred(_ctx, 18);
		case 15:
			return precpred(_ctx, 60);
		case 16:
			return precpred(_ctx, 59);
		case 17:
			return precpred(_ctx, 58);
		case 18:
			return precpred(_ctx, 57);
		case 19:
			return precpred(_ctx, 56);
		case 20:
			return precpred(_ctx, 55);
		case 21:
			return precpred(_ctx, 54);
		case 22:
			return precpred(_ctx, 53);
		case 23:
			return precpred(_ctx, 52);
		case 24:
			return precpred(_ctx, 51);
		case 25:
			return precpred(_ctx, 50);
		case 26:
			return precpred(_ctx, 49);
		case 27:
			return precpred(_ctx, 48);
		case 28:
			return precpred(_ctx, 47);
		}
		return true;
	}
	private boolean typeSpec_sempred(TypeSpecContext _localctx, int predIndex) {
		switch (predIndex) {
		case 29:
			return precpred(_ctx, 12);
		}
		return true;
	}

	public static final String _serializedATN =
		"\u0004\u0001\u0081\u03f6\u0002\u0000\u0007\u0000\u0002\u0001\u0007\u0001"+
		"\u0002\u0002\u0007\u0002\u0002\u0003\u0007\u0003\u0002\u0004\u0007\u0004"+
		"\u0002\u0005\u0007\u0005\u0002\u0006\u0007\u0006\u0002\u0007\u0007\u0007"+
		"\u0002\b\u0007\b\u0002\t\u0007\t\u0002\n\u0007\n\u0002\u000b\u0007\u000b"+
		"\u0002\f\u0007\f\u0002\r\u0007\r\u0002\u000e\u0007\u000e\u0002\u000f\u0007"+
		"\u000f\u0002\u0010\u0007\u0010\u0002\u0011\u0007\u0011\u0002\u0012\u0007"+
		"\u0012\u0002\u0013\u0007\u0013\u0002\u0014\u0007\u0014\u0002\u0015\u0007"+
		"\u0015\u0002\u0016\u0007\u0016\u0002\u0017\u0007\u0017\u0002\u0018\u0007"+
		"\u0018\u0002\u0019\u0007\u0019\u0002\u001a\u0007\u001a\u0002\u001b\u0007"+
		"\u001b\u0002\u001c\u0007\u001c\u0002\u001d\u0007\u001d\u0002\u001e\u0007"+
		"\u001e\u0002\u001f\u0007\u001f\u0002 \u0007 \u0002!\u0007!\u0002\"\u0007"+
		"\"\u0002#\u0007#\u0002$\u0007$\u0002%\u0007%\u0002&\u0007&\u0002\'\u0007"+
		"\'\u0002(\u0007(\u0002)\u0007)\u0002*\u0007*\u0002+\u0007+\u0002,\u0007"+
		",\u0002-\u0007-\u0002.\u0007.\u0002/\u0007/\u00020\u00070\u00021\u0007"+
		"1\u00022\u00072\u00023\u00073\u00024\u00074\u00025\u00075\u00026\u0007"+
		"6\u00027\u00077\u00028\u00078\u00029\u00079\u0002:\u0007:\u0001\u0000"+
		"\u0003\u0000x\b\u0000\u0001\u0000\u0005\u0000{\b\u0000\n\u0000\f\u0000"+
		"~\t\u0000\u0001\u0000\u0005\u0000\u0081\b\u0000\n\u0000\f\u0000\u0084"+
		"\t\u0000\u0001\u0000\u0001\u0000\u0001\u0001\u0001\u0001\u0003\u0001\u008a"+
		"\b\u0001\u0001\u0002\u0001\u0002\u0001\u0002\u0001\u0002\u0001\u0003\u0001"+
		"\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0001"+
		"\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0001\u0003\u0005"+
		"\u0003\u009d\b\u0003\n\u0003\f\u0003\u00a0\t\u0003\u0001\u0003\u0001\u0003"+
		"\u0001\u0003\u0003\u0003\u00a5\b\u0003\u0001\u0004\u0001\u0004\u0001\u0004"+
		"\u0005\u0004\u00aa\b\u0004\n\u0004\f\u0004\u00ad\t\u0004\u0001\u0005\u0001"+
		"\u0005\u0001\u0006\u0001\u0006\u0001\u0006\u0001\u0006\u0001\u0006\u0001"+
		"\u0006\u0001\u0006\u0003\u0006\u00b8\b\u0006\u0001\u0007\u0001\u0007\u0001"+
		"\u0007\u0001\u0007\u0001\u0007\u0001\u0007\u0001\b\u0001\b\u0001\b\u0001"+
		"\b\u0001\b\u0001\b\u0005\b\u00c6\b\b\n\b\f\b\u00c9\t\b\u0001\b\u0003\b"+
		"\u00cc\b\b\u0001\b\u0001\b\u0001\t\u0001\t\u0001\t\u0003\t\u00d3\b\t\u0001"+
		"\t\u0001\t\u0005\t\u00d7\b\t\n\t\f\t\u00da\t\t\u0001\t\u0001\t\u0001\n"+
		"\u0001\n\u0001\n\u0001\n\u0001\u000b\u0001\u000b\u0001\u000b\u0001\u000b"+
		"\u0005\u000b\u00e6\b\u000b\n\u000b\f\u000b\u00e9\t\u000b\u0001\u000b\u0001"+
		"\u000b\u0001\f\u0001\f\u0001\f\u0001\f\u0001\r\u0001\r\u0001\r\u0001\r"+
		"\u0001\r\u0003\r\u00f6\b\r\u0001\r\u0001\r\u0003\r\u00fa\b\r\u0001\r\u0001"+
		"\r\u0001\r\u0001\u000e\u0001\u000e\u0001\u000e\u0005\u000e\u0102\b\u000e"+
		"\n\u000e\f\u000e\u0105\t\u000e\u0001\u000f\u0001\u000f\u0003\u000f\u0109"+
		"\b\u000f\u0001\u0010\u0001\u0010\u0001\u0010\u0003\u0010\u010e\b\u0010"+
		"\u0001\u0010\u0001\u0010\u0003\u0010\u0112\b\u0010\u0001\u0010\u0001\u0010"+
		"\u0001\u0010\u0001\u0011\u0001\u0011\u0001\u0011\u0003\u0011\u011a\b\u0011"+
		"\u0001\u0011\u0001\u0011\u0005\u0011\u011e\b\u0011\n\u0011\f\u0011\u0121"+
		"\t\u0011\u0001\u0011\u0001\u0011\u0001\u0012\u0001\u0012\u0001\u0012\u0001"+
		"\u0012\u0005\u0012\u0129\b\u0012\n\u0012\f\u0012\u012c\t\u0012\u0001\u0012"+
		"\u0001\u0012\u0001\u0013\u0001\u0013\u0001\u0013\u0003\u0013\u0133\b\u0013"+
		"\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0014"+
		"\u0001\u0014\u0005\u0014\u013c\b\u0014\n\u0014\f\u0014\u013f\t\u0014\u0001"+
		"\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0001"+
		"\u0014\u0003\u0014\u0148\b\u0014\u0001\u0014\u0001\u0014\u0001\u0014\u0003"+
		"\u0014\u014d\b\u0014\u0001\u0015\u0001\u0015\u0001\u0015\u0005\u0015\u0152"+
		"\b\u0015\n\u0015\f\u0015\u0155\t\u0015\u0001\u0016\u0001\u0016\u0001\u0016"+
		"\u0001\u0016\u0001\u0016\u0001\u0016\u0001\u0016\u0003\u0016\u015e\b\u0016"+
		"\u0001\u0017\u0001\u0017\u0005\u0017\u0162\b\u0017\n\u0017\f\u0017\u0165"+
		"\t\u0017\u0001\u0017\u0001\u0017\u0001\u0018\u0001\u0018\u0001\u0018\u0001"+
		"\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001"+
		"\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001"+
		"\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001\u0018\u0001"+
		"\u0018\u0001\u0018\u0003\u0018\u0180\b\u0018\u0001\u0019\u0001\u0019\u0001"+
		"\u0019\u0001\u0019\u0003\u0019\u0186\b\u0019\u0001\u001a\u0001\u001a\u0001"+
		"\u001a\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0005"+
		"\u001b\u0190\b\u001b\n\u001b\f\u001b\u0193\t\u001b\u0001\u001b\u0001\u001b"+
		"\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b"+
		"\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b\u0001\u001b"+
		"\u0001\u001b\u0003\u001b\u01a4\b\u001b\u0001\u001c\u0001\u001c\u0001\u001c"+
		"\u0001\u001c\u0001\u001c\u0005\u001c\u01ab\b\u001c\n\u001c\f\u001c\u01ae"+
		"\t\u001c\u0001\u001c\u0001\u001c\u0001\u001c\u0001\u001c\u0001\u001d\u0001"+
		"\u001d\u0001\u001d\u0001\u001d\u0001\u001d\u0001\u001e\u0001\u001e\u0001"+
		"\u001e\u0004\u001e\u01bc\b\u001e\u000b\u001e\f\u001e\u01bd\u0001\u001e"+
		"\u0001\u001e\u0001\u001e\u0001\u001e\u0001\u001f\u0001\u001f\u0001\u001f"+
		"\u0001\u001f\u0001\u001f\u0004\u001f\u01c9\b\u001f\u000b\u001f\f\u001f"+
		"\u01ca\u0001\u001f\u0001\u001f\u0004\u001f\u01cf\b\u001f\u000b\u001f\f"+
		"\u001f\u01d0\u0001\u001f\u0001\u001f\u0001\u001f\u0001\u001f\u0001 \u0001"+
		" \u0001 \u0001 \u0001 \u0001 \u0001 \u0001 \u0001 \u0001 \u0001 \u0001"+
		" \u0001 \u0001 \u0003 \u01e5\b \u0001!\u0001!\u0001!\u0001!\u0001!\u0001"+
		"!\u0001!\u0001\"\u0001\"\u0001\"\u0001\"\u0001\"\u0001\"\u0001\"\u0001"+
		"#\u0001#\u0001#\u0003#\u01f8\b#\u0001#\u0001#\u0001#\u0001$\u0001$\u0001"+
		"$\u0005$\u0200\b$\n$\f$\u0203\t$\u0001%\u0001%\u0003%\u0207\b%\u0001%"+
		"\u0001%\u0001&\u0001&\u0001&\u0001&\u0001&\u0001&\u0005&\u0211\b&\n&\f"+
		"&\u0214\t&\u0001&\u0003&\u0217\b&\u0001&\u0001&\u0001&\u0001&\u0005&\u021d"+
		"\b&\n&\f&\u0220\t&\u0001&\u0003&\u0223\b&\u0003&\u0225\b&\u0001\'\u0001"+
		"\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001\'\u0001"+
		"\'\u0001\'\u0003\'\u0233\b\'\u0001(\u0001(\u0001(\u0001)\u0001)\u0001"+
		")\u0001)\u0001)\u0001)\u0001)\u0001)\u0001)\u0001)\u0001)\u0001)\u0001"+
		")\u0001)\u0001)\u0001)\u0001)\u0001)\u0003)\u024a\b)\u0001*\u0001*\u0001"+
		"*\u0001+\u0001+\u0001+\u0001,\u0001,\u0001,\u0001-\u0001-\u0001-\u0001"+
		"-\u0001-\u0001-\u0001-\u0001-\u0001-\u0001-\u0003-\u025f\b-\u0001.\u0001"+
		".\u0001.\u0001.\u0001.\u0001.\u0001.\u0001.\u0001.\u0001.\u0001.\u0001"+
		".\u0001.\u0001.\u0003.\u026f\b.\u0001/\u0001/\u0001/\u0001/\u0001/\u0001"+
		"/\u00010\u00010\u00010\u00040\u027a\b0\u000b0\f0\u027b\u00010\u00030\u027f"+
		"\b0\u00010\u00010\u00010\u00010\u00030\u0285\b0\u00011\u00011\u00011\u0001"+
		"1\u00011\u00011\u00011\u00012\u00012\u00012\u00013\u00013\u00013\u0001"+
		"3\u00014\u00014\u00014\u00014\u00054\u0299\b4\n4\f4\u029c\t4\u00014\u0003"+
		"4\u029f\b4\u00014\u00014\u00014\u00014\u00014\u00014\u00014\u00014\u0005"+
		"4\u02a9\b4\n4\f4\u02ac\t4\u00014\u00034\u02af\b4\u00014\u00014\u00034"+
		"\u02b3\b4\u00015\u00015\u00015\u00015\u00055\u02b9\b5\n5\f5\u02bc\t5\u0001"+
		"5\u00015\u00016\u00016\u00016\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00057\u02cd\b7\n7\f7\u02d0\t7\u00037\u02d2"+
		"\b7\u00017\u00017\u00017\u00017\u00017\u00017\u00037\u02da\b7\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00057\u0304"+
		"\b7\n7\f7\u0307\t7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00037\u031b\b7\u00017\u00017\u00017\u00017\u00017\u00017\u00057\u0323"+
		"\b7\n7\f7\u0326\t7\u00037\u0328\b7\u00017\u00017\u00017\u00017\u00017"+
		"\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00037\u0336\b7\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00037\u036e\b7\u00017\u00017\u00017\u00017\u0003"+
		"7\u0374\b7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00037\u0385\b7\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u00017\u0001"+
		"7\u00017\u00017\u00017\u00057\u03a0\b7\n7\f7\u03a3\t7\u00018\u00018\u0001"+
		"8\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u0001"+
		"8\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u0001"+
		"8\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u00018\u0001"+
		"8\u00048\u03c6\b8\u000b8\f8\u03c7\u00018\u00018\u00018\u00018\u00018\u0001"+
		"8\u00018\u00058\u03d1\b8\n8\f8\u03d4\t8\u00018\u00018\u00018\u00018\u0001"+
		"8\u00038\u03db\b8\u00018\u00018\u00058\u03df\b8\n8\f8\u03e2\t8\u00019"+
		"\u00019\u00019\u00019\u00019\u00059\u03e9\b9\n9\f9\u03ec\t9\u00039\u03ee"+
		"\b9\u00019\u00019\u00019\u00019\u0001:\u0001:\u0001:\u0000\u0002np;\u0000"+
		"\u0002\u0004\u0006\b\n\f\u000e\u0010\u0012\u0014\u0016\u0018\u001a\u001c"+
		"\u001e \"$&(*,.02468:<>@BDFHJLNPRTVXZ\\^`bdfhjlnprt\u0000\u0007\u0001"+
		"\u0000*+\u0001\u0000S\\\u0002\u0000iimn\u0001\u0000kl\u0001\u0000qt\u0001"+
		"\u0000op\u0001\u00000H\u0465\u0000w\u0001\u0000\u0000\u0000\u0002\u0089"+
		"\u0001\u0000\u0000\u0000\u0004\u008b\u0001\u0000\u0000\u0000\u0006\u00a4"+
		"\u0001\u0000\u0000\u0000\b\u00a6\u0001\u0000\u0000\u0000\n\u00ae\u0001"+
		"\u0000\u0000\u0000\f\u00b7\u0001\u0000\u0000\u0000\u000e\u00b9\u0001\u0000"+
		"\u0000\u0000\u0010\u00bf\u0001\u0000\u0000\u0000\u0012\u00cf\u0001\u0000"+
		"\u0000\u0000\u0014\u00dd\u0001\u0000\u0000\u0000\u0016\u00e1\u0001\u0000"+
		"\u0000\u0000\u0018\u00ec\u0001\u0000\u0000\u0000\u001a\u00f0\u0001\u0000"+
		"\u0000\u0000\u001c\u00fe\u0001\u0000\u0000\u0000\u001e\u0106\u0001\u0000"+
		"\u0000\u0000 \u010a\u0001\u0000\u0000\u0000\"\u0116\u0001\u0000\u0000"+
		"\u0000$\u0124\u0001\u0000\u0000\u0000&\u012f\u0001\u0000\u0000\u0000("+
		"\u014c\u0001\u0000\u0000\u0000*\u014e\u0001\u0000\u0000\u0000,\u015d\u0001"+
		"\u0000\u0000\u0000.\u015f\u0001\u0000\u0000\u00000\u017f\u0001\u0000\u0000"+
		"\u00002\u0185\u0001\u0000\u0000\u00004\u0187\u0001\u0000\u0000\u00006"+
		"\u01a3\u0001\u0000\u0000\u00008\u01a5\u0001\u0000\u0000\u0000:\u01b3\u0001"+
		"\u0000\u0000\u0000<\u01b8\u0001\u0000\u0000\u0000>\u01c3\u0001\u0000\u0000"+
		"\u0000@\u01e4\u0001\u0000\u0000\u0000B\u01e6\u0001\u0000\u0000\u0000D"+
		"\u01ed\u0001\u0000\u0000\u0000F\u01f4\u0001\u0000\u0000\u0000H\u01fc\u0001"+
		"\u0000\u0000\u0000J\u0204\u0001\u0000\u0000\u0000L\u0224\u0001\u0000\u0000"+
		"\u0000N\u0232\u0001\u0000\u0000\u0000P\u0234\u0001\u0000\u0000\u0000R"+
		"\u0249\u0001\u0000\u0000\u0000T\u024b\u0001\u0000\u0000\u0000V\u024e\u0001"+
		"\u0000\u0000\u0000X\u0251\u0001\u0000\u0000\u0000Z\u025e\u0001\u0000\u0000"+
		"\u0000\\\u026e\u0001\u0000\u0000\u0000^\u0270\u0001\u0000\u0000\u0000"+
		"`\u0284\u0001\u0000\u0000\u0000b\u0286\u0001\u0000\u0000\u0000d\u028d"+
		"\u0001\u0000\u0000\u0000f\u0290\u0001\u0000\u0000\u0000h\u02b2\u0001\u0000"+
		"\u0000\u0000j\u02b4\u0001\u0000\u0000\u0000l\u02bf\u0001\u0000\u0000\u0000"+
		"n\u0335\u0001\u0000\u0000\u0000p\u03da\u0001\u0000\u0000\u0000r\u03e3"+
		"\u0001\u0000\u0000\u0000t\u03f3\u0001\u0000\u0000\u0000vx\u0003\u0004"+
		"\u0002\u0000wv\u0001\u0000\u0000\u0000wx\u0001\u0000\u0000\u0000x|\u0001"+
		"\u0000\u0000\u0000y{\u0003\u0002\u0001\u0000zy\u0001\u0000\u0000\u0000"+
		"{~\u0001\u0000\u0000\u0000|z\u0001\u0000\u0000\u0000|}\u0001\u0000\u0000"+
		"\u0000}\u0082\u0001\u0000\u0000\u0000~|\u0001\u0000\u0000\u0000\u007f"+
		"\u0081\u0003\f\u0006\u0000\u0080\u007f\u0001\u0000\u0000\u0000\u0081\u0084"+
		"\u0001\u0000\u0000\u0000\u0082\u0080\u0001\u0000\u0000\u0000\u0082\u0083"+
		"\u0001\u0000\u0000\u0000\u0083\u0085\u0001\u0000\u0000\u0000\u0084\u0082"+
		"\u0001\u0000\u0000\u0000\u0085\u0086\u0005\u0000\u0000\u0001\u0086\u0001"+
		"\u0001\u0000\u0000\u0000\u0087\u008a\u0003\u0006\u0003\u0000\u0088\u008a"+
		"\u0003\n\u0005\u0000\u0089\u0087\u0001\u0000\u0000\u0000\u0089\u0088\u0001"+
		"\u0000\u0000\u0000\u008a\u0003\u0001\u0000\u0000\u0000\u008b\u008c\u0005"+
		"\u0001\u0000\u0000\u008c\u008d\u0005R\u0000\u0000\u008d\u008e\u0005]\u0000"+
		"\u0000\u008e\u0005\u0001\u0000\u0000\u0000\u008f\u0090\u0005\u0002\u0000"+
		"\u0000\u0090\u0091\u0003\b\u0004\u0000\u0091\u0092\u0005_\u0000\u0000"+
		"\u0092\u0093\u0005R\u0000\u0000\u0093\u0094\u0005]\u0000\u0000\u0094\u00a5"+
		"\u0001\u0000\u0000\u0000\u0095\u0096\u0005\u0002\u0000\u0000\u0096\u0097"+
		"\u0003\b\u0004\u0000\u0097\u0098\u0005_\u0000\u0000\u0098\u0099\u0005"+
		"e\u0000\u0000\u0099\u009e\u0005R\u0000\u0000\u009a\u009b\u0005`\u0000"+
		"\u0000\u009b\u009d\u0005R\u0000\u0000\u009c\u009a\u0001\u0000\u0000\u0000"+
		"\u009d\u00a0\u0001\u0000\u0000\u0000\u009e\u009c\u0001\u0000\u0000\u0000"+
		"\u009e\u009f\u0001\u0000\u0000\u0000\u009f\u00a1\u0001\u0000\u0000\u0000"+
		"\u00a0\u009e\u0001\u0000\u0000\u0000\u00a1\u00a2\u0005f\u0000\u0000\u00a2"+
		"\u00a3\u0005]\u0000\u0000\u00a3\u00a5\u0001\u0000\u0000\u0000\u00a4\u008f"+
		"\u0001\u0000\u0000\u0000\u00a4\u0095\u0001\u0000\u0000\u0000\u00a5\u0007"+
		"\u0001\u0000\u0000\u0000\u00a6\u00ab\u0005R\u0000\u0000\u00a7\u00a8\u0005"+
		"_\u0000\u0000\u00a8\u00aa\u0005R\u0000\u0000\u00a9\u00a7\u0001\u0000\u0000"+
		"\u0000\u00aa\u00ad\u0001\u0000\u0000\u0000\u00ab\u00a9\u0001\u0000\u0000"+
		"\u0000\u00ab\u00ac\u0001\u0000\u0000\u0000\u00ac\t\u0001\u0000\u0000\u0000"+
		"\u00ad\u00ab\u0001\u0000\u0000\u0000\u00ae\u00af\u0007\u0000\u0000\u0000"+
		"\u00af\u000b\u0001\u0000\u0000\u0000\u00b0\u00b8\u0003\u000e\u0007\u0000"+
		"\u00b1\u00b8\u0003\u0012\t\u0000\u00b2\u00b8\u0003\u0016\u000b\u0000\u00b3"+
		"\u00b8\u0003\u0010\b\u0000\u00b4\u00b8\u0003\"\u0011\u0000\u00b5\u00b8"+
		"\u0003\u001a\r\u0000\u00b6\u00b8\u0003 \u0010\u0000\u00b7\u00b0\u0001"+
		"\u0000\u0000\u0000\u00b7\u00b1\u0001\u0000\u0000\u0000\u00b7\u00b2\u0001"+
		"\u0000\u0000\u0000\u00b7\u00b3\u0001\u0000\u0000\u0000\u00b7\u00b4\u0001"+
		"\u0000\u0000\u0000\u00b7\u00b5\u0001\u0000\u0000\u0000\u00b7\u00b6\u0001"+
		"\u0000\u0000\u0000\u00b8\r\u0001\u0000\u0000\u0000\u00b9\u00ba\u0005\t"+
		"\u0000\u0000\u00ba\u00bb\u0005R\u0000\u0000\u00bb\u00bc\u0005b\u0000\u0000"+
		"\u00bc\u00bd\u0003p8\u0000\u00bd\u00be\u0005]\u0000\u0000\u00be\u000f"+
		"\u0001\u0000\u0000\u0000\u00bf\u00c0\u0005\u0006\u0000\u0000\u00c0\u00c1"+
		"\u0005R\u0000\u0000\u00c1\u00c2\u0005e\u0000\u0000\u00c2\u00c7\u0005R"+
		"\u0000\u0000\u00c3\u00c4\u0005`\u0000\u0000\u00c4\u00c6\u0005R\u0000\u0000"+
		"\u00c5\u00c3\u0001\u0000\u0000\u0000\u00c6\u00c9\u0001\u0000\u0000\u0000"+
		"\u00c7\u00c5\u0001\u0000\u0000\u0000\u00c7\u00c8\u0001\u0000\u0000\u0000"+
		"\u00c8\u00cb\u0001\u0000\u0000\u0000\u00c9\u00c7\u0001\u0000\u0000\u0000"+
		"\u00ca\u00cc\u0005`\u0000\u0000\u00cb\u00ca\u0001\u0000\u0000\u0000\u00cb"+
		"\u00cc\u0001\u0000\u0000\u0000\u00cc\u00cd\u0001\u0000\u0000\u0000\u00cd"+
		"\u00ce\u0005f\u0000\u0000\u00ce\u0011\u0001\u0000\u0000\u0000\u00cf\u00d0"+
		"\u0005\u0004\u0000\u0000\u00d0\u00d2\u0005R\u0000\u0000\u00d1\u00d3\u0003"+
		"$\u0012\u0000\u00d2\u00d1\u0001\u0000\u0000\u0000\u00d2\u00d3\u0001\u0000"+
		"\u0000\u0000\u00d3\u00d4\u0001\u0000\u0000\u0000\u00d4\u00d8\u0005e\u0000"+
		"\u0000\u00d5\u00d7\u0003\u0014\n\u0000\u00d6\u00d5\u0001\u0000\u0000\u0000"+
		"\u00d7\u00da\u0001\u0000\u0000\u0000\u00d8\u00d6\u0001\u0000\u0000\u0000"+
		"\u00d8\u00d9\u0001\u0000\u0000\u0000\u00d9\u00db\u0001\u0000\u0000\u0000"+
		"\u00da\u00d8\u0001\u0000\u0000\u0000\u00db\u00dc\u0005f\u0000\u0000\u00dc"+
		"\u0013\u0001\u0000\u0000\u0000\u00dd\u00de\u0003p8\u0000\u00de\u00df\u0005"+
		"R\u0000\u0000\u00df\u00e0\u0005]\u0000\u0000\u00e0\u0015\u0001\u0000\u0000"+
		"\u0000\u00e1\u00e2\u0005\u0005\u0000\u0000\u00e2\u00e3\u0005R\u0000\u0000"+
		"\u00e3\u00e7\u0005e\u0000\u0000\u00e4\u00e6\u0003\u0018\f\u0000\u00e5"+
		"\u00e4\u0001\u0000\u0000\u0000\u00e6\u00e9\u0001\u0000\u0000\u0000\u00e7"+
		"\u00e5\u0001\u0000\u0000\u0000\u00e7\u00e8\u0001\u0000\u0000\u0000\u00e8"+
		"\u00ea\u0001\u0000\u0000\u0000\u00e9\u00e7\u0001\u0000\u0000\u0000\u00ea"+
		"\u00eb\u0005f\u0000\u0000\u00eb\u0017\u0001\u0000\u0000\u0000\u00ec\u00ed"+
		"\u0003p8\u0000\u00ed\u00ee\u0005R\u0000\u0000\u00ee\u00ef\u0005]\u0000"+
		"\u0000\u00ef\u0019\u0001\u0000\u0000\u0000\u00f0\u00f1\u0005#\u0000\u0000"+
		"\u00f1\u00f2\u0003p8\u0000\u00f2\u00f3\u0005R\u0000\u0000\u00f3\u00f5"+
		"\u0005c\u0000\u0000\u00f4\u00f6\u0003\u001c\u000e\u0000\u00f5\u00f4\u0001"+
		"\u0000\u0000\u0000\u00f5\u00f6\u0001\u0000\u0000\u0000\u00f6\u00f9\u0001"+
		"\u0000\u0000\u0000\u00f7\u00f8\u0005`\u0000\u0000\u00f8\u00fa\u0005-\u0000"+
		"\u0000\u00f9\u00f7\u0001\u0000\u0000\u0000\u00f9\u00fa\u0001\u0000\u0000"+
		"\u0000\u00fa\u00fb\u0001\u0000\u0000\u0000\u00fb\u00fc\u0005d\u0000\u0000"+
		"\u00fc\u00fd\u0005]\u0000\u0000\u00fd\u001b\u0001\u0000\u0000\u0000\u00fe"+
		"\u0103\u0003\u001e\u000f\u0000\u00ff\u0100\u0005`\u0000\u0000\u0100\u0102"+
		"\u0003\u001e\u000f\u0000\u0101\u00ff\u0001\u0000\u0000\u0000\u0102\u0105"+
		"\u0001\u0000\u0000\u0000\u0103\u0101\u0001\u0000\u0000\u0000\u0103\u0104"+
		"\u0001\u0000\u0000\u0000\u0104\u001d\u0001\u0000\u0000\u0000\u0105\u0103"+
		"\u0001\u0000\u0000\u0000\u0106\u0108\u0003p8\u0000\u0107\u0109\u0005R"+
		"\u0000\u0000\u0108\u0107\u0001\u0000\u0000\u0000\u0108\u0109\u0001\u0000"+
		"\u0000\u0000\u0109\u001f\u0001\u0000\u0000\u0000\u010a\u010b\u0003p8\u0000"+
		"\u010b\u010d\u0005R\u0000\u0000\u010c\u010e\u0003$\u0012\u0000\u010d\u010c"+
		"\u0001\u0000\u0000\u0000\u010d\u010e\u0001\u0000\u0000\u0000\u010e\u010f"+
		"\u0001\u0000\u0000\u0000\u010f\u0111\u0005c\u0000\u0000\u0110\u0112\u0003"+
		"*\u0015\u0000\u0111\u0110\u0001\u0000\u0000\u0000\u0111\u0112\u0001\u0000"+
		"\u0000\u0000\u0112\u0113\u0001\u0000\u0000\u0000\u0113\u0114\u0005d\u0000"+
		"\u0000\u0114\u0115\u0003.\u0017\u0000\u0115!\u0001\u0000\u0000\u0000\u0116"+
		"\u0117\u0005\u001d\u0000\u0000\u0117\u0119\u0005R\u0000\u0000\u0118\u011a"+
		"\u0003$\u0012\u0000\u0119\u0118\u0001\u0000\u0000\u0000\u0119\u011a\u0001"+
		"\u0000\u0000\u0000\u011a\u011b\u0001\u0000\u0000\u0000\u011b\u011f\u0005"+
		"e\u0000\u0000\u011c\u011e\u0003(\u0014\u0000\u011d\u011c\u0001\u0000\u0000"+
		"\u0000\u011e\u0121\u0001\u0000\u0000\u0000\u011f\u011d\u0001\u0000\u0000"+
		"\u0000\u011f\u0120\u0001\u0000\u0000\u0000\u0120\u0122\u0001\u0000\u0000"+
		"\u0000\u0121\u011f\u0001\u0000\u0000\u0000\u0122\u0123\u0005f\u0000\u0000"+
		"\u0123#\u0001\u0000\u0000\u0000\u0124\u0125\u0005s\u0000\u0000\u0125\u012a"+
		"\u0003&\u0013\u0000\u0126\u0127\u0005`\u0000\u0000\u0127\u0129\u0003&"+
		"\u0013\u0000\u0128\u0126\u0001\u0000\u0000\u0000\u0129\u012c\u0001\u0000"+
		"\u0000\u0000\u012a\u0128\u0001\u0000\u0000\u0000\u012a\u012b\u0001\u0000"+
		"\u0000\u0000\u012b\u012d\u0001\u0000\u0000\u0000\u012c\u012a\u0001\u0000"+
		"\u0000\u0000\u012d\u012e\u0005t\u0000\u0000\u012e%\u0001\u0000\u0000\u0000"+
		"\u012f\u0132\u0005R\u0000\u0000\u0130\u0131\u0005^\u0000\u0000\u0131\u0133"+
		"\u0005R\u0000\u0000\u0132\u0130\u0001\u0000\u0000\u0000\u0132\u0133\u0001"+
		"\u0000\u0000\u0000\u0133\'\u0001\u0000\u0000\u0000\u0134\u0135\u0003p"+
		"8\u0000\u0135\u0136\u0005R\u0000\u0000\u0136\u0137\u0005c\u0000\u0000"+
		"\u0137\u0138\u0005j\u0000\u0000\u0138\u013d\u0005R\u0000\u0000\u0139\u013a"+
		"\u0005`\u0000\u0000\u013a\u013c\u0003,\u0016\u0000\u013b\u0139\u0001\u0000"+
		"\u0000\u0000\u013c\u013f\u0001\u0000\u0000\u0000\u013d\u013b\u0001\u0000"+
		"\u0000\u0000\u013d\u013e\u0001\u0000\u0000\u0000\u013e\u0140\u0001\u0000"+
		"\u0000\u0000\u013f\u013d\u0001\u0000\u0000\u0000\u0140\u0141\u0005d\u0000"+
		"\u0000\u0141\u0142\u0003.\u0017\u0000\u0142\u014d\u0001\u0000\u0000\u0000"+
		"\u0143\u0144\u0003p8\u0000\u0144\u0145\u0005R\u0000\u0000\u0145\u0147"+
		"\u0005c\u0000\u0000\u0146\u0148\u0003*\u0015\u0000\u0147\u0146\u0001\u0000"+
		"\u0000\u0000\u0147\u0148\u0001\u0000\u0000\u0000\u0148\u0149\u0001\u0000"+
		"\u0000\u0000\u0149\u014a\u0005d\u0000\u0000\u014a\u014b\u0003.\u0017\u0000"+
		"\u014b\u014d\u0001\u0000\u0000\u0000\u014c\u0134\u0001\u0000\u0000\u0000"+
		"\u014c\u0143\u0001\u0000\u0000\u0000\u014d)\u0001\u0000\u0000\u0000\u014e"+
		"\u0153\u0003,\u0016\u0000\u014f\u0150\u0005`\u0000\u0000\u0150\u0152\u0003"+
		",\u0016\u0000\u0151\u014f\u0001\u0000\u0000\u0000\u0152\u0155\u0001\u0000"+
		"\u0000\u0000\u0153\u0151\u0001\u0000\u0000\u0000\u0153\u0154\u0001\u0000"+
		"\u0000\u0000\u0154+\u0001\u0000\u0000\u0000\u0155\u0153\u0001\u0000\u0000"+
		"\u0000\u0156\u0157\u0003p8\u0000\u0157\u0158\u0005-\u0000\u0000\u0158"+
		"\u0159\u0005R\u0000\u0000\u0159\u015e\u0001\u0000\u0000\u0000\u015a\u015b"+
		"\u0003p8\u0000\u015b\u015c\u0005R\u0000\u0000\u015c\u015e\u0001\u0000"+
		"\u0000\u0000\u015d\u0156\u0001\u0000\u0000\u0000\u015d\u015a\u0001\u0000"+
		"\u0000\u0000\u015e-\u0001\u0000\u0000\u0000\u015f\u0163\u0005e\u0000\u0000"+
		"\u0160\u0162\u00030\u0018\u0000\u0161\u0160\u0001\u0000\u0000\u0000\u0162"+
		"\u0165\u0001\u0000\u0000\u0000\u0163\u0161\u0001\u0000\u0000\u0000\u0163"+
		"\u0164\u0001\u0000\u0000\u0000\u0164\u0166\u0001\u0000\u0000\u0000\u0165"+
		"\u0163\u0001\u0000\u0000\u0000\u0166\u0167\u0005f\u0000\u0000\u0167/\u0001"+
		"\u0000\u0000\u0000\u0168\u0180\u00036\u001b\u0000\u0169\u0180\u00038\u001c"+
		"\u0000\u016a\u0180\u0003:\u001d\u0000\u016b\u0180\u0003@ \u0000\u016c"+
		"\u0180\u0003>\u001f\u0000\u016d\u0180\u0003<\u001e\u0000\u016e\u0180\u0003"+
		"B!\u0000\u016f\u0180\u0003D\"\u0000\u0170\u0180\u0003F#\u0000\u0171\u0180"+
		"\u00034\u001a\u0000\u0172\u0180\u0003J%\u0000\u0173\u0180\u0003L&\u0000"+
		"\u0174\u0180\u0003R)\u0000\u0175\u0180\u0003X,\u0000\u0176\u0180\u0003"+
		"Z-\u0000\u0177\u0180\u0003\\.\u0000\u0178\u0180\u0003T*\u0000\u0179\u0180"+
		"\u0003V+\u0000\u017a\u0180\u0003h4\u0000\u017b\u0180\u0003^/\u0000\u017c"+
		"\u0180\u0003`0\u0000\u017d\u0180\u0003f3\u0000\u017e\u0180\u00032\u0019"+
		"\u0000\u017f\u0168\u0001\u0000\u0000\u0000\u017f\u0169\u0001\u0000\u0000"+
		"\u0000\u017f\u016a\u0001\u0000\u0000\u0000\u017f\u016b\u0001\u0000\u0000"+
		"\u0000\u017f\u016c\u0001\u0000\u0000\u0000\u017f\u016d\u0001\u0000\u0000"+
		"\u0000\u017f\u016e\u0001\u0000\u0000\u0000\u017f\u016f\u0001\u0000\u0000"+
		"\u0000\u017f\u0170\u0001\u0000\u0000\u0000\u017f\u0171\u0001\u0000\u0000"+
		"\u0000\u017f\u0172\u0001\u0000\u0000\u0000\u017f\u0173\u0001\u0000\u0000"+
		"\u0000\u017f\u0174\u0001\u0000\u0000\u0000\u017f\u0175\u0001\u0000\u0000"+
		"\u0000\u017f\u0176\u0001\u0000\u0000\u0000\u017f\u0177\u0001\u0000\u0000"+
		"\u0000\u017f\u0178\u0001\u0000\u0000\u0000\u017f\u0179\u0001\u0000\u0000"+
		"\u0000\u017f\u017a\u0001\u0000\u0000\u0000\u017f\u017b\u0001\u0000\u0000"+
		"\u0000\u017f\u017c\u0001\u0000\u0000\u0000\u017f\u017d\u0001\u0000\u0000"+
		"\u0000\u017f\u017e\u0001\u0000\u0000\u0000\u01801\u0001\u0000\u0000\u0000"+
		"\u0181\u0182\u0005\"\u0000\u0000\u0182\u0186\u0003F#\u0000\u0183\u0184"+
		"\u0005\"\u0000\u0000\u0184\u0186\u00034\u001a\u0000\u0185\u0181\u0001"+
		"\u0000\u0000\u0000\u0185\u0183\u0001\u0000\u0000\u0000\u01863\u0001\u0000"+
		"\u0000\u0000\u0187\u0188\u0003n7\u0000\u0188\u0189\u0005]\u0000\u0000"+
		"\u01895\u0001\u0000\u0000\u0000\u018a\u018b\u0003p8\u0000\u018b\u018c"+
		"\u0005c\u0000\u0000\u018c\u0191\u0005R\u0000\u0000\u018d\u018e\u0005`"+
		"\u0000\u0000\u018e\u0190\u0005R\u0000\u0000\u018f\u018d\u0001\u0000\u0000"+
		"\u0000\u0190\u0193\u0001\u0000\u0000\u0000\u0191\u018f\u0001\u0000\u0000"+
		"\u0000\u0191\u0192\u0001\u0000\u0000\u0000\u0192\u0194\u0001\u0000\u0000"+
		"\u0000\u0193\u0191\u0001\u0000\u0000\u0000\u0194\u0195\u0005d\u0000\u0000"+
		"\u0195\u0196\u0005b\u0000\u0000\u0196\u0197\u0003n7\u0000\u0197\u0198"+
		"\u0005]\u0000\u0000\u0198\u01a4\u0001\u0000\u0000\u0000\u0199\u019a\u0003"+
		"p8\u0000\u019a\u019b\u0005R\u0000\u0000\u019b\u019c\u0005b\u0000\u0000"+
		"\u019c\u019d\u0003n7\u0000\u019d\u019e\u0005]\u0000\u0000\u019e\u01a4"+
		"\u0001\u0000\u0000\u0000\u019f\u01a0\u0003p8\u0000\u01a0\u01a1\u0005R"+
		"\u0000\u0000\u01a1\u01a2\u0005]\u0000\u0000\u01a2\u01a4\u0001\u0000\u0000"+
		"\u0000\u01a3\u018a\u0001\u0000\u0000\u0000\u01a3\u0199\u0001\u0000\u0000"+
		"\u0000\u01a3\u019f\u0001\u0000\u0000\u0000\u01a47\u0001\u0000\u0000\u0000"+
		"\u01a5\u01ac\u0005R\u0000\u0000\u01a6\u01a7\u0005g\u0000\u0000\u01a7\u01a8"+
		"\u0003n7\u0000\u01a8\u01a9\u0005h\u0000\u0000\u01a9\u01ab\u0001\u0000"+
		"\u0000\u0000\u01aa\u01a6\u0001\u0000\u0000\u0000\u01ab\u01ae\u0001\u0000"+
		"\u0000\u0000\u01ac\u01aa\u0001\u0000\u0000\u0000\u01ac\u01ad\u0001\u0000"+
		"\u0000\u0000\u01ad\u01af\u0001\u0000\u0000\u0000\u01ae\u01ac\u0001\u0000"+
		"\u0000\u0000\u01af\u01b0\u0005b\u0000\u0000\u01b0\u01b1\u0003n7\u0000"+
		"\u01b1\u01b2\u0005]\u0000\u0000\u01b29\u0001\u0000\u0000\u0000\u01b3\u01b4"+
		"\u0005R\u0000\u0000\u01b4\u01b5\u0007\u0001\u0000\u0000\u01b5\u01b6\u0003"+
		"n7\u0000\u01b6\u01b7\u0005]\u0000\u0000\u01b7;\u0001\u0000\u0000\u0000"+
		"\u01b8\u01bb\u0005R\u0000\u0000\u01b9\u01ba\u0005a\u0000\u0000\u01ba\u01bc"+
		"\u0005R\u0000\u0000\u01bb\u01b9\u0001\u0000\u0000\u0000\u01bc\u01bd\u0001"+
		"\u0000\u0000\u0000\u01bd\u01bb\u0001\u0000\u0000\u0000\u01bd\u01be\u0001"+
		"\u0000\u0000\u0000\u01be\u01bf\u0001\u0000\u0000\u0000\u01bf\u01c0\u0005"+
		"b\u0000\u0000\u01c0\u01c1\u0003n7\u0000\u01c1\u01c2\u0005]\u0000\u0000"+
		"\u01c2=\u0001\u0000\u0000\u0000\u01c3\u01c8\u0005R\u0000\u0000\u01c4\u01c5"+
		"\u0005g\u0000\u0000\u01c5\u01c6\u0003n7\u0000\u01c6\u01c7\u0005h\u0000"+
		"\u0000\u01c7\u01c9\u0001\u0000\u0000\u0000\u01c8\u01c4\u0001\u0000\u0000"+
		"\u0000\u01c9\u01ca\u0001\u0000\u0000\u0000\u01ca\u01c8\u0001\u0000\u0000"+
		"\u0000\u01ca\u01cb\u0001\u0000\u0000\u0000\u01cb\u01ce\u0001\u0000\u0000"+
		"\u0000\u01cc\u01cd\u0005a\u0000\u0000\u01cd\u01cf\u0005R\u0000\u0000\u01ce"+
		"\u01cc\u0001\u0000\u0000\u0000\u01cf\u01d0\u0001\u0000\u0000\u0000\u01d0"+
		"\u01ce\u0001\u0000\u0000\u0000\u01d0\u01d1\u0001\u0000\u0000\u0000\u01d1"+
		"\u01d2\u0001\u0000\u0000\u0000\u01d2\u01d3\u0005b\u0000\u0000\u01d3\u01d4"+
		"\u0003n7\u0000\u01d4\u01d5\u0005]\u0000\u0000\u01d5?\u0001\u0000\u0000"+
		"\u0000\u01d6\u01d7\u0005i\u0000\u0000\u01d7\u01d8\u0005R\u0000\u0000\u01d8"+
		"\u01d9\u0005b\u0000\u0000\u01d9\u01da\u0003n7\u0000\u01da\u01db\u0005"+
		"]\u0000\u0000\u01db\u01e5\u0001\u0000\u0000\u0000\u01dc\u01dd\u0005i\u0000"+
		"\u0000\u01dd\u01de\u0005c\u0000\u0000\u01de\u01df\u0003n7\u0000\u01df"+
		"\u01e0\u0005d\u0000\u0000\u01e0\u01e1\u0005b\u0000\u0000\u01e1\u01e2\u0003"+
		"n7\u0000\u01e2\u01e3\u0005]\u0000\u0000\u01e3\u01e5\u0001\u0000\u0000"+
		"\u0000\u01e4\u01d6\u0001\u0000\u0000\u0000\u01e4\u01dc\u0001\u0000\u0000"+
		"\u0000\u01e5A\u0001\u0000\u0000\u0000\u01e6\u01e7\u0005R\u0000\u0000\u01e7"+
		"\u01e8\u0005)\u0000\u0000\u01e8\u01e9\u0005R\u0000\u0000\u01e9\u01ea\u0005"+
		"b\u0000\u0000\u01ea\u01eb\u0003n7\u0000\u01eb\u01ec\u0005]\u0000\u0000"+
		"\u01ecC\u0001\u0000\u0000\u0000\u01ed\u01ee\u0005R\u0000\u0000\u01ee\u01ef"+
		"\u0005)\u0000\u0000\u01ef\u01f0\u0005R\u0000\u0000\u01f0\u01f1\u0007\u0001"+
		"\u0000\u0000\u01f1\u01f2\u0003n7\u0000\u01f2\u01f3\u0005]\u0000\u0000"+
		"\u01f3E\u0001\u0000\u0000\u0000\u01f4\u01f5\u0005R\u0000\u0000\u01f5\u01f7"+
		"\u0005c\u0000\u0000\u01f6\u01f8\u0003H$\u0000\u01f7\u01f6\u0001\u0000"+
		"\u0000\u0000\u01f7\u01f8\u0001\u0000\u0000\u0000\u01f8\u01f9\u0001\u0000"+
		"\u0000\u0000\u01f9\u01fa\u0005d\u0000\u0000\u01fa\u01fb\u0005]\u0000\u0000"+
		"\u01fbG\u0001\u0000\u0000\u0000\u01fc\u0201\u0003n7\u0000\u01fd\u01fe"+
		"\u0005`\u0000\u0000\u01fe\u0200\u0003n7\u0000\u01ff\u01fd\u0001\u0000"+
		"\u0000\u0000\u0200\u0203\u0001\u0000\u0000\u0000\u0201\u01ff\u0001\u0000"+
		"\u0000\u0000\u0201\u0202\u0001\u0000\u0000\u0000\u0202I\u0001\u0000\u0000"+
		"\u0000\u0203\u0201\u0001\u0000\u0000\u0000\u0204\u0206\u0005\u0003\u0000"+
		"\u0000\u0205\u0207\u0003n7\u0000\u0206\u0205\u0001\u0000\u0000\u0000\u0206"+
		"\u0207\u0001\u0000\u0000\u0000\u0207\u0208\u0001\u0000\u0000\u0000\u0208"+
		"\u0209\u0005]\u0000\u0000\u0209K\u0001\u0000\u0000\u0000\u020a\u020b\u0005"+
		"\u000e\u0000\u0000\u020b\u020c\u0005c\u0000\u0000\u020c\u020d\u0003n7"+
		"\u0000\u020d\u020e\u0005d\u0000\u0000\u020e\u0212\u0003.\u0017\u0000\u020f"+
		"\u0211\u0003N\'\u0000\u0210\u020f\u0001\u0000\u0000\u0000\u0211\u0214"+
		"\u0001\u0000\u0000\u0000\u0212\u0210\u0001\u0000\u0000\u0000\u0212\u0213"+
		"\u0001\u0000\u0000\u0000\u0213\u0216\u0001\u0000\u0000\u0000\u0214\u0212"+
		"\u0001\u0000\u0000\u0000\u0215\u0217\u0003P(\u0000\u0216\u0215\u0001\u0000"+
		"\u0000\u0000\u0216\u0217\u0001\u0000\u0000\u0000\u0217\u0225\u0001\u0000"+
		"\u0000\u0000\u0218\u0219\u0005\u000e\u0000\u0000\u0219\u021a\u0003n7\u0000"+
		"\u021a\u021e\u0003.\u0017\u0000\u021b\u021d\u0003N\'\u0000\u021c\u021b"+
		"\u0001\u0000\u0000\u0000\u021d\u0220\u0001\u0000\u0000\u0000\u021e\u021c"+
		"\u0001\u0000\u0000\u0000\u021e\u021f\u0001\u0000\u0000\u0000\u021f\u0222"+
		"\u0001\u0000\u0000\u0000\u0220\u021e\u0001\u0000\u0000\u0000\u0221\u0223"+
		"\u0003P(\u0000\u0222\u0221\u0001\u0000\u0000\u0000\u0222\u0223\u0001\u0000"+
		"\u0000\u0000\u0223\u0225\u0001\u0000\u0000\u0000\u0224\u020a\u0001\u0000"+
		"\u0000\u0000\u0224\u0218\u0001\u0000\u0000\u0000\u0225M\u0001\u0000\u0000"+
		"\u0000\u0226\u0227\u0005\u000f\u0000\u0000\u0227\u0228\u0005\u000e\u0000"+
		"\u0000\u0228\u0229\u0005c\u0000\u0000\u0229\u022a\u0003n7\u0000\u022a"+
		"\u022b\u0005d\u0000\u0000\u022b\u022c\u0003.\u0017\u0000\u022c\u0233\u0001"+
		"\u0000\u0000\u0000\u022d\u022e\u0005\u000f\u0000\u0000\u022e\u022f\u0005"+
		"\u000e\u0000\u0000\u022f\u0230\u0003n7\u0000\u0230\u0231\u0003.\u0017"+
		"\u0000\u0231\u0233\u0001\u0000\u0000\u0000\u0232\u0226\u0001\u0000\u0000"+
		"\u0000\u0232\u022d\u0001\u0000\u0000\u0000\u0233O\u0001\u0000\u0000\u0000"+
		"\u0234\u0235\u0005\u000f\u0000\u0000\u0235\u0236\u0003.\u0017\u0000\u0236"+
		"Q\u0001\u0000\u0000\u0000\u0237\u0238\u0005\u0010\u0000\u0000\u0238\u0239"+
		"\u0003p8\u0000\u0239\u023a\u0005R\u0000\u0000\u023a\u023b\u0005\u0011"+
		"\u0000\u0000\u023b\u023c\u0003n7\u0000\u023c\u023d\u0003.\u0017\u0000"+
		"\u023d\u024a\u0001\u0000\u0000\u0000\u023e\u023f\u0005\u0010\u0000\u0000"+
		"\u023f\u0240\u0003p8\u0000\u0240\u0241\u0005R\u0000\u0000\u0241\u0242"+
		"\u0005b\u0000\u0000\u0242\u0243\u0003n7\u0000\u0243\u0244\u0005]\u0000"+
		"\u0000\u0244\u0245\u0003n7\u0000\u0245\u0246\u0005]\u0000\u0000\u0246"+
		"\u0247\u0003n7\u0000\u0247\u0248\u0003.\u0017\u0000\u0248\u024a\u0001"+
		"\u0000\u0000\u0000\u0249\u0237\u0001\u0000\u0000\u0000\u0249\u023e\u0001"+
		"\u0000\u0000\u0000\u024aS\u0001\u0000\u0000\u0000\u024b\u024c\u0005\u0015"+
		"\u0000\u0000\u024c\u024d\u0005]\u0000\u0000\u024dU\u0001\u0000\u0000\u0000"+
		"\u024e\u024f\u0005\u0016\u0000\u0000\u024f\u0250\u0005]\u0000\u0000\u0250"+
		"W\u0001\u0000\u0000\u0000\u0251\u0252\u0005\u0012\u0000\u0000\u0252\u0253"+
		"\u0003.\u0017\u0000\u0253Y\u0001\u0000\u0000\u0000\u0254\u0255\u0005\u0013"+
		"\u0000\u0000\u0255\u0256\u0003n7\u0000\u0256\u0257\u0003.\u0017\u0000"+
		"\u0257\u025f\u0001\u0000\u0000\u0000\u0258\u0259\u0005\u0013\u0000\u0000"+
		"\u0259\u025a\u0005c\u0000\u0000\u025a\u025b\u0003n7\u0000\u025b\u025c"+
		"\u0005d\u0000\u0000\u025c\u025d\u0003.\u0017\u0000\u025d\u025f\u0001\u0000"+
		"\u0000\u0000\u025e\u0254\u0001\u0000\u0000\u0000\u025e\u0258\u0001\u0000"+
		"\u0000\u0000\u025f[\u0001\u0000\u0000\u0000\u0260\u0261\u0005\u0014\u0000"+
		"\u0000\u0261\u0262\u0003.\u0017\u0000\u0262\u0263\u0005\u0013\u0000\u0000"+
		"\u0263\u0264\u0003n7\u0000\u0264\u0265\u0005]\u0000\u0000\u0265\u026f"+
		"\u0001\u0000\u0000\u0000\u0266\u0267\u0005\u0014\u0000\u0000\u0267\u0268"+
		"\u0003.\u0017\u0000\u0268\u0269\u0005\u0013\u0000\u0000\u0269\u026a\u0005"+
		"c\u0000\u0000\u026a\u026b\u0003n7\u0000\u026b\u026c\u0005d\u0000\u0000"+
		"\u026c\u026d\u0005]\u0000\u0000\u026d\u026f\u0001\u0000\u0000\u0000\u026e"+
		"\u0260\u0001\u0000\u0000\u0000\u026e\u0266\u0001\u0000\u0000\u0000\u026f"+
		"]\u0001\u0000\u0000\u0000\u0270\u0271\u0005\u001c\u0000\u0000\u0271\u0272"+
		"\u0005c\u0000\u0000\u0272\u0273\u0003n7\u0000\u0273\u0274\u0005d\u0000"+
		"\u0000\u0274\u0275\u0003.\u0017\u0000\u0275_\u0001\u0000\u0000\u0000\u0276"+
		"\u0277\u0005\u001e\u0000\u0000\u0277\u0279\u0003.\u0017\u0000\u0278\u027a"+
		"\u0003b1\u0000\u0279\u0278\u0001\u0000\u0000\u0000\u027a\u027b\u0001\u0000"+
		"\u0000\u0000\u027b\u0279\u0001\u0000\u0000\u0000\u027b\u027c\u0001\u0000"+
		"\u0000\u0000\u027c\u027e\u0001\u0000\u0000\u0000\u027d\u027f\u0003d2\u0000"+
		"\u027e\u027d\u0001\u0000\u0000\u0000\u027e\u027f\u0001\u0000\u0000\u0000"+
		"\u027f\u0285\u0001\u0000\u0000\u0000\u0280\u0281\u0005\u001e\u0000\u0000"+
		"\u0281\u0282\u0003.\u0017\u0000\u0282\u0283\u0003d2\u0000\u0283\u0285"+
		"\u0001\u0000\u0000\u0000\u0284\u0276\u0001\u0000\u0000\u0000\u0284\u0280"+
		"\u0001\u0000\u0000\u0000\u0285a\u0001\u0000\u0000\u0000\u0286\u0287\u0005"+
		"\u001f\u0000\u0000\u0287\u0288\u0005c\u0000\u0000\u0288\u0289\u0003p8"+
		"\u0000\u0289\u028a\u0005R\u0000\u0000\u028a\u028b\u0005d\u0000\u0000\u028b"+
		"\u028c\u0003.\u0017\u0000\u028cc\u0001\u0000\u0000\u0000\u028d\u028e\u0005"+
		" \u0000\u0000\u028e\u028f\u0003.\u0017\u0000\u028fe\u0001\u0000\u0000"+
		"\u0000\u0290\u0291\u0005!\u0000\u0000\u0291\u0292\u0003n7\u0000\u0292"+
		"\u0293\u0005]\u0000\u0000\u0293g\u0001\u0000\u0000\u0000\u0294\u0295\u0005"+
		"\u0017\u0000\u0000\u0295\u0296\u0003n7\u0000\u0296\u029a\u0005e\u0000"+
		"\u0000\u0297\u0299\u0003j5\u0000\u0298\u0297\u0001\u0000\u0000\u0000\u0299"+
		"\u029c\u0001\u0000\u0000\u0000\u029a\u0298\u0001\u0000\u0000\u0000\u029a"+
		"\u029b\u0001\u0000\u0000\u0000\u029b\u029e\u0001\u0000\u0000\u0000\u029c"+
		"\u029a\u0001\u0000\u0000\u0000\u029d\u029f\u0003l6\u0000\u029e\u029d\u0001"+
		"\u0000\u0000\u0000\u029e\u029f\u0001\u0000\u0000\u0000\u029f\u02a0\u0001"+
		"\u0000\u0000\u0000\u02a0\u02a1\u0005f\u0000\u0000\u02a1\u02b3\u0001\u0000"+
		"\u0000\u0000\u02a2\u02a3\u0005\u0017\u0000\u0000\u02a3\u02a4\u0005c\u0000"+
		"\u0000\u02a4\u02a5\u0003n7\u0000\u02a5\u02a6\u0005d\u0000\u0000\u02a6"+
		"\u02aa\u0005e\u0000\u0000\u02a7\u02a9\u0003j5\u0000\u02a8\u02a7\u0001"+
		"\u0000\u0000\u0000\u02a9\u02ac\u0001\u0000\u0000\u0000\u02aa\u02a8\u0001"+
		"\u0000\u0000\u0000\u02aa\u02ab\u0001\u0000\u0000\u0000\u02ab\u02ae\u0001"+
		"\u0000\u0000\u0000\u02ac\u02aa\u0001\u0000\u0000\u0000\u02ad\u02af\u0003"+
		"l6\u0000\u02ae\u02ad\u0001\u0000\u0000\u0000\u02ae\u02af\u0001\u0000\u0000"+
		"\u0000\u02af\u02b0\u0001\u0000\u0000\u0000\u02b0\u02b1\u0005f\u0000\u0000"+
		"\u02b1\u02b3\u0001\u0000\u0000\u0000\u02b2\u0294\u0001\u0000\u0000\u0000"+
		"\u02b2\u02a2\u0001\u0000\u0000\u0000\u02b3i\u0001\u0000\u0000\u0000\u02b4"+
		"\u02b5\u0005\u0018\u0000\u0000\u02b5\u02ba\u0003n7\u0000\u02b6\u02b7\u0005"+
		"`\u0000\u0000\u02b7\u02b9\u0003n7\u0000\u02b8\u02b6\u0001\u0000\u0000"+
		"\u0000\u02b9\u02bc\u0001\u0000\u0000\u0000\u02ba\u02b8\u0001\u0000\u0000"+
		"\u0000\u02ba\u02bb\u0001\u0000\u0000\u0000\u02bb\u02bd\u0001\u0000\u0000"+
		"\u0000\u02bc\u02ba\u0001\u0000\u0000\u0000\u02bd\u02be\u0003.\u0017\u0000"+
		"\u02bek\u0001\u0000\u0000\u0000\u02bf\u02c0\u0005\u0019\u0000\u0000\u02c0"+
		"\u02c1\u0003.\u0017\u0000\u02c1m\u0001\u0000\u0000\u0000\u02c2\u02c3\u0006"+
		"7\uffff\uffff\u0000\u02c3\u02c4\u0005R\u0000\u0000\u02c4\u02d1\u0005e"+
		"\u0000\u0000\u02c5\u02c6\u0005R\u0000\u0000\u02c6\u02c7\u0005^\u0000\u0000"+
		"\u02c7\u02ce\u0003n7\u0000\u02c8\u02c9\u0005`\u0000\u0000\u02c9\u02ca"+
		"\u0005R\u0000\u0000\u02ca\u02cb\u0005^\u0000\u0000\u02cb\u02cd\u0003n"+
		"7\u0000\u02cc\u02c8\u0001\u0000\u0000\u0000\u02cd\u02d0\u0001\u0000\u0000"+
		"\u0000\u02ce\u02cc\u0001\u0000\u0000\u0000\u02ce\u02cf\u0001\u0000\u0000"+
		"\u0000\u02cf\u02d2\u0001\u0000\u0000\u0000\u02d0\u02ce\u0001\u0000\u0000"+
		"\u0000\u02d1\u02c5\u0001\u0000\u0000\u0000\u02d1\u02d2\u0001\u0000\u0000"+
		"\u0000\u02d2\u02d3\u0001\u0000\u0000\u0000\u02d3\u0336\u0005f\u0000\u0000"+
		"\u02d4\u02d5\u0005R\u0000\u0000\u02d5\u02d6\u0005_\u0000\u0000\u02d6\u02d7"+
		"\u0005R\u0000\u0000\u02d7\u02d9\u0005c\u0000\u0000\u02d8\u02da\u0003H"+
		"$\u0000\u02d9\u02d8\u0001\u0000\u0000\u0000\u02d9\u02da\u0001\u0000\u0000"+
		"\u0000\u02da\u02db\u0001\u0000\u0000\u0000\u02db\u0336\u0005d\u0000\u0000"+
		"\u02dc\u02dd\u0005R\u0000\u0000\u02dd\u02de\u0005_\u0000\u0000\u02de\u0336"+
		"\u0005R\u0000\u0000\u02df\u02e0\u0005i\u0000\u0000\u02e0\u0336\u0003n"+
		"7+\u02e1\u02e2\u0005j\u0000\u0000\u02e2\u0336\u0003n7*\u02e3\u02e4\u0005"+
		"k\u0000\u0000\u02e4\u0336\u0003n7)\u02e5\u02e6\u0005\u001a\u0000\u0000"+
		"\u02e6\u0336\u0003n7(\u02e7\u02e8\u0005\u001b\u0000\u0000\u02e8\u0336"+
		"\u0003n7\'\u02e9\u02ea\u0005w\u0000\u0000\u02ea\u0336\u0003n7&\u02eb\u02ec"+
		"\u0005}\u0000\u0000\u02ec\u0336\u0003n7%\u02ed\u02ee\u0005x\u0000\u0000"+
		"\u02ee\u0336\u0003n7$\u02ef\u02f0\u0005y\u0000\u0000\u02f0\u0336\u0003"+
		"n7#\u02f1\u02f2\u0005\f\u0000\u0000\u02f2\u02f3\u0005c\u0000\u0000\u02f3"+
		"\u02f4\u0003p8\u0000\u02f4\u02f5\u0005d\u0000\u0000\u02f5\u0336\u0001"+
		"\u0000\u0000\u0000\u02f6\u02f7\u0005\r\u0000\u0000\u02f7\u02f8\u0005c"+
		"\u0000\u0000\u02f8\u02f9\u0003n7\u0000\u02f9\u02fa\u0005d\u0000\u0000"+
		"\u02fa\u0336\u0001\u0000\u0000\u0000\u02fb\u02fc\u0005\u001e\u0000\u0000"+
		"\u02fc\u0336\u0003n7\u0011\u02fd\u02fe\u0005c\u0000\u0000\u02fe\u02ff"+
		"\u0003n7\u0000\u02ff\u0300\u0005`\u0000\u0000\u0300\u0305\u0003n7\u0000"+
		"\u0301\u0302\u0005`\u0000\u0000\u0302\u0304\u0003n7\u0000\u0303\u0301"+
		"\u0001\u0000\u0000\u0000\u0304\u0307\u0001\u0000\u0000\u0000\u0305\u0303"+
		"\u0001\u0000\u0000\u0000\u0305\u0306\u0001\u0000\u0000\u0000\u0306\u0308"+
		"\u0001\u0000\u0000\u0000\u0307\u0305\u0001\u0000\u0000\u0000\u0308\u0309"+
		"\u0005d\u0000\u0000\u0309\u0336\u0001\u0000\u0000\u0000\u030a\u030b\u0005"+
		"c\u0000\u0000\u030b\u030c\u0003n7\u0000\u030c\u030d\u0005d\u0000\u0000"+
		"\u030d\u0336\u0001\u0000\u0000\u0000\u030e\u030f\u0005-\u0000\u0000\u030f"+
		"\u0336\u0003n7\u000e\u0310\u0311\u0005g\u0000\u0000\u0311\u0312\u0003"+
		"n7\u0000\u0312\u0313\u0005{\u0000\u0000\u0313\u0314\u0005\u0010\u0000"+
		"\u0000\u0314\u0315\u0003p8\u0000\u0315\u0316\u0005R\u0000\u0000\u0316"+
		"\u0317\u0005\u0011\u0000\u0000\u0317\u031a\u0003n7\u0000\u0318\u0319\u0005"+
		"\u000e\u0000\u0000\u0319\u031b\u0003n7\u0000\u031a\u0318\u0001\u0000\u0000"+
		"\u0000\u031a\u031b\u0001\u0000\u0000\u0000\u031b\u031c\u0001\u0000\u0000"+
		"\u0000\u031c\u031d\u0005h\u0000\u0000\u031d\u0336\u0001\u0000\u0000\u0000"+
		"\u031e\u0327\u0005g\u0000\u0000\u031f\u0324\u0003n7\u0000\u0320\u0321"+
		"\u0005`\u0000\u0000\u0321\u0323\u0003n7\u0000\u0322\u0320\u0001\u0000"+
		"\u0000\u0000\u0323\u0326\u0001\u0000\u0000\u0000\u0324\u0322\u0001\u0000"+
		"\u0000\u0000\u0324\u0325\u0001\u0000\u0000\u0000\u0325\u0328\u0001\u0000"+
		"\u0000\u0000\u0326\u0324\u0001\u0000\u0000\u0000\u0327\u031f\u0001\u0000"+
		"\u0000\u0000\u0327\u0328\u0001\u0000\u0000\u0000\u0328\u0329\u0001\u0000"+
		"\u0000\u0000\u0329\u0336\u0005h\u0000\u0000\u032a\u0336\u0005\u0007\u0000"+
		"\u0000\u032b\u0336\u0005L\u0000\u0000\u032c\u0336\u0005I\u0000\u0000\u032d"+
		"\u0336\u0005J\u0000\u0000\u032e\u0336\u0005K\u0000\u0000\u032f\u0336\u0005"+
		"M\u0000\u0000\u0330\u0336\u0005N\u0000\u0000\u0331\u0336\u0005Q\u0000"+
		"\u0000\u0332\u0336\u0005P\u0000\u0000\u0333\u0336\u0005O\u0000\u0000\u0334"+
		"\u0336\u0005R\u0000\u0000\u0335\u02c2\u0001\u0000\u0000\u0000\u0335\u02d4"+
		"\u0001\u0000\u0000\u0000\u0335\u02dc\u0001\u0000\u0000\u0000\u0335\u02df"+
		"\u0001\u0000\u0000\u0000\u0335\u02e1\u0001\u0000\u0000\u0000\u0335\u02e3"+
		"\u0001\u0000\u0000\u0000\u0335\u02e5\u0001\u0000\u0000\u0000\u0335\u02e7"+
		"\u0001\u0000\u0000\u0000\u0335\u02e9\u0001\u0000\u0000\u0000\u0335\u02eb"+
		"\u0001\u0000\u0000\u0000\u0335\u02ed\u0001\u0000\u0000\u0000\u0335\u02ef"+
		"\u0001\u0000\u0000\u0000\u0335\u02f1\u0001\u0000\u0000\u0000\u0335\u02f6"+
		"\u0001\u0000\u0000\u0000\u0335\u02fb\u0001\u0000\u0000\u0000\u0335\u02fd"+
		"\u0001\u0000\u0000\u0000\u0335\u030a\u0001\u0000\u0000\u0000\u0335\u030e"+
		"\u0001\u0000\u0000\u0000\u0335\u0310\u0001\u0000\u0000\u0000\u0335\u031e"+
		"\u0001\u0000\u0000\u0000\u0335\u032a\u0001\u0000\u0000\u0000\u0335\u032b"+
		"\u0001\u0000\u0000\u0000\u0335\u032c\u0001\u0000\u0000\u0000\u0335\u032d"+
		"\u0001\u0000\u0000\u0000\u0335\u032e\u0001\u0000\u0000\u0000\u0335\u032f"+
		"\u0001\u0000\u0000\u0000\u0335\u0330\u0001\u0000\u0000\u0000\u0335\u0331"+
		"\u0001\u0000\u0000\u0000\u0335\u0332\u0001\u0000\u0000\u0000\u0335\u0333"+
		"\u0001\u0000\u0000\u0000\u0335\u0334\u0001\u0000\u0000\u0000\u0336\u03a1"+
		"\u0001\u0000\u0000\u0000\u0337\u0338\n \u0000\u0000\u0338\u0339\u0007"+
		"\u0002\u0000\u0000\u0339\u03a0\u0003n7!\u033a\u033b\n\u001f\u0000\u0000"+
		"\u033b\u033c\u0007\u0003\u0000\u0000\u033c\u03a0\u0003n7 \u033d\u033e"+
		"\n\u001e\u0000\u0000\u033e\u033f\u0005z\u0000\u0000\u033f\u03a0\u0003"+
		"n7\u001f\u0340\u0341\n\u001d\u0000\u0000\u0341\u0342\u0005t\u0000\u0000"+
		"\u0342\u0343\u0005t\u0000\u0000\u0343\u03a0\u0003n7\u001e\u0344\u0345"+
		"\n\u001c\u0000\u0000\u0345\u0346\u0007\u0004\u0000\u0000\u0346\u03a0\u0003"+
		"n7\u001d\u0347\u0348\n\u001b\u0000\u0000\u0348\u0349\u0007\u0005\u0000"+
		"\u0000\u0349\u03a0\u0003n7\u001c\u034a\u034b\n\u001a\u0000\u0000\u034b"+
		"\u034c\u0005j\u0000\u0000\u034c\u03a0\u0003n7\u001b\u034d\u034e\n\u0019"+
		"\u0000\u0000\u034e\u034f\u0005|\u0000\u0000\u034f\u03a0\u0003n7\u001a"+
		"\u0350\u0351\n\u0018\u0000\u0000\u0351\u0352\u0005{\u0000\u0000\u0352"+
		"\u03a0\u0003n7\u0019\u0353\u0354\n\u0017\u0000\u0000\u0354\u0355\u0005"+
		"u\u0000\u0000\u0355\u03a0\u0003n7\u0018\u0356\u0357\n\u0016\u0000\u0000"+
		"\u0357\u0358\u0005v\u0000\u0000\u0358\u03a0\u0003n7\u0017\u0359\u035a"+
		"\n\u0015\u0000\u0000\u035a\u035b\u0005,\u0000\u0000\u035b\u03a0\u0003"+
		"n7\u0016\u035c\u035d\n\u0014\u0000\u0000\u035d\u035e\u0005/\u0000\u0000"+
		"\u035e\u03a0\u0003n7\u0015\u035f\u0360\n\u0013\u0000\u0000\u0360\u0361"+
		"\u0005.\u0000\u0000\u0361\u03a0\u0003n7\u0014\u0362\u0363\n\u0012\u0000"+
		"\u0000\u0363\u0364\u0005~\u0000\u0000\u0364\u0365\u0003n7\u0000\u0365"+
		"\u0366\u0005^\u0000\u0000\u0366\u0367\u0003n7\u0012\u0367\u03a0\u0001"+
		"\u0000\u0000\u0000\u0368\u0369\n<\u0000\u0000\u0369\u036a\u0005a\u0000"+
		"\u0000\u036a\u036b\u0005R\u0000\u0000\u036b\u036d\u0005c\u0000\u0000\u036c"+
		"\u036e\u0003H$\u0000\u036d\u036c\u0001\u0000\u0000\u0000\u036d\u036e\u0001"+
		"\u0000\u0000\u0000\u036e\u036f\u0001\u0000\u0000\u0000\u036f\u03a0\u0005"+
		"d\u0000\u0000\u0370\u0371\n;\u0000\u0000\u0371\u0373\u0005c\u0000\u0000"+
		"\u0372\u0374\u0003H$\u0000\u0373\u0372\u0001\u0000\u0000\u0000\u0373\u0374"+
		"\u0001\u0000\u0000\u0000\u0374\u0375\u0001\u0000\u0000\u0000\u0375\u03a0"+
		"\u0005d\u0000\u0000\u0376\u0377\n:\u0000\u0000\u0377\u0378\u0005a\u0000"+
		"\u0000\u0378\u03a0\u0005R\u0000\u0000\u0379\u037a\n9\u0000\u0000\u037a"+
		"\u037b\u0005a\u0000\u0000\u037b\u03a0\u0005L\u0000\u0000\u037c\u037d\n"+
		"8\u0000\u0000\u037d\u037e\u0005a\u0000\u0000\u037e\u03a0\u0005M\u0000"+
		"\u0000\u037f\u0380\n7\u0000\u0000\u0380\u0381\u0005)\u0000\u0000\u0381"+
		"\u0382\u0005R\u0000\u0000\u0382\u0384\u0005c\u0000\u0000\u0383\u0385\u0003"+
		"H$\u0000\u0384\u0383\u0001\u0000\u0000\u0000\u0384\u0385\u0001\u0000\u0000"+
		"\u0000\u0385\u0386\u0001\u0000\u0000\u0000\u0386\u03a0\u0005d\u0000\u0000"+
		"\u0387\u0388\n6\u0000\u0000\u0388\u0389\u0005)\u0000\u0000\u0389\u03a0"+
		"\u0005R\u0000\u0000\u038a\u038b\n5\u0000\u0000\u038b\u038c\u0005)\u0000"+
		"\u0000\u038c\u03a0\u0005L\u0000\u0000\u038d\u038e\n4\u0000\u0000\u038e"+
		"\u038f\u0005)\u0000\u0000\u038f\u03a0\u0005M\u0000\u0000\u0390\u0391\n"+
		"3\u0000\u0000\u0391\u0392\u0005g\u0000\u0000\u0392\u0393\u0003n7\u0000"+
		"\u0393\u0394\u0005h\u0000\u0000\u0394\u03a0\u0001\u0000\u0000\u0000\u0395"+
		"\u0396\n2\u0000\u0000\u0396\u0397\u0005\n\u0000\u0000\u0397\u03a0\u0003"+
		"p8\u0000\u0398\u0399\n1\u0000\u0000\u0399\u039a\u0005\u000b\u0000\u0000"+
		"\u039a\u03a0\u0003p8\u0000\u039b\u039c\n0\u0000\u0000\u039c\u03a0\u0005"+
		"x\u0000\u0000\u039d\u039e\n/\u0000\u0000\u039e\u03a0\u0005y\u0000\u0000"+
		"\u039f\u0337\u0001\u0000\u0000\u0000\u039f\u033a\u0001\u0000\u0000\u0000"+
		"\u039f\u033d\u0001\u0000\u0000\u0000\u039f\u0340\u0001\u0000\u0000\u0000"+
		"\u039f\u0344\u0001\u0000\u0000\u0000\u039f\u0347\u0001\u0000\u0000\u0000"+
		"\u039f\u034a\u0001\u0000\u0000\u0000\u039f\u034d\u0001\u0000\u0000\u0000"+
		"\u039f\u0350\u0001\u0000\u0000\u0000\u039f\u0353\u0001\u0000\u0000\u0000"+
		"\u039f\u0356\u0001\u0000\u0000\u0000\u039f\u0359\u0001\u0000\u0000\u0000"+
		"\u039f\u035c\u0001\u0000\u0000\u0000\u039f\u035f\u0001\u0000\u0000\u0000"+
		"\u039f\u0362\u0001\u0000\u0000\u0000\u039f\u0368\u0001\u0000\u0000\u0000"+
		"\u039f\u0370\u0001\u0000\u0000\u0000\u039f\u0376\u0001\u0000\u0000\u0000"+
		"\u039f\u0379\u0001\u0000\u0000\u0000\u039f\u037c\u0001\u0000\u0000\u0000"+
		"\u039f\u037f\u0001\u0000\u0000\u0000\u039f\u0387\u0001\u0000\u0000\u0000"+
		"\u039f\u038a\u0001\u0000\u0000\u0000\u039f\u038d\u0001\u0000\u0000\u0000"+
		"\u039f\u0390\u0001\u0000\u0000\u0000\u039f\u0395\u0001\u0000\u0000\u0000"+
		"\u039f\u0398\u0001\u0000\u0000\u0000\u039f\u039b\u0001\u0000\u0000\u0000"+
		"\u039f\u039d\u0001\u0000\u0000\u0000\u03a0\u03a3\u0001\u0000\u0000\u0000"+
		"\u03a1\u039f\u0001\u0000\u0000\u0000\u03a1\u03a2\u0001\u0000\u0000\u0000"+
		"\u03a2o\u0001\u0000\u0000\u0000\u03a3\u03a1\u0001\u0000\u0000\u0000\u03a4"+
		"\u03a5\u00068\uffff\uffff\u0000\u03a5\u03a6\u0005i\u0000\u0000\u03a6\u03db"+
		"\u0003p8\r\u03a7\u03a8\u0005g\u0000\u0000\u03a8\u03a9\u0005L\u0000\u0000"+
		"\u03a9\u03aa\u0005h\u0000\u0000\u03aa\u03db\u0003p8\u000b\u03ab\u03ac"+
		"\u0005g\u0000\u0000\u03ac\u03ad\u0005h\u0000\u0000\u03ad\u03db\u0003p"+
		"8\n\u03ae\u03db\u0003r9\u0000\u03af\u03b0\u0005%\u0000\u0000\u03b0\u03b1"+
		"\u0005s\u0000\u0000\u03b1\u03b2\u0003p8\u0000\u03b2\u03b3\u0005t\u0000"+
		"\u0000\u03b3\u03db\u0001\u0000\u0000\u0000\u03b4\u03b5\u0005&\u0000\u0000"+
		"\u03b5\u03b6\u0005s\u0000\u0000\u03b6\u03b7\u0003p8\u0000\u03b7\u03b8"+
		"\u0005`\u0000\u0000\u03b8\u03b9\u0003p8\u0000\u03b9\u03ba\u0005t\u0000"+
		"\u0000\u03ba\u03db\u0001\u0000\u0000\u0000\u03bb\u03bc\u0005\'\u0000\u0000"+
		"\u03bc\u03bd\u0005s\u0000\u0000\u03bd\u03be\u0003p8\u0000\u03be\u03bf"+
		"\u0005t\u0000\u0000\u03bf\u03db\u0001\u0000\u0000\u0000\u03c0\u03c1\u0005"+
		"(\u0000\u0000\u03c1\u03c2\u0005s\u0000\u0000\u03c2\u03c5\u0003p8\u0000"+
		"\u03c3\u03c4\u0005`\u0000\u0000\u03c4\u03c6\u0003p8\u0000\u03c5\u03c3"+
		"\u0001\u0000\u0000\u0000\u03c6\u03c7\u0001\u0000\u0000\u0000\u03c7\u03c5"+
		"\u0001\u0000\u0000\u0000\u03c7\u03c8\u0001\u0000\u0000\u0000\u03c8\u03c9"+
		"\u0001\u0000\u0000\u0000\u03c9\u03ca\u0005t\u0000\u0000\u03ca\u03db\u0001"+
		"\u0000\u0000\u0000\u03cb\u03cc\u0005R\u0000\u0000\u03cc\u03cd\u0005s\u0000"+
		"\u0000\u03cd\u03d2\u0003p8\u0000\u03ce\u03cf\u0005`\u0000\u0000\u03cf"+
		"\u03d1\u0003p8\u0000\u03d0\u03ce\u0001\u0000\u0000\u0000\u03d1\u03d4\u0001"+
		"\u0000\u0000\u0000\u03d2\u03d0\u0001\u0000\u0000\u0000\u03d2\u03d3\u0001"+
		"\u0000\u0000\u0000\u03d3\u03d5\u0001\u0000\u0000\u0000\u03d4\u03d2\u0001"+
		"\u0000\u0000\u0000\u03d5\u03d6\u0005t\u0000\u0000\u03d6\u03db\u0001\u0000"+
		"\u0000\u0000\u03d7\u03db\u0003t:\u0000\u03d8\u03db\u0005$\u0000\u0000"+
		"\u03d9\u03db\u0005R\u0000\u0000\u03da\u03a4\u0001\u0000\u0000\u0000\u03da"+
		"\u03a7\u0001\u0000\u0000\u0000\u03da\u03ab\u0001\u0000\u0000\u0000\u03da"+
		"\u03ae\u0001\u0000\u0000\u0000\u03da\u03af\u0001\u0000\u0000\u0000\u03da"+
		"\u03b4\u0001\u0000\u0000\u0000\u03da\u03bb\u0001\u0000\u0000\u0000\u03da"+
		"\u03c0\u0001\u0000\u0000\u0000\u03da\u03cb\u0001\u0000\u0000\u0000\u03da"+
		"\u03d7\u0001\u0000\u0000\u0000\u03da\u03d8\u0001\u0000\u0000\u0000\u03da"+
		"\u03d9\u0001\u0000\u0000\u0000\u03db\u03e0\u0001\u0000\u0000\u0000\u03dc"+
		"\u03dd\n\f\u0000\u0000\u03dd\u03df\u0005i\u0000\u0000\u03de\u03dc\u0001"+
		"\u0000\u0000\u0000\u03df\u03e2\u0001\u0000\u0000\u0000\u03e0\u03de\u0001"+
		"\u0000\u0000\u0000\u03e0\u03e1\u0001\u0000\u0000\u0000\u03e1q\u0001\u0000"+
		"\u0000\u0000\u03e2\u03e0\u0001\u0000\u0000\u0000\u03e3\u03e4\u0005\b\u0000"+
		"\u0000\u03e4\u03ed\u0005c\u0000\u0000\u03e5\u03ea\u0003p8\u0000\u03e6"+
		"\u03e7\u0005`\u0000\u0000\u03e7\u03e9\u0003p8\u0000\u03e8\u03e6\u0001"+
		"\u0000\u0000\u0000\u03e9\u03ec\u0001\u0000\u0000\u0000\u03ea\u03e8\u0001"+
		"\u0000\u0000\u0000\u03ea\u03eb\u0001\u0000\u0000\u0000\u03eb\u03ee\u0001"+
		"\u0000\u0000\u0000\u03ec\u03ea\u0001\u0000\u0000\u0000\u03ed\u03e5\u0001"+
		"\u0000\u0000\u0000\u03ed\u03ee\u0001\u0000\u0000\u0000\u03ee\u03ef\u0001"+
		"\u0000\u0000\u0000\u03ef\u03f0\u0005d\u0000\u0000\u03f0\u03f1\u0005)\u0000"+
		"\u0000\u03f1\u03f2\u0003p8\u0000\u03f2s\u0001\u0000\u0000\u0000\u03f3"+
		"\u03f4\u0007\u0006\u0000\u0000\u03f4u\u0001\u0000\u0000\u0000Nw|\u0082"+
		"\u0089\u009e\u00a4\u00ab\u00b7\u00c7\u00cb\u00d2\u00d8\u00e7\u00f5\u00f9"+
		"\u0103\u0108\u010d\u0111\u0119\u011f\u012a\u0132\u013d\u0147\u014c\u0153"+
		"\u015d\u0163\u017f\u0185\u0191\u01a3\u01ac\u01bd\u01ca\u01d0\u01e4\u01f7"+
		"\u0201\u0206\u0212\u0216\u021e\u0222\u0224\u0232\u0249\u025e\u026e\u027b"+
		"\u027e\u0284\u029a\u029e\u02aa\u02ae\u02b2\u02ba\u02ce\u02d1\u02d9\u0305"+
		"\u031a\u0324\u0327\u0335\u036d\u0373\u0384\u039f\u03a1\u03c7\u03d2\u03da"+
		"\u03e0\u03ea\u03ed";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}