#ifndef LOCIC_CODEGEN_GENVALUE_HPP
#define LOCIC_CODEGEN_GENVALUE_HPP

#include <Locic/CodeGen/LLVMIncludes.hpp>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Function.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Value* generateLValue(Function& function, SEM::Value* value);
		
		llvm::Value* genValue(Function& function, SEM::Value* value);
		
	}
	
}

#endif