#include <cassert>
#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/ConvertValue.hpp>

namespace Locic {

	namespace SemanticAnalysis {

SEM::Value* ConvertValue(LocalContext& context, AST::Value* value) {
	assert(value != NULL && "Cannot convert NULL AST::Value");
	
	switch(value->typeEnum) {
		case AST::Value::CONSTANT: {
			if(value->constant->getType() == Locic::Constant::NULLVAL){
				return SEM::Value::Constant(value->constant, SEM::Type::Null(SEM::Type::CONST));
			}else if(value->constant->getType() == Locic::Constant::STRING && value->constant->getStringType() == Locic::Constant::CSTRING){
				// C strings have the type 'const char *', as opposed to just a
				// type name, so their type needs to be generated specially.
				SEM::TypeInstance * charType = context.getNode(Name::Absolute() + "char").getTypeInstance();
				assert(charType != NULL && "Couldn't find char constant type");
				SEM::Type * constCharPtrType = SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE,
					SEM::Type::Named(SEM::Type::CONST, SEM::Type::LVALUE, charType));
				return SEM::Value::Constant(value->constant, constCharPtrType);
			}else{
				const std::string typeName = value->constant->getTypeName();
				SEM::TypeInstance * typeInstance = context.getNode(Name::Absolute() + typeName).getTypeInstance();
				if(typeInstance == NULL){
					printf("Couldn't find '::%s' constant type.\n", typeName.c_str());
				}
				assert(typeInstance != NULL && "Couldn't find constant type");
				return SEM::Value::Constant(value->constant, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, typeInstance));
			}
			
			assert(false && "Invalid if fallthrough in ConvertValue for constant");
			return NULL;
		}
		case AST::Value::NAMEREF: {
			const Locic::Name& name = value->nameRef.name;
			
			// Check if it could be a local variable.
			// Local variables must just be a single plain string,
			// and be a relative name (so no precending '::').
			if(name.size() == 1 && name.isRelative()){
				SEM::Var* semVar = context.findLocalVar(name.first());
				
				if(semVar != NULL) {
					return SEM::Value::VarValue(semVar);
				}
			}
			
			// Not a local variable => do a symbol lookup.
			SEM::NamespaceNode node = context.getNode(name);
			
			if(node.isNone()){
				printf("Semantic Analysis Error: Couldn't find symbol or value '%s'.\n", name.toString().c_str());
				return NULL;
			}
			
			if(node.isNamespace()){
				printf("Semantic Analysis Error: Namespace '%s' is not a valid value.\n", name.toString().c_str());
				return NULL;
			}
			
			if(node.isFunction()){
				SEM::Function * function = node.getFunction();
				assert(function != NULL && "Function pointer must not be NULL (as indicated by isFunction() being true)");
				assert(!function->isMethod && "TODO");
				
				return SEM::Value::FunctionRef(function, function->type);
			}else if(node.isTypeInstance()){
				SEM::TypeInstance * typeInstance = node.getTypeInstance();
				assert(typeInstance != NULL && "Type instance pointer must not be NULL (as indicated by isTypeInstance() being true)");
				
				if(typeInstance->isInterface()){
					printf("Semantic Analysis Error: Can't construct interface type '%s' (full name: '%s').\n",
						name.toString().c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
				
				SEM::Function * function = typeInstance->lookup(typeInstance->name + "Default").getFunction();
				if(function == NULL){
					printf("Semantic Analysis Error: Couldn't find default constructor for type '%s' (full name: '%s').\n",
						name.toString().c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
				
				return SEM::Value::FunctionRef(function, function->type);
			}else{
				assert(false && "Unknown node for name reference");
				return NULL;
			}
			
			assert(false && "Invalid if-statement fallthrough in ConvertValue for name reference");
			return NULL;
		}
		case AST::Value::MEMBERREF: {
			const std::string& memberName = value->memberRef.name;
			SEM::Var* semVar = context.getThisVar(memberName);
			
			if(semVar == NULL){
				printf("Semantic Analysis Error: member variable '@%s' not found.\n", memberName.c_str());
				return NULL;
			}
			
			return SEM::Value::VarValue(semVar);
		}
		case AST::Value::ADDRESSOF: {
			SEM::Value* operand = ConvertValue(context, value->addressOf.value);
			if(operand == NULL) return NULL;
		
			if(operand->type->isLValue) {
				return SEM::Value::AddressOf(operand);
			}
			
			printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
			return NULL;
		}
		case AST::Value::DEREFERENCE: {
			SEM::Value* operand = ConvertValue(context, value->dereference.value);
			if(operand == NULL) return NULL;
		
			if(operand->type->isPointer()) {
				return SEM::Value::Deref(operand);
			}
			
			printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
			return NULL;
		}
		case AST::Value::TERNARY: {
			SEM::Value* cond = ConvertValue(context, value->ternary.condition);
			if(cond == NULL) return NULL;
			
			SEM::TypeInstance * boolType = context.getNode(Name::Absolute() + "bool").getTypeInstance();
			assert(boolType != NULL && "Couldn't find bool type");
			SEM::Value* boolValue = ImplicitConvertValueToType(cond, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, boolType));
			
			SEM::Value* ifTrue = ConvertValue(context, value->ternary.ifTrue);
			SEM::Value* ifFalse = ConvertValue(context, value->ternary.ifFalse);
			
			SEM::Type* targetType = new SEM::Type(*(UnifyTypes(ifTrue->type, ifFalse->type)));
			
			// Can only result in an lvalue if both possible results are lvalues.
			if(ifTrue->type->isLValue == SEM::Type::RVALUE || ifFalse->type->isLValue == SEM::Type::RVALUE) {
				targetType->isLValue = SEM::Type::RVALUE;
			}
			
			SEM::Value* castIfTrue = ImplicitConvertValueToType(ifTrue, targetType);
			SEM::Value* castIfFalse = ImplicitConvertValueToType(ifFalse, targetType);
			
			return SEM::Value::Ternary(boolValue, castIfTrue, castIfFalse);
		}
		case AST::Value::CAST: {
			SEM::Type* type = ConvertType(context, value->cast.targetType, SEM::Type::RVALUE);
			SEM::Value* val = ConvertValue(context, value->cast.value);
			
			if(type == NULL || val == NULL) {
				return NULL;
			}
			
			std::string s;
			switch(value->cast.castKind){
				case AST::Value::CAST_STATIC:
					s = "STATIC";
					break;
				case AST::Value::CAST_CONST:
					s = "CONST";
					break;
				case AST::Value::CAST_DYNAMIC:
					s = "DYNAMIC";
					break;
				case AST::Value::CAST_REINTERPRET:
					s = "REINTERPRET";
					break;
				default:
					assert(false && "Unknown cast kind");
					return NULL;
			}
			
			printf("Internal Compiler Error: Casts of kind '%s' not yet implemented.\n",
				s.c_str());
			return NULL;
		}
		case AST::Value::INTERNALCONSTRUCT: {
			const std::vector<AST::Value *>& astValues = value->internalConstruct.parameters;
		
			SEM::TypeInstance * thisTypeInstance = context.getThisTypeInstance();
			
			if(astValues.size() != thisTypeInstance->variables.size()){
				printf("Semantic Analysis Error: Internal constructor called with wrong number of arguments; received %lu, expected %lu.\n",
					(unsigned long) astValues.size(), (unsigned long) thisTypeInstance->variables.size());
				return NULL;
			}
			
			std::vector<SEM::Value *> semValues(astValues.size(), NULL);
			
			StringMap<SEM::Var *>::Range range = thisTypeInstance->variables.range();
			for(; !range.empty(); range.popFront()){
				SEM::Var * var = range.front().value();
				const size_t id = var->id;
				
				assert(semValues.at(id) == NULL);
				
				SEM::Value * semValue = ConvertValue(context, astValues.at(id));
				if(semValue == NULL) return NULL;
				
				SEM::Value * semParam = ImplicitConvertValueToType(semValue, var->type);
				semValues.at(id) = semParam;
			}
			
			for(size_t i = 0; i < semValues.size(); i++){
				assert(semValues.at(i) != NULL);
			}
			
			return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
		}
		case AST::Value::MEMBERACCESS: {
			const std::string memberName = value->memberAccess.memberName;
		
			SEM::Value * object = ConvertValue(context, value->memberAccess.object);
			if(object == NULL) return NULL;
			
			SEM::Type * objectType = object->type;
			if(objectType->typeEnum != SEM::Type::NAMED){
				printf("Semantic Analysis Error: Can't access member of non-object type.\n");
				return NULL;
			}
		
			SEM::TypeInstance * typeInstance = objectType->namedType.typeInstance;
			assert(typeInstance != NULL);
			
			if(typeInstance->isStructDef()){
				// Look for struct variables.
				Optional<SEM::Var *> varResult = typeInstance->variables.tryGet(memberName);
				if(varResult.hasValue()){
					SEM::Var * var = varResult.getValue();
					if(objectType->isLValue){
						return SEM::Value::MemberAccess(object, var->id, var->type);
					}else{
						// If the struct type is an R-value, then the member must
						// also be (preventing assignments to R-value members).
						SEM::Type * memberType = new SEM::Type(*(var->type));
						memberType->isLValue = false;
						return SEM::Value::MemberAccess(object, var->id, memberType);
					}
				}else{
					printf("Semantic Analysis Error: Can't access struct member '%s' in type '%s'.\n", memberName.c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
			}else if(typeInstance->isClass() || typeInstance->isPrimitive() || typeInstance->isInterface()){
				// Look for class methods.
				Optional<SEM::Function *> functionResult = typeInstance->functions.tryGet(memberName);
				
				if(functionResult.hasValue()){
					SEM::Function * function = functionResult.getValue();
					
					if(!function->isMethod){
						printf("Semantic Analysis Error: Cannot call static function '%s' in type '%s'.\n",
							function->name.last().c_str(), typeInstance->name.toString().c_str());
						return NULL;
					}
					
					SEM::Type * methodType = SEM::Type::Method(SEM::Type::MUTABLE, SEM::Type::RVALUE, typeInstance, function->type);
					
					return SEM::Value::MethodObject(function, object, methodType);
				}else{
					printf("Semantic Analysis Error: Can't find method '%s' in type '%s'.\n", memberName.c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
			}else if(typeInstance->isStructDecl()){
				printf("Semantic Analysis Error: Can't access member '%s' in unspecified struct type '%s'.\n", memberName.c_str(), typeInstance->name.toString().c_str());
				return NULL;
			}
			
			assert(false && "Invalid switch fallthrough in ConvertValue for member access");
			return NULL;
		}
		case AST::Value::FUNCTIONCALL: {
			assert(value->functionCall.functionValue != NULL && "Cannot call NULL function value");
			SEM::Value* functionValue = ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL) {
				return NULL;
			}
			
			switch(functionValue->type->typeEnum){
				case SEM::Type::FUNCTION:
				{
					const std::vector<SEM::Type*>& typeList = functionValue->type->functionType.parameterTypes;
					const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
					
					if(functionValue->type->functionType.isVarArg){
						if(astValueList.size() < typeList.size()) {
							printf("Semantic Analysis Error: Var Arg Function [%s] called with %lu number of parameters; expected at least %lu.\n",
								functionValue->toString().c_str(), astValueList.size(), typeList.size());
							return NULL;
						}
					}else{
						if(astValueList.size() != typeList.size()) {
							printf("Semantic Analysis Error: Function [%s] called with %lu number of parameters; expected %lu.\n",
								functionValue->toString().c_str(), astValueList.size(), typeList.size());
							return NULL;
						}
					}
					
					assert(astValueList.size() >= typeList.size());
					
					std::vector<SEM::Value*> semValueList;
					
					for(std::size_t i = 0; i < astValueList.size(); i++){
						SEM::Value* semArgValue = ConvertValue(context, astValueList.at(i));
						
						if(semArgValue == NULL) return NULL;
						
						SEM::Value* param = (i < typeList.size()) ? ImplicitConvertValueToType(semArgValue, typeList.at(i)) : semArgValue;
						
						semValueList.push_back(param);
					}
					
					return SEM::Value::FunctionCall(functionValue, semValueList, functionValue->type->functionType.returnType);
				}
				case SEM::Type::METHOD:
				{
					SEM::Type * functionType = functionValue->type->methodType.functionType;
					
					const std::vector<SEM::Type*>& typeList = functionType->functionType.parameterTypes;
					const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
					
					assert(!functionType->functionType.isVarArg && "Methods cannot be var args");
					
					if(typeList.size() != astValueList.size()) {
						printf("Semantic Analysis Error: Method called with %lu number of parameters; expected %lu.\n", astValueList.size(), typeList.size());
						return NULL;
					}
					
					std::vector<SEM::Value*> semValueList;
					
					for(std::size_t i = 0; i < astValueList.size(); i++){
						SEM::Value* semArgValue = ConvertValue(context, astValueList.at(i));
						
						if(semArgValue == NULL) return NULL;
						
						SEM::Value* param = ImplicitConvertValueToType(semArgValue, typeList.at(i));
						
						semValueList.push_back(param);
					}
					
					return SEM::Value::MethodCall(functionValue, semValueList, functionType->functionType.returnType);
				}
				default:
				{
					printf("Semantic Analysis Error: Can't call type that isn't a function or a method.\n");
					return NULL;
				}
			}
		}
		default:
			assert(false && "Unknown AST::Value type enum");
			return NULL;
	}
}

}

}


