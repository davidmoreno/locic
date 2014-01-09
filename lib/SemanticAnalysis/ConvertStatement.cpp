#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>

namespace locic {

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
					throw std::runtime_error("Unknown statement kind.");
				}
			}
		}
		
		SEM::Statement* ConvertStatement(Context& context, AST::Node<AST::Statement> statement) {
			switch(statement->typeEnum) {
				case AST::Statement::VALUE: {
					const auto value = ConvertValue(context, statement->valueStmt.value);
					if (statement->valueStmt.hasVoidCast) {
						if (value->type()->isVoid()) {
							throw TodoException(makeString("Void explicitly ignored in expression '%s'.",
								value->toString().c_str()));
						}
						return SEM::Statement::ValueStmt(SEM::Value::Cast(SEM::Type::Void(), value));
					} else {
						if (!value->type()->isVoid()) {
							throw TodoException(makeString("Non-void value result ignored in expression '%s'.",
								value->toString().c_str()));
						}
						return SEM::Statement::ValueStmt(value);
					}
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
							SEM::Type::Object(boolType, NO_TEMPLATE_ARGS));
							
					return SEM::Statement::If(boolValue, ifTrue, ifFalse);
				}
				case AST::Statement::WHILE: {
					const auto condition = ConvertValue(context, statement->whileStmt.condition);
					const auto whileTrue = ConvertScope(context, statement->whileStmt.whileTrue);
					
					const auto boolType = context.getBuiltInType("bool");
					
					const std::vector<SEM::Type*> NO_TEMPLATE_ARGS;
					
					const auto boolValue = ImplicitCast(condition,
							SEM::Type::Object(boolType, NO_TEMPLATE_ARGS));
					
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
					const auto& astTypeVarNode = statement->varDecl.typeVar;
					const auto& astInitialValueNode = statement->varDecl.value;
					
					const auto semValue = ConvertValue(context, astInitialValueNode);
					
					// Convert the AST type var.
					const bool isMemberVar = false;
					const auto semVar = ConvertInitialisedVar(context, isMemberVar, astTypeVarNode, semValue->type());
					assert(!semVar->isAny());
					
					// Cast the initialise value to the variable's type.
					// (The variable conversion above should have ensured
					// this will work.)
					const auto semInitialiseValue = ImplicitCast(semValue, semVar->constructType());
					assert(!semInitialiseValue->type()->isVoid());
					
					// Add the variable to the SEM scope.
					const auto semScope = context.node().getSEMScope();
					semScope->localVariables().push_back(semVar);
					
					// Generate the initialise statement.
					return SEM::Statement::InitialiseStmt(semVar, semInitialiseValue);
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
						const auto semValue = ConvertValue(context, statement->returnStmt.value);
						
						// Cast the return value to the function's
						// specified return type.
						const auto castValue = ImplicitCast(semValue, context.getParentFunctionReturnType());
						
						return SEM::Statement::Return(castValue);
					}
				}
				default:
					throw std::runtime_error("Unknown statement kind.");
			}
		}
		
	}
	
}


