#pragma once

#include "stella/ast/ast_fwd.hpp"

namespace stella {
namespace ast {

class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;

    virtual void Visit(const NodeBase& node);

    virtual void VisitProgram(const NodeProgram& node) = 0;

    virtual void VisitParamDecl(const NodeParamDecl& node) = 0;
    virtual void VisitDeclFun(const NodeDeclFun& node) = 0;

    virtual void VisitExprConstInt(const NodeExprConstInt& node) = 0;
    virtual void VisitExprSucc(const NodeExprSucc& node) = 0;
    virtual void VisitExprPred(const NodeExprPred& node) = 0;
    virtual void VisitExprIsZero(const NodeExprIsZero& node) = 0;
    virtual void VisitExprNatRec(const NodeExprNatRec& node) = 0;

    virtual void VisitExprConstTrue(const NodeExprConstTrue& node) = 0;
    virtual void VisitExprConstFalse(const NodeExprConstFalse& node) = 0;
    virtual void VisitExprIf(const NodeExprIf& node) = 0;

    virtual void VisitExprVar(const NodeExprVar& node) = 0;

    virtual void VisitExprAbstraction(const NodeExprAbstraction& node) = 0;
    virtual void VisitExprApplication(const NodeExprApplication& node) = 0;
    virtual void VisitExprFix(const NodeExprFix& node) = 0;

    virtual void VisitExprConstUnit(const NodeExprConstUnit& node) = 0;

    virtual void VisitExprTypeAsc(const NodeExprTypeAsc& node) = 0;

    virtual void VisitPatternVar(const NodePatternVar& node) = 0;
    virtual void VisitExprLet(const NodeExprLet& node) = 0;

    virtual void VisitExprList(const NodeExprList& node) = 0;
    virtual void VisitExprConsList(const NodeExprConsList& node) = 0;
    virtual void VisitExprHead(const NodeExprHead& node) = 0;
    virtual void VisitExprTail(const NodeExprTail& node) = 0;
    virtual void VisitExprIsEmpty(const NodeExprIsEmpty& node) = 0;

    virtual void VisitExprTuple(const NodeExprTuple& node) = 0;
    virtual void VisitExprDotTuple(const NodeExprDotTuple& node) = 0;

    virtual void VisitExprRecord(const NodeExprRecord& node) = 0;
    virtual void VisitExprDotRecord(const NodeExprDotRecord& node) = 0;

    virtual void VisitExprInl(const NodeExprInl& node) = 0;
    virtual void VisitExprInr(const NodeExprInr& node) = 0;
    virtual void VisitPatternInl(const NodePatternInl& node) = 0;
    virtual void VisitPatternInr(const NodePatternInr& node) = 0;
    virtual void VisitMatchCase(const NodeMatchCase& node) = 0;
    virtual void VisitExprMatch(const NodeExprMatch& node) = 0;

    virtual void VisitExprVariant(const NodeExprVariant& node) = 0;
    virtual void VisitPatternVariant(const NodePatternVariant& node) = 0;
};

class TypeVisitor {
public:
    virtual ~TypeVisitor() = default;

    virtual void Visit(const Type& type);

    virtual void VisitTypeFun(const TypeFun& type) = 0;
    virtual void VisitTypeBool(const TypeBool& type) = 0;
    virtual void VisitTypeNat(const TypeNat& type) = 0;
    virtual void VisitTypeUnit(const TypeUnit& type) = 0;
    virtual void VisitTypeList(const TypeList& type) = 0;
    virtual void VisitTypeTuple(const TypeTuple& type) = 0;
    virtual void VisitTypeRecord(const TypeRecord& type) = 0;
    virtual void VisitTypeSum(const TypeSum& type) = 0;
    virtual void VisitTypeVariant(const TypeVariant& type) = 0;
};

class BaseNodeVisitor : public NodeVisitor {
public:
    virtual ~BaseNodeVisitor() = default;

    virtual void VisitDefaultNode(const NodeBase& node);

    void VisitProgram(const NodeProgram& node) override;
    void VisitParamDecl(const NodeParamDecl& node) override;
    void VisitDeclFun(const NodeDeclFun& node) override;
    void VisitExprConstInt(const NodeExprConstInt& node) override;
    void VisitExprSucc(const NodeExprSucc& node) override;
    void VisitExprPred(const NodeExprPred& node) override;
    void VisitExprIsZero(const NodeExprIsZero& node) override;
    void VisitExprNatRec(const NodeExprNatRec& node) override;
    void VisitExprConstTrue(const NodeExprConstTrue& node) override;
    void VisitExprConstFalse(const NodeExprConstFalse& node) override;
    void VisitExprIf(const NodeExprIf& node) override;
    void VisitExprVar(const NodeExprVar& node) override;
    void VisitExprAbstraction(const NodeExprAbstraction& node) override;
    void VisitExprApplication(const NodeExprApplication& node) override;
    void VisitExprFix(const NodeExprFix& node) override;
    void VisitExprConstUnit(const NodeExprConstUnit& node) override;
    void VisitExprTypeAsc(const NodeExprTypeAsc& node) override;
    void VisitPatternVar(const NodePatternVar& node) override;
    void VisitExprLet(const NodeExprLet& node) override;
    void VisitExprList(const NodeExprList& node) override;
    void VisitExprConsList(const NodeExprConsList& node) override;
    void VisitExprHead(const NodeExprHead& node) override;
    void VisitExprTail(const NodeExprTail& node) override;
    void VisitExprIsEmpty(const NodeExprIsEmpty& node) override;
    void VisitExprTuple(const NodeExprTuple& node) override;
    void VisitExprDotTuple(const NodeExprDotTuple& node) override;
    void VisitExprRecord(const NodeExprRecord& node) override;
    void VisitExprDotRecord(const NodeExprDotRecord& node) override;

    void VisitExprInl(const NodeExprInl& node) override;
    void VisitExprInr(const NodeExprInr& node) override;
    void VisitPatternInl(const NodePatternInl& node) override;
    void VisitPatternInr(const NodePatternInr& node) override;
    void VisitMatchCase(const NodeMatchCase& node) override;
    void VisitExprMatch(const NodeExprMatch& node) override;

    void VisitExprVariant(const NodeExprVariant& node) override;
    void VisitPatternVariant(const NodePatternVariant& node) override;
};

class BaseTypeVisitor : public TypeVisitor {
public:
    virtual ~BaseTypeVisitor() = default;

    virtual void VisitDefaultType(const Type& type);

    void VisitTypeFun(const TypeFun& type) override;
    void VisitTypeBool(const TypeBool& type) override;
    void VisitTypeNat(const TypeNat& type) override;
    void VisitTypeUnit(const TypeUnit& type) override;
    void VisitTypeList(const TypeList& type) override;
    void VisitTypeTuple(const TypeTuple& type) override;
    void VisitTypeRecord(const TypeRecord& type) override;
    void VisitTypeSum(const TypeSum& type) override;
    void VisitTypeVariant(const TypeVariant& type) override;
};

} // namespace ast
} // namespace stella
