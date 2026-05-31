#include "stella/ast/visitor.hpp"

#include "stella/ast/asc.hpp"
#include "stella/ast/ast.hpp"
#include "stella/ast/auto.hpp"
#include "stella/ast/base.hpp"
#include "stella/ast/cast.hpp"
#include "stella/ast/exception.hpp"
#include "stella/ast/generic.hpp"
#include "stella/ast/let.hpp"
#include "stella/ast/list.hpp"
#include "stella/ast/match.hpp"
#include "stella/ast/panic.hpp"
#include "stella/ast/record.hpp"
#include "stella/ast/reference.hpp"
#include "stella/ast/sequence.hpp"
#include "stella/ast/sum.hpp"
#include "stella/ast/top_bottom.hpp"
#include "stella/ast/tuple.hpp"
#include "stella/ast/unit.hpp"
#include "stella/ast/variant.hpp"

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

void NodeDeclFunGeneric::Accept(NodeVisitor& visitor) const { visitor.VisitDeclFunGeneric(*this); }

void NodeExprApplication::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprApplication(*this);
}

void NodeExprFix::Accept(NodeVisitor& visitor) const { visitor.VisitExprFix(*this); }

void NodeExprTypeAbstraction::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprTypeAbstraction(*this);
}

void NodeExprTypeApplication::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprTypeApplication(*this);
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

