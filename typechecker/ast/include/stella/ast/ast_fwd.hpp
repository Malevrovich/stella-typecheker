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

class NodeVisitor;
class TypeVisitor;

} // namespace ast
} // namespace stella