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
};

class TypeVisitor {
public:
    virtual ~TypeVisitor() = default;

    virtual void Visit(const Type& type);

    virtual void VisitTypeFun(const TypeFun& type) = 0;
    virtual void VisitTypeBool(const TypeBool& type) = 0;
    virtual void VisitTypeNat(const TypeNat& type) = 0;
    virtual void VisitTypeUnit(const TypeUnit& type) = 0;
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
};

class BaseTypeVisitor : public TypeVisitor {
public:
    virtual ~BaseTypeVisitor() = default;

    virtual void VisitDefaultType(const Type& type);

    void VisitTypeFun(const TypeFun& type) override;
    void VisitTypeBool(const TypeBool& type) override;
    void VisitTypeNat(const TypeNat& type) override;
    void VisitTypeUnit(const TypeUnit& type) override;
};

} // namespace ast
} // namespace stella
