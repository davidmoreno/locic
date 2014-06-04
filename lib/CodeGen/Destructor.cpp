#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isPrimitive()) {
				return primitiveTypeHasDestructor(module, typeInstance);
			}
			
			return typeInstance->isClass() || typeInstance->isDatatype() || typeInstance->isUnionDatatype();
		}
		
		void genDestructorCall(Function& function, SEM::Type* type, llvm::Value* value) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasDestructor(module, type->getObjectType())) {
					return;
				}
				
				// Call destructor.
				const auto destructorFunction = genDestructorFunction(module, type->getObjectType());
				
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getI8PtrType());
				
				if (type->templateArguments().empty()) {
					function.getBuilder().CreateCall(destructorFunction, { castValue });
				} else {
					function.getBuilder().CreateCall(destructorFunction, std::vector<llvm::Value*>{ computeTemplateGenerator(function, type), castValue });
				}
			} else if (type->isTemplateVar()) {
				// TODO!!
			}
		}
		
		void genUnionDestructor(Function& function, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isUnionDatatype());
			
			const auto contextValue = function.getContextValue(typeInstance);
			
			const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(contextValue, 0, 0);
			const auto loadedTag = function.getBuilder().CreateLoad(loadedTagPtr);
			
			const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(contextValue, 0, 1);
			
			const auto endBB = function.createBasicBlock("end");
			const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, typeInstance->variants().size());
			
			std::vector<llvm::BasicBlock*> caseBlocks;
			
			uint8_t tag = 0;
			for (const auto variantTypeInstance: typeInstance->variants()) {
				const auto matchBB = function.createBasicBlock("tagMatch");
				const auto tagValue = ConstantGenerator(function.module()).getI8(tag++);
				
				switchInstruction->addCase(tagValue, matchBB);
				
				function.selectBasicBlock(matchBB);
				
				const auto variantType = variantTypeInstance->selfType();
				const auto unionValueType = genType(function.module(), variantType);
				const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
				
				genDestructorCall(function, variantType, castedUnionValuePtr);
				
				function.getBuilder().CreateBr(endBB);
			}
			
			function.selectBasicBlock(endBB);
		}
		
		llvm::FunctionType* destructorFunctionType(Module& module, SEM::TypeInstance* typeInstance) {
			TypeGenerator typeGen(module);
			if (typeInstance->templateVariables().empty()) {
				return typeGen.getVoidFunctionType({ typeGen.getI8PtrType() });
			} else {
				return typeGen.getVoidFunctionType(std::vector<llvm::Type*>{ templateGeneratorType(module), typeGen.getI8PtrType() });
			}
		}
		
		llvm::Function* genDestructorFunction(Module& module, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isClass() || typeInstance->isPrimitive() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			const auto mangledName = mangleDestructorName(typeInstance);
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			// --- Generate function declaration.
			const auto functionType = destructorFunctionType(module, typeInstance);
			
			const auto linkage = getFunctionLinkage(typeInstance, typeInstance->moduleScope());
			
			const auto llvmFunction = createLLVMFunction(module, functionType, linkage, mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			if (typeInstance->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveDestructor(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			assert(typeInstance->isClass() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			if (typeInstance->isClassDecl()) {
				return llvmFunction;
			}
			
			assert(typeInstance->isClassDef() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			Function function(module, *llvmFunction, ArgInfo::ContextOnly());
			
			if (typeInstance->isUnionDatatype()) {
				genUnionDestructor(function, typeInstance);
				function.getBuilder().CreateRetVoid();
				return llvmFunction;
			}
			
			assert(typeInstance->isClassDef() || typeInstance->isDatatype());
			
			const auto contextValue = function.getRawContextValue();
			
			// Call the custom destructor function, if one exists.
			const auto methodIterator = typeInstance->functions().find("__destructor");
			if (methodIterator != typeInstance->functions().end()) {
				const auto customDestructor = genFunction(module, typeInstance, methodIterator->second);
				function.getBuilder().CreateCall(customDestructor, contextValue);
			}
			
			const auto& memberVars = typeInstance->variables();
			
			// Call destructors for all objects within the
			// parent object, in *reverse order*.
			for (size_t i = 0; i < memberVars.size(); i++) {
				const auto memberVar = memberVars.at((memberVars.size() - 1) - i);
				const size_t memberIndex = module.getMemberVarMap().get(memberVar);
				const auto memberOffsetValue = genMemberOffset(function, typeInstance->selfType(), memberIndex);
				const auto ptrToMember = function.getBuilder().CreateInBoundsGEP(contextValue, memberOffsetValue);
				genDestructorCall(function, memberVar->type(), ptrToMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
	}
	
}

