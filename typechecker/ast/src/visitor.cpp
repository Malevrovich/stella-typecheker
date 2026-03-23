#include "stella/ast/visitor.hpp"

#include "stella/ast/ast.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/unit.hpp"

namespace stella {
namespace ast {

void NodeVisitor::Visit(const NodeBase& node) { node.Accept(*this); }

void TypeVisitor::Visit(const Type& type) { type.Accept(*this); }

void NodeProgram::Accept(NodeVisitor& visitor) const { visitor.VisitProgram(*this); }

void NodeParamDecl::Accept(NodeVisitor& visitor) const { visitor.VisitParamDecl(*this); }

void NodeExprAbstraction::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprAbstraction(*this);
}

void NodeDeclFun::Accept(NodeVisitor& visitor) const { visitor.VisitDeclFun(*this); }

void NodeExprApplication::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprApplication(*this);
}

void NodeExprConstInt::Accept(NodeVisitor& visitor) const { visitor.VisitExprConstInt(*this); }

void NodeExprSucc::Accept(NodeVisitor& visitor) const { visitor.VisitExprSucc(*this); }

void NodeExprPred::Accept(NodeVisitor& visitor) const { visitor.VisitExprPred(*this); }

void NodeExprIsZero::Accept(NodeVisitor& visitor) const { visitor.VisitExprIsZero(*this); }

void NodeExprNatRec::Accept(NodeVisitor& visitor) const { visitor.VisitExprNatRec(*this); }

void NodeExprConstTrue::Accept(NodeVisitor& visitor) const { visitor.VisitExprConstTrue(*this); }

void NodeExprConstFalse::Accept(NodeVisitor& visitor) const { visitor.VisitExprConstFalse(*this); }

void NodeExprIf::Accept(NodeVisitor& visitor) const { visitor.VisitExprIf(*this); }

void NodeExprVar::Accept(NodeVisitor& visitor) const { visitor.VisitExprVar(*this); }

void TypeFun::Accept(TypeVisitor& visitor) const { visitor.VisitTypeFun(*this); }

void TypeBool::Accept(TypeVisitor& visitor) const { visitor.VisitTypeBool(*this); }

void TypeNat::Accept(TypeVisitor& visitor) const { visitor.VisitTypeNat(*this); }

void BaseNodeVisitor::VisitDefaultNode(const NodeBase& node) {}

void BaseNodeVisitor::VisitProgram(const NodeProgram& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitParamDecl(const NodeParamDecl& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitDeclFun(const NodeDeclFun& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprConstInt(const NodeExprConstInt& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprSucc(const NodeExprSucc& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprPred(const NodeExprPred& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprIsZero(const NodeExprIsZero& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprNatRec(const NodeExprNatRec& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprConstTrue(const NodeExprConstTrue& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprConstFalse(const NodeExprConstFalse& node) {
    VisitDefaultNode(node);
}

void BaseNodeVisitor::VisitExprIf(const NodeExprIf& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprVar(const NodeExprVar& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprAbstraction(const NodeExprAbstraction& node) {
    VisitDefaultNode(node);
}

void BaseNodeVisitor::VisitExprApplication(const NodeExprApplication& node) {
    VisitDefaultNode(node);
}

void BaseNodeVisitor::VisitExprConstUnit(const NodeExprConstUnit& node) {
    VisitDefaultNode(node);
}

void NodeExprConstUnit::Accept(NodeVisitor& visitor) const { visitor.VisitExprConstUnit(*this); }

void TypeUnit::Accept(TypeVisitor& visitor) const { visitor.VisitTypeUnit(*this); }

void BaseTypeVisitor::VisitDefaultType(const Type& type) {}

void BaseTypeVisitor::VisitTypeFun(const TypeFun& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeBool(const TypeBool& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeNat(const TypeNat& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeUnit(const TypeUnit& type) { VisitDefaultType(type); }

} // namespace ast
} // namespace stella
