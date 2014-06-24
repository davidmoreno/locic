#ifndef LOCIC_CODEGEN_GENFUNCTION_HPP
#define LOCIC_CODEGEN_GENFUNCTION_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* typeInstance, SEM::ModuleScope* moduleScope);
		
		llvm::Function* genFunctionDecl(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genFunctionDef(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function);
		
		llvm::Function* genTemplateFunctionStub(Module& module, SEM::TemplateVar* templateVar, SEM::Function* function);
		
	}
	
}

#endif
