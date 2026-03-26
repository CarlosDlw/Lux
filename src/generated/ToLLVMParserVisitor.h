
// Generated from /home/carlos/projects/cpp/tollvm/grammar/ToLLVMParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ToLLVMParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by ToLLVMParser.
 */
class  ToLLVMParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by ToLLVMParser.
   */
    virtual std::any visitProgram(ToLLVMParser::ProgramContext *context) = 0;

    virtual std::any visitNamespaceDecl(ToLLVMParser::NamespaceDeclContext *context) = 0;

    virtual std::any visitUseItem(ToLLVMParser::UseItemContext *context) = 0;

    virtual std::any visitUseGroup(ToLLVMParser::UseGroupContext *context) = 0;

    virtual std::any visitModulePath(ToLLVMParser::ModulePathContext *context) = 0;

    virtual std::any visitIncludeDecl(ToLLVMParser::IncludeDeclContext *context) = 0;

    virtual std::any visitTopLevelDecl(ToLLVMParser::TopLevelDeclContext *context) = 0;

    virtual std::any visitTypeAliasDecl(ToLLVMParser::TypeAliasDeclContext *context) = 0;

    virtual std::any visitEnumDecl(ToLLVMParser::EnumDeclContext *context) = 0;

    virtual std::any visitStructDecl(ToLLVMParser::StructDeclContext *context) = 0;

    virtual std::any visitStructField(ToLLVMParser::StructFieldContext *context) = 0;

    virtual std::any visitUnionDecl(ToLLVMParser::UnionDeclContext *context) = 0;

    virtual std::any visitUnionField(ToLLVMParser::UnionFieldContext *context) = 0;

    virtual std::any visitExternDecl(ToLLVMParser::ExternDeclContext *context) = 0;

    virtual std::any visitExternParamList(ToLLVMParser::ExternParamListContext *context) = 0;

    virtual std::any visitExternParam(ToLLVMParser::ExternParamContext *context) = 0;

    virtual std::any visitFunctionDecl(ToLLVMParser::FunctionDeclContext *context) = 0;

    virtual std::any visitExtendDecl(ToLLVMParser::ExtendDeclContext *context) = 0;

    virtual std::any visitExtendMethod(ToLLVMParser::ExtendMethodContext *context) = 0;

    virtual std::any visitParamList(ToLLVMParser::ParamListContext *context) = 0;

    virtual std::any visitParam(ToLLVMParser::ParamContext *context) = 0;

    virtual std::any visitBlock(ToLLVMParser::BlockContext *context) = 0;

    virtual std::any visitStatement(ToLLVMParser::StatementContext *context) = 0;

    virtual std::any visitDeferStmt(ToLLVMParser::DeferStmtContext *context) = 0;

    virtual std::any visitExprStmt(ToLLVMParser::ExprStmtContext *context) = 0;

    virtual std::any visitVarDeclStmt(ToLLVMParser::VarDeclStmtContext *context) = 0;

    virtual std::any visitAssignStmt(ToLLVMParser::AssignStmtContext *context) = 0;

    virtual std::any visitCompoundAssignStmt(ToLLVMParser::CompoundAssignStmtContext *context) = 0;

    virtual std::any visitFieldAssignStmt(ToLLVMParser::FieldAssignStmtContext *context) = 0;

    virtual std::any visitIndexFieldAssignStmt(ToLLVMParser::IndexFieldAssignStmtContext *context) = 0;

    virtual std::any visitDerefAssignStmt(ToLLVMParser::DerefAssignStmtContext *context) = 0;

    virtual std::any visitArrowAssignStmt(ToLLVMParser::ArrowAssignStmtContext *context) = 0;

    virtual std::any visitArrowCompoundAssignStmt(ToLLVMParser::ArrowCompoundAssignStmtContext *context) = 0;

    virtual std::any visitCallStmt(ToLLVMParser::CallStmtContext *context) = 0;

    virtual std::any visitArgList(ToLLVMParser::ArgListContext *context) = 0;

    virtual std::any visitReturnStmt(ToLLVMParser::ReturnStmtContext *context) = 0;

    virtual std::any visitIfStmt(ToLLVMParser::IfStmtContext *context) = 0;

    virtual std::any visitElseIfClause(ToLLVMParser::ElseIfClauseContext *context) = 0;

    virtual std::any visitElseClause(ToLLVMParser::ElseClauseContext *context) = 0;

    virtual std::any visitForInStmt(ToLLVMParser::ForInStmtContext *context) = 0;

    virtual std::any visitForClassicStmt(ToLLVMParser::ForClassicStmtContext *context) = 0;

    virtual std::any visitBreakStmt(ToLLVMParser::BreakStmtContext *context) = 0;

    virtual std::any visitContinueStmt(ToLLVMParser::ContinueStmtContext *context) = 0;

    virtual std::any visitLoopStmt(ToLLVMParser::LoopStmtContext *context) = 0;

    virtual std::any visitWhileStmt(ToLLVMParser::WhileStmtContext *context) = 0;

    virtual std::any visitDoWhileStmt(ToLLVMParser::DoWhileStmtContext *context) = 0;

    virtual std::any visitLockStmt(ToLLVMParser::LockStmtContext *context) = 0;

    virtual std::any visitTryCatchStmt(ToLLVMParser::TryCatchStmtContext *context) = 0;

    virtual std::any visitCatchClause(ToLLVMParser::CatchClauseContext *context) = 0;

    virtual std::any visitFinallyClause(ToLLVMParser::FinallyClauseContext *context) = 0;

    virtual std::any visitThrowStmt(ToLLVMParser::ThrowStmtContext *context) = 0;

    virtual std::any visitSwitchStmt(ToLLVMParser::SwitchStmtContext *context) = 0;

    virtual std::any visitCaseClause(ToLLVMParser::CaseClauseContext *context) = 0;

    virtual std::any visitDefaultClause(ToLLVMParser::DefaultClauseContext *context) = 0;

    virtual std::any visitFieldAccessExpr(ToLLVMParser::FieldAccessExprContext *context) = 0;

    virtual std::any visitTypeofExpr(ToLLVMParser::TypeofExprContext *context) = 0;

    virtual std::any visitBitXorExpr(ToLLVMParser::BitXorExprContext *context) = 0;

    virtual std::any visitLogicalNotExpr(ToLLVMParser::LogicalNotExprContext *context) = 0;

    virtual std::any visitIdentExpr(ToLLVMParser::IdentExprContext *context) = 0;

    virtual std::any visitPreIncrExpr(ToLLVMParser::PreIncrExprContext *context) = 0;

    virtual std::any visitTernaryExpr(ToLLVMParser::TernaryExprContext *context) = 0;

    virtual std::any visitNullLitExpr(ToLLVMParser::NullLitExprContext *context) = 0;

    virtual std::any visitMulExpr(ToLLVMParser::MulExprContext *context) = 0;

    virtual std::any visitBitAndExpr(ToLLVMParser::BitAndExprContext *context) = 0;

    virtual std::any visitIsExpr(ToLLVMParser::IsExprContext *context) = 0;

    virtual std::any visitAddSubExpr(ToLLVMParser::AddSubExprContext *context) = 0;

    virtual std::any visitIntLitExpr(ToLLVMParser::IntLitExprContext *context) = 0;

    virtual std::any visitAddrOfExpr(ToLLVMParser::AddrOfExprContext *context) = 0;

    virtual std::any visitFloatLitExpr(ToLLVMParser::FloatLitExprContext *context) = 0;

    virtual std::any visitSpawnExpr(ToLLVMParser::SpawnExprContext *context) = 0;

    virtual std::any visitArrowAccessExpr(ToLLVMParser::ArrowAccessExprContext *context) = 0;

    virtual std::any visitListCompExpr(ToLLVMParser::ListCompExprContext *context) = 0;

    virtual std::any visitShiftExpr(ToLLVMParser::ShiftExprContext *context) = 0;

    virtual std::any visitIndexExpr(ToLLVMParser::IndexExprContext *context) = 0;

    virtual std::any visitNegExpr(ToLLVMParser::NegExprContext *context) = 0;

    virtual std::any visitDerefExpr(ToLLVMParser::DerefExprContext *context) = 0;

    virtual std::any visitPreDecrExpr(ToLLVMParser::PreDecrExprContext *context) = 0;

    virtual std::any visitSpreadExpr(ToLLVMParser::SpreadExprContext *context) = 0;

    virtual std::any visitStaticMethodCallExpr(ToLLVMParser::StaticMethodCallExprContext *context) = 0;

    virtual std::any visitNullCoalExpr(ToLLVMParser::NullCoalExprContext *context) = 0;

    virtual std::any visitCastExpr(ToLLVMParser::CastExprContext *context) = 0;

    virtual std::any visitEnumAccessExpr(ToLLVMParser::EnumAccessExprContext *context) = 0;

    virtual std::any visitParenExpr(ToLLVMParser::ParenExprContext *context) = 0;

    virtual std::any visitBitNotExpr(ToLLVMParser::BitNotExprContext *context) = 0;

    virtual std::any visitArrayLitExpr(ToLLVMParser::ArrayLitExprContext *context) = 0;

    virtual std::any visitMethodCallExpr(ToLLVMParser::MethodCallExprContext *context) = 0;

    virtual std::any visitStructLitExpr(ToLLVMParser::StructLitExprContext *context) = 0;

    virtual std::any visitPostDecrExpr(ToLLVMParser::PostDecrExprContext *context) = 0;

    virtual std::any visitRelExpr(ToLLVMParser::RelExprContext *context) = 0;

    virtual std::any visitRangeInclExpr(ToLLVMParser::RangeInclExprContext *context) = 0;

    virtual std::any visitLogicalAndExpr(ToLLVMParser::LogicalAndExprContext *context) = 0;

    virtual std::any visitStrLitExpr(ToLLVMParser::StrLitExprContext *context) = 0;

    virtual std::any visitAwaitExpr(ToLLVMParser::AwaitExprContext *context) = 0;

    virtual std::any visitCStrLitExpr(ToLLVMParser::CStrLitExprContext *context) = 0;

    virtual std::any visitFnCallExpr(ToLLVMParser::FnCallExprContext *context) = 0;

    virtual std::any visitLogicalOrExpr(ToLLVMParser::LogicalOrExprContext *context) = 0;

    virtual std::any visitSizeofExpr(ToLLVMParser::SizeofExprContext *context) = 0;

    virtual std::any visitEqExpr(ToLLVMParser::EqExprContext *context) = 0;

    virtual std::any visitBitOrExpr(ToLLVMParser::BitOrExprContext *context) = 0;

    virtual std::any visitCharLitExpr(ToLLVMParser::CharLitExprContext *context) = 0;

    virtual std::any visitTryExpr(ToLLVMParser::TryExprContext *context) = 0;

    virtual std::any visitPostIncrExpr(ToLLVMParser::PostIncrExprContext *context) = 0;

    virtual std::any visitBoolLitExpr(ToLLVMParser::BoolLitExprContext *context) = 0;

    virtual std::any visitRangeExpr(ToLLVMParser::RangeExprContext *context) = 0;

    virtual std::any visitTypeSpec(ToLLVMParser::TypeSpecContext *context) = 0;

    virtual std::any visitFnTypeSpec(ToLLVMParser::FnTypeSpecContext *context) = 0;

    virtual std::any visitPrimitiveType(ToLLVMParser::PrimitiveTypeContext *context) = 0;


};

