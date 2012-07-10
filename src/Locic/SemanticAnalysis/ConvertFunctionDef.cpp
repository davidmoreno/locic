#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertScope.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertFunctionDef(Context& context, AST::Function* function) {
			const std::string functionName = function->getFullName();
			
			// Find the corresponding semantic function
			// (which MUST have been created previously).
			SEM::Function* semFunction = context.getFunction(functionName);
			
			assert(semFunction != NULL);
			assert(semFunction->typeEnum == SEM::Function::DEFINITION);
			assert(semFunction->scope == NULL);
			
			LocalContext localContext(context, semFunction);
			
			// AST information gives parameter names; SEM information gives parameter variable information.
			const std::vector<AST::TypeVar*>& astParameters = function->parameters;
			const std::vector<SEM::Var*>& semParameters = semFunction->parameters;
			
			const std::size_t paramOffset = function->parentType != NULL ? 1 : 0;
			
			assert(astParameters.size() + paramOffset == semParameters.size());
			
			if(paramOffset == 1) QueryTypeDependencies(context, semParameters.front()->type);
			
			for(std::size_t i = 0; i < astParameters.size(); i++){
				AST::TypeVar* typeVar = astParameters.at(i);
				SEM::Var* paramVar = semParameters.at(paramOffset + i);
				
				// Ensure that any dependencies (e.g. structs) of the parameter
				// types are included in the module by querying them.
				QueryTypeDependencies(context, paramVar->type);
				
				// Create a mapping from the parameter's name to its variable information.
				if(!localContext.defineFunctionParameter(typeVar->name, paramVar)) {
					printf("Semantic Analysis Error: cannot share names between function parameters.\n");
					return false;
				}
			}
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			SEM::Scope* scope = ConvertScope(localContext, function->scope);
			
			if(scope == NULL) {
				return false;
			}
			
			SEM::Type* returnType = semFunction->type->functionType.returnType;
			
			if(!returnType->isVoid()) {
				// Functions with non-void return types must return a value.
				if(!WillScopeReturn(scope)) {
					printf("Semantic Analysis Error: Control reaches end of function with non-void return type (i.e. need to add a return statement).\n");
					return false;
				}
			} else {
				// Need to add a void return statement in case the program didn't.
				scope->statementList.push_back(SEM::Statement::ReturnVoid());
			}
			
			semFunction->scope = scope;
			return true;
		}
		
	}
	
}

