#include <assert.h>

#include <llvm/InlineAsm.h>
#include <llvm/Value.h>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenValue.hpp>
#include <Locic/CodeGen/GenVTable.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Value* generateLValue(Function& function, SEM::Value* value) {
			if (value->type()->isLValue()) {
				return genValue(function, value, true);
			} else {
				llvm::Value* lValue = genAlloca(function, value->type());
				llvm::Value* rValue = genValue(function, value);
				genStore(function, rValue, lValue, value->type());
				return lValue;
			}
		}
		
		llvm::Value* genValue(Function& function, SEM::Value* value, bool genLValue) {
			assert(value != NULL && "Cannot generate NULL value");
			
			LOG(LOG_INFO, "Generating value %s.",
				value->toString().c_str());
			
			Module& module = function.getModule();
				
			switch (value->kind()) {
				case SEM::Value::CONSTANT: {
					switch (value->constant->getType()) {
						case Locic::Constant::NULLVAL:
							return ConstantGenerator(module).getNullPointer(
									   TypeGenerator(module).getI8PtrType());
									   
						case Locic::Constant::BOOLEAN:
							return ConstantGenerator(module).getI1(value->constant->getBool());
							
						case Locic::Constant::SIGNEDINT: {
							return ConstantGenerator(module).getPrimitiveInt(
									   value->constant->getTypeName(), value->constant->getInt());
						}
						
						case Locic::Constant::UNSIGNEDINT: {
							return ConstantGenerator(module).getPrimitiveInt(
									   value->constant->getTypeName(), value->constant->getUint());
						}
						
						case Locic::Constant::FLOATINGPOINT: {
							switch (value->constant->getFloatType()) {
								case Locic::Constant::FLOAT:
									return ConstantGenerator(module).getFloat(
										value->constant->getFloat());
											   
								case Locic::Constant::DOUBLE:
									return ConstantGenerator(module).getDouble(
										value->constant->getFloat());
											   
								case Locic::Constant::LONGDOUBLE:
									assert(false && "Long double not implemented yet");
									return NULL;
									
								default:
									assert(false && "Unknown float constant type");
									return NULL;
							}
						}
						
						case Locic::Constant::STRING: {
							const std::string stringValue = value->constant->getString();
							
							switch (value->constant->getStringType()) {
								case Locic::Constant::CSTRING: {
									llvm::ArrayType* arrayType =
										TypeGenerator(module).getArrayType(
											TypeGenerator(module).getI8Type(),
											stringValue.size() + 1);
									llvm::Constant* constArray =
										ConstantGenerator(module).getString(
											stringValue.c_str());
									llvm::GlobalVariable* globalArray =
										module.createConstGlobal("cstring_constant",
												arrayType, llvm::GlobalValue::PrivateLinkage, constArray);
									globalArray->setAlignment(1);
									// Convert array to a pointer.
									return function.getBuilder().CreateConstGEP2_32(globalArray, 0, 0);
								}
								
								case Locic::Constant::LOCISTRING: {
									assert(false && "Loci string constants not yet implemented.");
									return NULL;
								}
								
								default:
									assert(false && "Unknown string constant type.");
									return NULL;
							}
						}
						
						default:
							assert(false && "Unknown constant type.");
							return NULL;
					}
				}
				
				case SEM::Value::COPY: {
					// TODO!
					return genValue(function, value->copyValue.value);
				}
				
				case SEM::Value::VAR: {
					SEM::Var* var = value->varValue.var;
					
					switch (var->kind()) {
						case SEM::Var::PARAM:
						case SEM::Var::LOCAL: {
							llvm::Value* val = function.getLocalVarMap().get(var);
							
							if (genLValue) {
								return val;
							} else {
								return genLoad(function, val, value->type());
							}
						}
						
						case SEM::Var::MEMBER: {
							llvm::Value* memberPtr =
								function.getBuilder().CreateConstInBoundsGEP2_32(
									function.getContextValue(), 0,
									module.getMemberVarMap().get(var));
														 
							if (genLValue) {
								return memberPtr;
							} else {
								return genLoad(function, memberPtr, value->type());
							}
						}
						
						default: {
							assert(false && "Unknown variable type in variable access.");
							return NULL;
						}
					}
				}
				
				case SEM::Value::ADDRESSOF: {
					return genValue(function, value->addressOf.value, true);
				}
				
				case SEM::Value::DEREF_POINTER: {
					if (genLValue) {
						return genValue(function, value->derefPointer.value);
					} else {
						return genLoad(function, genValue(function, value->derefPointer.value), value->type());
					}
				}
				
				case SEM::Value::REFERENCEOF: {
					return genValue(function, value->referenceOf.value, true);
				}
				
				case SEM::Value::DEREF_REFERENCE: {
					if (genLValue) {
						return genValue(function, value->derefReference.value);
					} else {
						return genLoad(function, genValue(function, value->derefReference.value), value->type());
					}
				}
				
				case SEM::Value::TERNARY: {
					return function.getBuilder().CreateSelect(genValue(function, value->ternary.condition),
							genValue(function, value->ternary.ifTrue, genLValue),
							genValue(function, value->ternary.ifFalse, genLValue));
				}
				
				case SEM::Value::CAST: {
					llvm::Value* codeValue = genValue(function, value->cast.value, genLValue);
					SEM::Type* sourceType = value->cast.value->type();
					SEM::Type* destType = value->type();
					assert((sourceType->kind() == destType->kind()
							|| sourceType->isNull()
							|| destType->isVoid())
						   && "Types must be in the same group for cast, or "
						   "it should be a cast from null, or a cast to void");
						   
					LOG(LOG_NOTICE, "Generating cast from type %s to type %s.",
						sourceType->toString().c_str(), destType->toString().c_str());
						
					if (destType->isVoid()) {
						// All casts to void have the same outcome.
						return ConstantGenerator(module).getVoidUndef();
					}
					
					switch (sourceType->kind()) {
						case SEM::Type::VOID: {
							return codeValue;
						}
						
						case SEM::Type::NULLT: {
							switch (destType->kind()) {
								case SEM::Type::NULLT:
									return codeValue;
									
								case SEM::Type::POINTER:
								case SEM::Type::FUNCTION:
									return function.getBuilder().CreatePointerCast(codeValue,
											genType(module, destType));
											
								case SEM::Type::OBJECT: {
									assert(false && "TODO");
									return NULL;
								}
								
								default: {
									assert(false && "Invalid cast from null.");
									return NULL;
								}
							}
						}
						
						case SEM::Type::OBJECT: {
							if (sourceType->getObjectType() == destType->getObjectType()) {
								return codeValue;
							}
							
							assert(false && "Casts between named types not implemented.");
							return NULL;
						}
						
						case SEM::Type::REFERENCE: {
							if (genLValue) {
								return function.getBuilder().CreatePointerCast(codeValue,
										genType(module, destType)->getPointerTo());
							} else {
								return function.getBuilder().CreatePointerCast(codeValue, genType(module, destType));
							}
						}
						
						case SEM::Type::POINTER: {
							if (genLValue) {
								return function.getBuilder().CreatePointerCast(codeValue,
										genType(module, destType)->getPointerTo());
							} else {
								return function.getBuilder().CreatePointerCast(codeValue, genType(module, destType));
							}
						}
						
						case SEM::Type::FUNCTION: {
							return codeValue;
						}
						
						case SEM::Type::METHOD: {
							return codeValue;
						}
						
						case SEM::Type::TEMPLATEVAR: {
							return codeValue;
						}
						
						default:
							assert(false && "Unknown type in cast.");
							return NULL;
					}
				}
				
				case SEM::Value::POLYCAST: {
					assert(!genLValue && "Cannot generate interfaces as lvalues in polycast");
					llvm::Value* rawValue = genValue(function, value->polyCast.value);
					SEM::Type* sourceType = value->polyCast.value->type();
					SEM::Type* destType = value->type();
					assert((sourceType->isPointer() || sourceType->isReference())  && "Polycast source type must be pointer or reference.");
					assert((destType->isPointer() || destType->isReference()) && "Polycast dest type must be pointer or reference.");
					SEM::Type* sourceTarget = sourceType->getPointerOrReferenceTarget();
					SEM::Type* destTarget = destType->getPointerOrReferenceTarget();
					assert(destTarget->isInterface() && "Polycast dest target type must be interface");
					
					if (sourceTarget->isInterface()) {
						/*// Get the object pointer (of type i8*).
						llvm::Value* objectPointer = function.getBuilder().CreateExtractValue(rawValue,
														  std::vector<unsigned>(1, 0));
													 
						// Get the vtable pointer.
						llvm::Value* vtablePointer = function.getBuilder().CreateExtractValue(rawValue,
													 std::vector<unsigned>(1, 1));
													 
						// Build the new interface pointer struct with these values.
						llvm::Value* interfaceValue = llvm::UndefValue::get(genType(module, destType));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, objectPointer,
										 std::vector<unsigned>(1, 0));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, vtablePointer,
										 std::vector<unsigned>(1, 1));*/
						return rawValue;
					} else {
						// Cast class pointer to pointer to the opaque struct
						// representing destination interface type.
						llvm::Value* objectPointer = function.getBuilder().CreatePointerCast(rawValue,
								TypeGenerator(module).getI8PtrType());
													 
						// Create the vtable.
						llvm::Value* vtablePointer = genVTable(module, sourceTarget);
						
						// Build the new interface pointer struct with these values.
						llvm::Value* interfaceValue = llvm::UndefValue::get(genType(module, destType));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, objectPointer,
										 std::vector<unsigned>(1, 0));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, vtablePointer,
										 std::vector<unsigned>(1, 1));
						return interfaceValue;
					}
				}
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const std::vector<SEM::Value*>& parameters = value->internalConstruct.parameters;
					llvm::Value* objectValue = genAlloca(function, value->type());
					
					LOG(LOG_INFO, "Type is %s.",
						value->type()->toString().c_str());
						
					genType(module, value->type())->dump();
					
					objectValue->dump();
													  
					for (size_t i = 0; i < parameters.size(); i++) {
						SEM::Value* paramValue = parameters.at(i);
						genStore(function, genValue(function, paramValue),
								 function.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i),
								 paramValue->type());
					}
					
					return objectValue;
				}
				
				case SEM::Value::MEMBERACCESS: {
					const size_t offset = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					if (genLValue) {
						return function.getBuilder().CreateConstInBoundsGEP2_32(
								   genValue(function, value->memberAccess.object, true), 0,
								   offset);
					} else {
						return function.getBuilder().CreateExtractValue(
								   genValue(function, value->memberAccess.object),
								   std::vector<unsigned>(1, offset));
					}
				}
				
				case SEM::Value::FUNCTIONCALL: {
					LOG(LOG_EXCESSIVE, "Generating function call value %s.",
						value->functionCall.functionValue->toString().c_str());
						
					llvm::Value* functionValue = genValue(function, value->functionCall.functionValue);
					assert(functionValue->getType()->isPointerTy());
					llvm::Type* functionType = functionValue->getType()->getPointerElementType();
					assert(functionType->isFunctionTy());
					std::vector<llvm::Value*> parameters;
					const std::vector<SEM::Value*>& paramList = value->functionCall.parameters;
					SEM::Type* returnType = value->type();
					llvm::Value* returnValue = NULL;
					
					if (resolvesToClassType(module, returnType)) {
						returnValue = genAlloca(function, returnType);
						parameters.push_back(returnValue);
					}
					
					LOG(LOG_NOTICE, "Function:");
					functionValue->dump();
					
					for (std::size_t i = 0; i < paramList.size(); i++) {
						llvm::Value* argValue = genValue(function, paramList.at(i));
						
						// When calling var-args functions, all 'char' and
						// 'short' values must be extended to 'int' values,
						// and all 'float' values must be converted to 'double'
						// values.
						if (functionType->isFunctionVarArg()) {
							llvm::Type* argType = argValue->getType();
							const unsigned sizeInBits = argType->getPrimitiveSizeInBits();
							
							if (argType->isIntegerTy() && sizeInBits < module.getTargetInfo().getPrimitiveSize("int")) {
								// Need to extend to int.
								// TODO: this doesn't handle unsigned types; perhaps
								// this code should be moved to semantic analysis.
								argValue = function.getBuilder().CreateSExt(argValue,
										   getPrimitiveType(module, "int"));
							} else if (argType->isFloatingPointTy() && sizeInBits < 64) {
								// Need to extend to double.
								argValue = function.getBuilder().CreateFPExt(argValue,
										   TypeGenerator(module).getDoubleType());
							}
						}
						
						parameters.push_back(argValue);
						
						LOG(LOG_NOTICE, "    Param %llu:",
							(unsigned long long) i);
						argValue->dump();
					}
					
					const size_t numFunctionArgs =
						llvm::cast<llvm::Function>(functionValue)->arg_size();
						
					if (numFunctionArgs != parameters.size()) {
						LOG(LOG_NOTICE, "POSSIBLE ERROR: number of arguments given (%llu) "
							"doesn't match required number (%llu).",
							(unsigned long long) parameters.size(),
							(unsigned long long) numFunctionArgs);
					}
					
					//assert(numFunctionArgs == parameters.size());
					
					llvm::Value* callReturnValue = function.getBuilder().CreateCall(functionValue, parameters);
					
					if (returnValue != NULL) {
						return genLoad(function, returnValue, returnType);
					} else {
						return callReturnValue;
					}
				}
				
				case SEM::Value::FUNCTIONREF: {
					return genFunction(module, value->functionRef.parentType,
						value->functionRef.function);
				}
				
				case SEM::Value::METHODOBJECT: {
					llvm::Value* functionValue = genValue(function, value->methodObject.method);
					assert(functionValue != NULL && "MethodObject requires a valid function");
					llvm::Value* dataPointer = generateLValue(function, value->methodObject.methodOwner);
					assert(dataPointer != NULL && "MethodObject requires a valid data pointer");
					
					assert(value->type()->isMethod());
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(
												   genType(module, value->type()));
												   
					functionValue->dump();
					dataPointer->dump();
					methodValue->dump();
					
					llvm::Value* functionPtr =
						function.getBuilder().CreatePointerCast(functionValue,
								genFunctionType(module, value->type()->getMethodFunctionType(), i8PtrType())->getPointerTo(),
								"dynamic_method_function_ptr");
								
					llvm::Value* contextPtr =
						function.getBuilder().CreatePointerCast(dataPointer, i8PtrType(), "this_ptr_cast_to_void_ptr");
						
					methodValue = function.getBuilder().CreateInsertValue(methodValue, functionPtr, std::vector<unsigned>(1, 0));
					methodValue = function.getBuilder().CreateInsertValue(methodValue, contextPtr, std::vector<unsigned>(1, 1));
					return methodValue;
				}
				
				case SEM::Value::METHODCALL: {
					LOG(LOG_EXCESSIVE, "Generating method call value %s.",
						value->methodCall.methodValue->toString().c_str());
						
					llvm::Value* method = genValue(function, value->methodCall.methodValue);
					llvm::Value* functionValue = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 0));
					llvm::Value* contextPointer = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 1));
					
					std::vector<llvm::Value*> parameters;
					
					SEM::Type* returnType = value->type();
					llvm::Value* returnValue = NULL;
					
					if (resolvesToClassType(module, returnType)) {
						returnValue = genAlloca(function, returnType);
						assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference.");
						parameters.push_back(returnValue);
					}
					
					parameters.push_back(contextPointer);
					
					const std::vector<SEM::Value*>& paramList = value->methodCall.parameters;
					
					for (std::size_t i = 0; i < paramList.size(); i++) {
						LOG(LOG_EXCESSIVE, "Generating method call argument %s.",
							paramList.at(i)->toString().c_str());
						parameters.push_back(genValue(function, paramList.at(i)));
					}
					
					LOG(LOG_EXCESSIVE, "Creating method call.");
					functionValue->dump();
					contextPointer->dump();
					
					llvm::Value* callReturnValue = function.getBuilder().CreateCall(functionValue, parameters);
					
					if (returnValue != NULL) {
						return genLoad(function, returnValue, returnType);
					} else {
						return callReturnValue;
					}
				}
				
				case SEM::Value::INTERFACEMETHODOBJECT: {
					SEM::Value* method = value->interfaceMethodObject.method;
					llvm::Value* methodOwner = genValue(function, value->interfaceMethodObject.methodOwner);
					
					assert(method->kind() == SEM::Value::FUNCTIONREF);
					
					SEM::Function* interfaceFunction = method->functionRef.function;
					const MethodHash methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					
					llvm::Value* methodHashValue =
						ConstantGenerator(module).getI32(methodHash);
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(
						genType(module, value->type()));
					
					LOG(LOG_INFO, "Interface method is:");
					methodValue->dump();
					
					LOG(LOG_INFO, "Interface method owner is:");
					methodOwner->dump();
					
					methodValue = function.getBuilder().CreateInsertValue(methodValue, methodOwner, std::vector<unsigned>(1, 0));
					methodValue = function.getBuilder().CreateInsertValue(methodValue, methodHashValue, std::vector<unsigned>(1, 1));
					
					LOG(LOG_INFO, "Interface method is:");
					methodValue->dump();
					
					return methodValue;
				}
				
				case SEM::Value::INTERFACEMETHODCALL: {
					SEM::Value* method = value->interfaceMethodCall.methodValue;
					const std::vector<SEM::Value*>& paramList = value->interfaceMethodCall.parameters;
					llvm::Value* methodValue = genValue(function, method);
					
					llvm::Value* contextValue = function.getBuilder().CreateExtractValue(methodValue,
						std::vector<unsigned>(1, 0), "context");
					
					llvm::Value* objectPointer = function.getBuilder().CreateExtractValue(contextValue,
						std::vector<unsigned>(1, 0), "object");
					
					llvm::Value* vtablePointer = function.getBuilder().CreateExtractValue(contextValue,
						std::vector<unsigned>(1, 1), "vtable");
					
					llvm::Value* methodHashValue = function.getBuilder().CreateExtractValue(methodValue,
						std::vector<unsigned>(1, 1), "methodHash");
					
					const ConstantGenerator constantGen(module);
					
					llvm::Value* vtableSizeValue =
						constantGen.getI32(VTABLE_SIZE);
					
					llvm::Value* vtableOffsetValue =
						function.getBuilder().CreateURem(methodHashValue, vtableSizeValue, "vtableOffset");
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(constantGen.getI32(0));
					vtableEntryGEP.push_back(constantGen.getI32(2));
					vtableEntryGEP.push_back(vtableOffsetValue);
					
					llvm::Value* vtableEntryPointer =
						function.getBuilder().CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					
					llvm::Value* methodFunctionPointer =
						function.getBuilder().CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					
					SEM::Type* functionType = method->type()->getInterfaceMethodFunctionType();
					llvm::Type* methodFunctionType = genFunctionType(module, functionType,
						TypeGenerator(module).getI8PtrType());
					
					llvm::Value* castedMethodFunctionPointer = function.getBuilder().CreatePointerCast(
						methodFunctionPointer, methodFunctionType->getPointerTo(), "castedMethodFunctionPointer");
					
					SEM::Type* returnType = functionType->getFunctionReturnType();
					
					std::vector<llvm::Value*> parameters;
					llvm::Value* returnValue = NULL;
					
					if (resolvesToClassType(module, returnType)) {
						returnValue = genAlloca(function, returnType);
						assert(returnValue != NULL && "Must have lvalue for holding class return value so it can be passed by reference.");
						parameters.push_back(returnValue);
					}
					
					parameters.push_back(objectPointer);
					
					for (std::size_t i = 0; i < paramList.size(); i++) {
						LOG(LOG_EXCESSIVE, "Generating method call argument %s.",
							paramList.at(i)->toString().c_str());
						parameters.push_back(genValue(function, paramList.at(i)));
					}
					
					LOG(LOG_EXCESSIVE, "Creating interface call assembly.");
					llvm::FunctionType* asmFunctionType =
						TypeGenerator(module).getFunctionType(voidType(),
							std::vector<llvm::Type*>(1, TypeGenerator(module).getI32Type()));
					llvm::InlineAsm* setEax = llvm::InlineAsm::get(asmFunctionType, "movl $0, %eax", "r,~{eax}", true);
					function.getBuilder().CreateCall(setEax, std::vector<llvm::Value*>(1, methodHashValue));
					
					LOG(LOG_EXCESSIVE, "Creating interface method call.");
					
					llvm::Value* callReturnValue =
						function.getBuilder().CreateCall(castedMethodFunctionPointer, parameters);
					
					if (returnValue != NULL) {
						return genLoad(function, returnValue, returnType);
					} else {
						return callReturnValue;
					}
					
					/*// Get the 'this' record, which is the
					// pair of the 'this' pointer and the
					// method vtable pointer.
					llvm::Value* thisRecord = arg++;
					
					// Get the 'this' pointer.
					llvm::Value* thisPointer = function.getBuilder().CreateExtractValue(thisRecord, std::vector<unsigned>(1, 0), "thisPointer");
					
					// Get the vtable pointer.
					llvm::Value* vtablePointer = function.getBuilder().CreateExtractValue(thisRecord,
												 std::vector<unsigned>(1, 1), "vtablePointer");
												 
					const MethodHash methodHash = CreateMethodNameHash(function->name().last());
					const size_t offset = methodHash % VTABLE_SIZE;
					
					std::vector<llvm::Value*> vtableEntryGEP;
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 0)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, 2)));
					vtableEntryGEP.push_back(llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(32, offset)));
					
					llvm::Value* vtableEntryPointer = function.getBuilder().CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
					llvm::Value* methodFunctionPointer = function.getBuilder().CreateLoad(vtableEntryPointer, "methodFunctionPointer");
					llvm::Type* methodFunctionType = genFunctionType(function->type(), thisPointer->getType());
					llvm::Value* castedMethodFunctionPointer = function.getBuilder().CreatePointerCast(
								methodFunctionPointer, methodFunctionType->getPointerTo(), "castedMethodFunctionPointer");
					std::vector<llvm::Value*> arguments;
					
					if (returnVar != NULL) {
						arguments.push_back(returnVar);
					}
					
					arguments.push_back(thisPointer);
					
					while (arg != generatedFunction->arg_end()) {
						arguments.push_back(arg++);
					}
					
					llvm::FunctionType* asmFunctionType = llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(), false);
					const std::string assembly = makeString("movl $$%llu, %%eax",
															(unsigned long long) methodHash);
															
					llvm::InlineAsm* setEax = llvm::InlineAsm::get(asmFunctionType, assembly, "~eax", true);
					function.getBuilder().CreateCall(setEax);
					
					const bool isVoidReturnType = returnType->isVoid() || returnType->isClassOrTemplateVar();
					
					llvm::Value* methodCallValue = function.getBuilder().CreateCall(castedMethodFunctionPointer,
												   arguments, isVoidReturnType ? "" : "methodCallValue");
												   
					if (isVoidReturnType) {
						function.getBuilder().CreateRetVoid();
					} else {
						function.getBuilder().CreateRet(methodCallValue);
					}
					
					LOG(LOG_INFO, "Interface method is:");
					methodValue->dump();
					assert(false && "TODO");
					return NULL;*/
				}
				
				default:
					assert(false && "Unknown value enum.");
					return NULL;
			}
		}
		
	}
	
}

