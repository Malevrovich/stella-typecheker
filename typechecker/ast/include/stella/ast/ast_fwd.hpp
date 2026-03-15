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
class NodeExprConstTrue;
class NodeExprConstFalse;
class NodeExprIf;
class NodeExprVar;
class NodeExprAbstraction;
class NodeExprApplication;
class NodeDeclFun;

class TypeFun;
class TypeBool;
class TypeNat;

class NodeVisitor;
class TypeVisitor;

} // namespace ast
} // namespace stella