void BaseNodeVisitor::VisitExprFix(const NodeExprFix& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprConstUnit(const NodeExprConstUnit& node) { VisitDefaultNode(node); }

void BaseNodeVisitor::VisitExprTypeAsc(const NodeExprTypeAsc& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitPatternVar(const NodePatternVar& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprLet(const NodeExprLet& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprList(const NodeExprList& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprConsList(const NodeExprConsList& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprHead(const NodeExprHead& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprTail(const NodeExprTail& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprIsEmpty(const NodeExprIsEmpty& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprTuple(const NodeExprTuple& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprDotTuple(const NodeExprDotTuple& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprRecord(const NodeExprRecord& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprDotRecord(const NodeExprDotRecord& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprInl(const NodeExprInl& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprInr(const NodeExprInr& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitPatternInl(const NodePatternInl& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitPatternInr(const NodePatternInr& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitMatchCase(const NodeMatchCase& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitExprMatch(const NodeExprMatch& node) { VisitDefaultNode(node); }

void NodeExprConstUnit::Accept(NodeVisitor& visitor) const { visitor.VisitExprConstUnit(*this); }

void NodeExprTypeAsc::Accept(NodeVisitor& visitor) const { visitor.VisitExprTypeAsc(*this); }

void NodePatternVar::Accept(NodeVisitor& visitor) const { visitor.VisitPatternVar(*this); }

void NodeExprLet::Accept(NodeVisitor& visitor) const { visitor.VisitExprLet(*this); }

void TypeUnit::Accept(TypeVisitor& visitor) const { visitor.VisitTypeUnit(*this); }

void BaseTypeVisitor::VisitDefaultType(const Type& type) {}

void BaseTypeVisitor::VisitTypeFun(const TypeFun& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeBool(const TypeBool& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeNat(const TypeNat& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeUnit(const TypeUnit& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeList(const TypeList& type) { VisitDefaultType(type); }
void BaseTypeVisitor::VisitTypeTuple(const TypeTuple& type) { VisitDefaultType(type); }

void NodeExprList::Accept(NodeVisitor& visitor) const { visitor.VisitExprList(*this); }

void NodeExprConsList::Accept(NodeVisitor& visitor) const { visitor.VisitExprConsList(*this); }

void NodeExprHead::Accept(NodeVisitor& visitor) const { visitor.VisitExprHead(*this); }

void NodeExprTail::Accept(NodeVisitor& visitor) const { visitor.VisitExprTail(*this); }

void NodeExprIsEmpty::Accept(NodeVisitor& visitor) const { visitor.VisitExprIsEmpty(*this); }

void TypeList::Accept(TypeVisitor& visitor) const { visitor.VisitTypeList(*this); }

void NodeExprTuple::Accept(NodeVisitor& visitor) const { visitor.VisitExprTuple(*this); }

void NodeExprDotTuple::Accept(NodeVisitor& visitor) const { visitor.VisitExprDotTuple(*this); }

void TypeTuple::Accept(TypeVisitor& visitor) const { visitor.VisitTypeTuple(*this); }

void NodeExprRecord::Accept(NodeVisitor& visitor) const { visitor.VisitExprRecord(*this); }

void NodeExprDotRecord::Accept(NodeVisitor& visitor) const { visitor.VisitExprDotRecord(*this); }

void TypeRecord::Accept(TypeVisitor& visitor) const { visitor.VisitTypeRecord(*this); }

void BaseTypeVisitor::VisitTypeRecord(const TypeRecord& type) { VisitDefaultType(type); }

void NodeExprInl::Accept(NodeVisitor& visitor) const { visitor.VisitExprInl(*this); }

void NodeExprInr::Accept(NodeVisitor& visitor) const { visitor.VisitExprInr(*this); }

void NodePatternInl::Accept(NodeVisitor& visitor) const { visitor.VisitPatternInl(*this); }

void NodePatternInr::Accept(NodeVisitor& visitor) const { visitor.VisitPatternInr(*this); }

void TypeSum::Accept(TypeVisitor& visitor) const { visitor.VisitTypeSum(*this); }

void BaseTypeVisitor::VisitTypeSum(const TypeSum& type) { VisitDefaultType(type); }

void BaseNodeVisitor::VisitExprVariant(const NodeExprVariant& node) { VisitDefaultNode(node); }
void BaseNodeVisitor::VisitPatternVariant(const NodePatternVariant& node) {
    VisitDefaultNode(node);
}

void BaseTypeVisitor::VisitTypeVariant(const TypeVariant& type) { VisitDefaultType(type); }

void NodeExprSequence::Accept(NodeVisitor& visitor) const { visitor.VisitExprSequence(*this); }

void BaseNodeVisitor::VisitExprSequence(const NodeExprSequence& node) { VisitDefaultNode(node); }

void TypeTop::Accept(TypeVisitor& visitor) const { visitor.VisitTypeTop(*this); }

void TypeBottom::Accept(TypeVisitor& visitor) const { visitor.VisitTypeBottom(*this); }

void BaseTypeVisitor::VisitTypeTop(const TypeTop& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeBottom(const TypeBottom& type) { VisitDefaultType(type); }

void NodeExprPanic::Accept(NodeVisitor& visitor) const { visitor.VisitExprPanic(*this); }

void BaseNodeVisitor::VisitExprPanic(const NodeExprPanic& node) { VisitDefaultNode(node); }

void NodeDeclExceptionType::Accept(NodeVisitor& visitor) const {
    visitor.VisitDeclExceptionType(*this);
}

void BaseNodeVisitor::VisitDeclExceptionType(const NodeDeclExceptionType& node) {
    VisitDefaultNode(node);
}

void NodeDeclExceptionVariant::Accept(NodeVisitor& visitor) const {
    visitor.VisitDeclExceptionVariant(*this);
}

void BaseNodeVisitor::VisitDeclExceptionVariant(const NodeDeclExceptionVariant& node) {
    VisitDefaultNode(node);
}

void NodeExprThrow::Accept(NodeVisitor& visitor) const { visitor.VisitExprThrow(*this); }

void BaseNodeVisitor::VisitExprThrow(const NodeExprThrow& node) { VisitDefaultNode(node); }

void NodeExprTryWith::Accept(NodeVisitor& visitor) const { visitor.VisitExprTryWith(*this); }

void BaseNodeVisitor::VisitExprTryWith(const NodeExprTryWith& node) { VisitDefaultNode(node); }

void NodeExprTryCatch::Accept(NodeVisitor& visitor) const { visitor.VisitExprTryCatch(*this); }

void BaseNodeVisitor::VisitExprTryCatch(const NodeExprTryCatch& node) { VisitDefaultNode(node); }

void NodePatternCastAs::Accept(NodeVisitor& visitor) const { visitor.VisitPatternCastAs(*this); }

void BaseNodeVisitor::VisitPatternCastAs(const NodePatternCastAs& node) { VisitDefaultNode(node); }

void NodeExprTypeCast::Accept(NodeVisitor& visitor) const { visitor.VisitExprTypeCast(*this); }

void BaseNodeVisitor::VisitExprTypeCast(const NodeExprTypeCast& node) { VisitDefaultNode(node); }

void NodeExprTryCastAs::Accept(NodeVisitor& visitor) const { visitor.VisitExprTryCastAs(*this); }

void BaseNodeVisitor::VisitExprTryCastAs(const NodeExprTryCastAs& node) { VisitDefaultNode(node); }

void NodeExprRef::Accept(NodeVisitor& visitor) const { visitor.VisitExprRef(*this); }

void BaseNodeVisitor::VisitExprRef(const NodeExprRef& node) { VisitDefaultNode(node); }

void NodeExprDeref::Accept(NodeVisitor& visitor) const { visitor.VisitExprDeref(*this); }

void BaseNodeVisitor::VisitExprDeref(const NodeExprDeref& node) { VisitDefaultNode(node); }

void NodeExprAssign::Accept(NodeVisitor& visitor) const { visitor.VisitExprAssign(*this); }

void BaseNodeVisitor::VisitExprAssign(const NodeExprAssign& node) { VisitDefaultNode(node); }

void NodeExprConstMemory::Accept(NodeVisitor& visitor) const {
    visitor.VisitExprConstMemory(*this);
}

void BaseNodeVisitor::VisitExprConstMemory(const NodeExprConstMemory& node) {
    VisitDefaultNode(node);
}

void TypeRef::Accept(TypeVisitor& visitor) const { visitor.VisitTypeRef(*this); }

void BaseTypeVisitor::VisitTypeRef(const TypeRef& type) { VisitDefaultType(type); }

void TypeAuto::Accept(TypeVisitor& visitor) const { visitor.VisitTypeAuto(*this); }

void BaseTypeVisitor::VisitTypeAuto(const TypeAuto& type) { VisitDefaultType(type); }

void TypeForAll::Accept(TypeVisitor& visitor) const { visitor.VisitTypeForAll(*this); }

void TypeVar::Accept(TypeVisitor& visitor) const { visitor.VisitTypeVar(*this); }

void BaseTypeVisitor::VisitTypeForAll(const TypeForAll& type) { VisitDefaultType(type); }

void BaseTypeVisitor::VisitTypeVar(const TypeVar& type) { VisitDefaultType(type); }

void BaseNodeVisitor::VisitDeclFunGeneric(const NodeDeclFunGeneric& node) {
    VisitDefaultNode(node);
}

void BaseNodeVisitor::VisitExprTypeAbstraction(const NodeExprTypeAbstraction& node) {
    VisitDefaultNode(node);
}

void BaseNodeVisitor::VisitExprTypeApplication(const NodeExprTypeApplication& node) {
    VisitDefaultNode(node);
}

} // namespace ast
} // namespace stella
