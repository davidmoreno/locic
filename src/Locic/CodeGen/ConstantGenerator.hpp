#ifndef LOCIC_CODEGEN_CONSTANTGENERATOR_HPP
#define LOCIC_CODEGEN_CONSTANTGENERATOR_HPP

#include <llvm/Constants.h>
#include <llvm/Type.h>

#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		class ConstantGenerator {
			public:
				inline ConstantGenerator(const Module& module)
					: module_(module) { }
				
				inline llvm::UndefValue* getVoidUndef() const {
					return llvm::UndefValue::get(
						llvm::Type::getVoidTy(module_.getLLVMContext()));
				}
				
				inline llvm::UndefValue* getUndef(llvm::Type* type) const {
					return llvm::UndefValue::get(type);
				}
				
				inline llvm::ConstantPointerNull* getNullPointer(llvm::PointerType* pointerType) const {
					return llvm::ConstantPointerNull::get(pointerType);
				}
				
				inline llvm::ConstantInt* getI1(bool value) const {
					return llvm::ConstantInt::get(module_.getLLVMContext(),
						llvm::APInt(1, value ? 1 : 0));
				}
					
				inline llvm::ConstantInt* getSize(size_t sizeValue) const {
					const size_t sizeTypeWidth = module_.getTargetInfo().getPrimitiveSize("size_t");
					return llvm::ConstantInt::get(module_.getLLVMContext(),
						llvm::APInt(sizeTypeWidth, sizeValue));
				}
				
				inline llvm::ConstantInt* getPrimitiveInt(const std::string& primitiveName, long long intValue) const {
					const size_t primitiveWidth = module_.getTargetInfo().getPrimitiveSize(primitiveName);
					return llvm::ConstantInt::get(module_.getLLVMContext(),
						llvm::APInt(primitiveWidth, intValue));
				}
				
				inline llvm::ConstantFP* getFloat(double value) const {
					return llvm::ConstantFP::get(module_.getLLVMContext(),
						llvm::APFloat(value));
				}
				
				inline llvm::Constant* getString(const std::string& value, bool withNullTerminator = true) const {
					return llvm::ConstantDataArray::getString(module_.getLLVMContext(),
						value, withNullTerminator);
				}
				
			private:
				const Module& module_;
				
		};
		
	}
	
}

#endif