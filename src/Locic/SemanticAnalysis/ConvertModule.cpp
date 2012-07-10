#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertModule.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		bool ConvertModule(GlobalContext& globalContext, AST::Module* module, SEM::Module* semModule) {
			ModuleContext moduleContext(globalContext, semModule);
			
			for(std::size_t i = 0; i < module->functions.size(); i++) {
				AST::Function* astFunction = module->functions.at(i);
				
				if(astFunction->typeEnum == AST::Function::DEFINITION){
					if(!ConvertFunctionDef(moduleContext, astFunction)) {
						return false;
					}
				}
			}
			
			for(std::size_t i = 0; i < module->typeInstances.size(); i++){
				AST::TypeInstance * astTypeInstance = module->typeInstances.at(i);
				
				if(astTypeInstance->typeEnum == AST::TypeInstance::CLASSDEF){
					if(!ConvertClassDef(moduleContext, astTypeInstance)){
						return false;
					}
				}
			}
			
			return true;
		}
		
	}
	
}

