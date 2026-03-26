
// Generated from /home/carlos/projects/cpp/tollvm/grammar/ToLLVMParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "ToLLVMParserVisitor.h"


/**
 * This class provides an empty implementation of ToLLVMParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  ToLLVMParserBaseVisitor : public ToLLVMParserVisitor {
public:

  virtual std::any visitProgram(ToLLVMParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNamespaceDecl(ToLLVMParser::NamespaceDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUseItem(ToLLVMParser::UseItemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUseGroup(ToLLVMParser::UseGroupContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModulePath(ToLLVMParser::ModulePathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIncludeDecl(ToLLVMParser::IncludeDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTopLevelDecl(ToLLVMParser::TopLevelDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeAliasDecl(ToLLVMParser::TypeAliasDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumDecl(ToLLVMParser::EnumDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructDecl(ToLLVMParser::StructDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructField(ToLLVMParser::StructFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnionDecl(ToLLVMParser::UnionDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnionField(ToLLVMParser::UnionFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExternDecl(ToLLVMParser::ExternDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExternParamList(ToLLVMParser::ExternParamListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExternParam(ToLLVMParser::ExternParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunctionDecl(ToLLVMParser::FunctionDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExtendDecl(ToLLVMParser::ExtendDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExtendMethod(ToLLVMParser::ExtendMethodContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParamList(ToLLVMParser::ParamListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParam(ToLLVMParser::ParamContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBlock(ToLLVMParser::BlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(ToLLVMParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeferStmt(ToLLVMParser::DeferStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExprStmt(ToLLVMParser::ExprStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVarDeclStmt(ToLLVMParser::VarDeclStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAssignStmt(ToLLVMParser::AssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompoundAssignStmt(ToLLVMParser::CompoundAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFieldAssignStmt(ToLLVMParser::FieldAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexFieldAssignStmt(ToLLVMParser::IndexFieldAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDerefAssignStmt(ToLLVMParser::DerefAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrowAssignStmt(ToLLVMParser::ArrowAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrowCompoundAssignStmt(ToLLVMParser::ArrowCompoundAssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCallStmt(ToLLVMParser::CallStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArgList(ToLLVMParser::ArgListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturnStmt(ToLLVMParser::ReturnStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIfStmt(ToLLVMParser::IfStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitElseIfClause(ToLLVMParser::ElseIfClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitElseClause(ToLLVMParser::ElseClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForInStmt(ToLLVMParser::ForInStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForClassicStmt(ToLLVMParser::ForClassicStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBreakStmt(ToLLVMParser::BreakStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitContinueStmt(ToLLVMParser::ContinueStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoopStmt(ToLLVMParser::LoopStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhileStmt(ToLLVMParser::WhileStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDoWhileStmt(ToLLVMParser::DoWhileStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLockStmt(ToLLVMParser::LockStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTryCatchStmt(ToLLVMParser::TryCatchStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCatchClause(ToLLVMParser::CatchClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFinallyClause(ToLLVMParser::FinallyClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitThrowStmt(ToLLVMParser::ThrowStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSwitchStmt(ToLLVMParser::SwitchStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCaseClause(ToLLVMParser::CaseClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefaultClause(ToLLVMParser::DefaultClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFieldAccessExpr(ToLLVMParser::FieldAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeofExpr(ToLLVMParser::TypeofExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitXorExpr(ToLLVMParser::BitXorExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalNotExpr(ToLLVMParser::LogicalNotExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentExpr(ToLLVMParser::IdentExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPreIncrExpr(ToLLVMParser::PreIncrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTernaryExpr(ToLLVMParser::TernaryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNullLitExpr(ToLLVMParser::NullLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMulExpr(ToLLVMParser::MulExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitAndExpr(ToLLVMParser::BitAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIsExpr(ToLLVMParser::IsExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddSubExpr(ToLLVMParser::AddSubExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIntLitExpr(ToLLVMParser::IntLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAddrOfExpr(ToLLVMParser::AddrOfExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFloatLitExpr(ToLLVMParser::FloatLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSpawnExpr(ToLLVMParser::SpawnExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrowAccessExpr(ToLLVMParser::ArrowAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitListCompExpr(ToLLVMParser::ListCompExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShiftExpr(ToLLVMParser::ShiftExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexExpr(ToLLVMParser::IndexExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNegExpr(ToLLVMParser::NegExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDerefExpr(ToLLVMParser::DerefExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPreDecrExpr(ToLLVMParser::PreDecrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSpreadExpr(ToLLVMParser::SpreadExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStaticMethodCallExpr(ToLLVMParser::StaticMethodCallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNullCoalExpr(ToLLVMParser::NullCoalExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCastExpr(ToLLVMParser::CastExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEnumAccessExpr(ToLLVMParser::EnumAccessExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitParenExpr(ToLLVMParser::ParenExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitNotExpr(ToLLVMParser::BitNotExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitArrayLitExpr(ToLLVMParser::ArrayLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitMethodCallExpr(ToLLVMParser::MethodCallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStructLitExpr(ToLLVMParser::StructLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPostDecrExpr(ToLLVMParser::PostDecrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelExpr(ToLLVMParser::RelExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRangeInclExpr(ToLLVMParser::RangeInclExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalAndExpr(ToLLVMParser::LogicalAndExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStrLitExpr(ToLLVMParser::StrLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAwaitExpr(ToLLVMParser::AwaitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCStrLitExpr(ToLLVMParser::CStrLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFnCallExpr(ToLLVMParser::FnCallExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLogicalOrExpr(ToLLVMParser::LogicalOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSizeofExpr(ToLLVMParser::SizeofExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitEqExpr(ToLLVMParser::EqExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBitOrExpr(ToLLVMParser::BitOrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCharLitExpr(ToLLVMParser::CharLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTryExpr(ToLLVMParser::TryExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPostIncrExpr(ToLLVMParser::PostIncrExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBoolLitExpr(ToLLVMParser::BoolLitExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRangeExpr(ToLLVMParser::RangeExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTypeSpec(ToLLVMParser::TypeSpecContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFnTypeSpec(ToLLVMParser::FnTypeSpecContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimitiveType(ToLLVMParser::PrimitiveTypeContext *ctx) override {
    return visitChildren(ctx);
  }


};

