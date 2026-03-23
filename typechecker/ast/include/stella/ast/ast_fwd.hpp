#pragma once

namespace stella {
namespace ast {

class NodeBase;
class Type;

class NodeProgram;
class NodeParamDecl;
class NodeExprConstInt;
class NodeExprSucc;
class NodeExprPred;
class NodeExprIsZero;
class NodeExprNatRec;
class NodeExprConstTrue;
class NodeExprConstFalse;
class NodeExprIf;
class NodeExprVar;
class NodeExprAbstraction;
class NodeExprApplication;
class NodeExprFix;
class NodeDeclFun;

class TypeFun;
class TypeBool;
class TypeNat;
class TypeUnit;

class NodeExprConstUnit;

class NodeExprTypeAsc;

class NodePatternVar;
class NodeExprLet;

class NodeExprList;
class NodeExprConsList;
class NodeExprHead;
class NodeExprTail;
class NodeExprIsEmpty;
class TypeList;

class NodeExprTuple;
class NodeExprDotTuple;
class TypeTuple;

class NodeExprRecord;
class NodeExprDotRecord;
class TypeRecord;

class NodeExprInl;
class NodeExprInr;
class NodePatternInl;
class NodePatternInr;
class NodeMatchCase;
class NodeExprMatch;
class TypeSum;

class NodeVisitor;
class TypeVisitor;

} // namespace ast
} // namespace stella