#include <cassert>
#include <cstdio>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/ConvertValue.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool WillStatementReturn(SEM::Statement* statement) {
			switch(statement->kind()) {
				case SEM::Statement::VALUE: {
					return false;
				}
				case SEM::Statement::SCOPE: {
					return WillScopeReturn(statement->getScope());
				}
				case SEM::Statement::INITIALISE: {
					return false;
				}
				case SEM::Statement::IF: {
					return WillScopeReturn(statement->getIfTrueScope()) &&
						   WillScopeReturn(statement->getIfFalseScope());
				}
				case SEM::Statement::WHILE: {
					return WillScopeReturn(statement->getWhileScope());
				}
				case SEM::Statement::RETURN: {
					return true;
				}
				default: {
					printf("Internal Compiler Error: Unknown statement type in WillStatementReturn.\n");
					return false;
				}
			}
		}
		
		SEM::Statement* ConvertStatement(Context& context, AST::Node<AST::Statement> statement) {
			switch(statement->typeEnum) {
				case AST::Statement::VALUE: {
					SEM::Value* value = ConvertValue(context, statement->valueStmt.value);
					return SEM::Statement::ValueStmt(value);
				}
				case AST::Statement::SCOPE: {
					return SEM::Statement::ScopeStmt(ConvertScope(context, statement->scopeStmt.scope));
				}
				case AST::Statement::IF: {
					SEM::Value* condition = ConvertValue(context, statement->ifStmt.condition);
					SEM::Scope* ifTrue = ConvertScope(context, statement->ifStmt.ifTrue);
					SEM::Scope* ifFalse = ConvertScope(context, statement->ifStmt.ifFalse);
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					const std::vector<SEM::Type*> NO_TEMPLATE_ARGS;
					
					SEM::Value* boolValue = ImplicitCast(condition,
							SEM::Type::Object(SEM::Type::CONST, boolType, NO_TEMPLATE_ARGS));
							
					return SEM::Statement::If(boolValue, ifTrue, ifFalse);
				}
				case AST::Statement::WHILE: {
					SEM::Value* condition = ConvertValue(context, statement->whileStmt.condition);
					SEM::Scope* whileTrue = ConvertScope(context, statement->whileStmt.whileTrue);
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					const std::vector<SEM::Type*> NO_TEMPLATE_ARGS;
					
					SEM::Value* boolValue = ImplicitCast(condition,
							SEM::Type::Object(SEM::Type::CONST, boolType, NO_TEMPLATE_ARGS));
							
					return SEM::Statement::While(boolValue, whileTrue);
				}
				// TODO: replace code in parser with this.
				/*case AST::Statement::FOR: {
					AST::TypeVar* astTypeVar = statement->forStmt.typeVar;
					AST::Node<AST::Value> astRangeValue = statement->forStmt.rangeValue;
					
					SEM::Value* semRangeValue = ConvertValue(context, astRangeValue);
					
					// Create an anonymous variable to hold the range.
					SEM::Var* rangeVar = SEM::Var::Local(semRangeValue->type());
					
					// Start off by assigning the range value to the variable.
					SEM::Statement* setRangeVar = SEM::Statement::InitialiseStmt(rangeVar, semRangeValue);
					
					
					
					// Check whether a type annotation has been used.
					const bool autoType = (astTypeVar->type == NULL);
					
					// If type is 'auto', infer it from type value.
					SEM::Type* varType = autoType ? semValue->type() : ConvertType(context, astTypeVar->type);
					
					assert(varType != NULL);
					if (varType->isVoid()) {
						printf("Semantic Analysis Error: Local variable cannot have void type.\n");
						// TODO: throw exception.
						assert(false);
						return NULL;
					}
					
					return SEM::Statement::ScopeStmt(
				}*/
				case AST::Statement::VARDECL: {
					AST::Node<AST::TypeVar> astTypeVarNode = statement->varDecl.typeVar;
					assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
					AST::Node<AST::Value> astInitialValueNode = statement->varDecl.value;
					
					SEM::Value* semValue = ConvertValue(context, astInitialValueNode);
					
					// Check whether a type annotation has been used.
					const bool autoType = (astTypeVarNode->namedVar.type->typeEnum == AST::Type::UNDEFINED);
					
					// If type is 'auto', infer it from type value.
					SEM::Type* varType = autoType ? semValue->type() : ConvertType(context, astTypeVarNode->namedVar.type);
					
					assert(varType != NULL);
					if (varType->isVoid()) {
						printf("Semantic Analysis Error: Local variable cannot have void type.\n");
						// TODO: throw exception.
						assert(false);
						return NULL;
					}
					
					// TODO: implement 'final'.
					const bool isLvalMutable = SEM::Type::MUTABLE;
					
					SEM::Type* lvalType = makeLvalType(context, astTypeVarNode->namedVar.usesCustomLval, isLvalMutable, varType);
					
					SEM::Var* semVar = SEM::Var::Local(lvalType);
					
					const Node localVarNode = Node::Variable(astTypeVarNode, semVar);
					
					if(!context.node().tryAttach(astTypeVarNode->namedVar.name, localVarNode)) {
						throw TodoException(makeString("Local variable name '%s' already exists.",
							astTypeVarNode->namedVar.name.c_str()));
					}
					
					SEM::Scope* semScope = context.node().getSEMScope();
					assert(semScope != NULL);
					semScope->localVariables().push_back(semVar);
					
					// Initialise variable.
					return SEM::Statement::InitialiseStmt(semVar, ImplicitCast(semValue, varType));
				}
				case AST::Statement::RETURN: {
					if (statement->returnStmt.value.get() == NULL) {
						// Void return statement (i.e. return;)
						if (!context.getParentFunctionReturnType()->isVoid()) {
							throw TodoException(makeString("Cannot return void in function '%s' with non-void return type.",
								context.lookupParentFunction().getSEMFunction()->name().toString().c_str()));
						}
						
						return SEM::Statement::ReturnVoid();
					} else {
						SEM::Value* semValue = ConvertValue(context, statement->returnStmt.value);
						
						SEM::Value* castValue = ImplicitCast(semValue, context.getParentFunctionReturnType());
						
						return SEM::Statement::Return(castValue);
					}
				}
				default:
					printf("Internal Compiler Error: Unknown statement type in 'ConvertStatement'.\n");
					// TODO: throw exception.
					assert(false);
					return NULL;
			}
		}
		
	}
	
}

