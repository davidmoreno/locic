#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			const Name fullName = astSymbol->createName();
			assert(fullName.size() == astSymbol->size());
			
			Map<SEM::TemplateVar*, SEM::Type*> templateVarMap;
			
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				const auto& astTemplateArgs = astSymbolElement->templateArguments();
				const size_t numTemplateArguments = astTemplateArgs->size();
				
				const Name name = fullName.substr(i + 1);
				
				const Node objectNode = context.lookupName(name);
				if (objectNode.isTypeInstance()) {
					SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
					const size_t numTemplateVariables = typeInstance->templateVariables().size();
					if (numTemplateVariables != numTemplateArguments) {
						throw TodoException(makeString("Incorrect number of template "
							"arguments provided for type '%s'; %llu were required, "
							"but %llu were provided.", name.toString().c_str(),
							(unsigned long long) numTemplateVariables,
							(unsigned long long) numTemplateArguments));
					}
					
					for (size_t j = 0; j < numTemplateArguments; j++) {
						SEM::Type* templateTypeValue = ConvertType(context, astTemplateArgs->at(j));
						
						if (templateTypeValue->isInterface()) {
							throw TodoException(makeString("Cannot use abstract type '%s' "
							"as template parameter %llu for type '%s'.",
							templateTypeValue->getObjectType()->name().toString().c_str(),
							(unsigned long long) j, name.toString().c_str()));
						}
						
						templateVarMap.insert(typeInstance->templateVariables().at(j),
							templateTypeValue);
					}
				} else {
					if (numTemplateArguments > 0) {
						throw TodoException(makeString("%llu template "
							"arguments provided for non-type node '%s'; none should be provided.",
							(unsigned long long) numTemplateArguments,
							name.toString().c_str()));
					}
				}
			}
			
			return templateVarMap;
		}
		
		std::vector<SEM::Type*> GetTemplateValues(Context& context, const AST::Node<AST::Symbol>& astSymbol) {
			std::vector<SEM::Type*> templateArguments;
			for (size_t i = 0; i < astSymbol->size(); i++) {
				const auto& astSymbolElement = astSymbol->at(i);
				for (const auto& astTemplateArg: *(astSymbolElement->templateArguments())) {
					templateArguments.push_back(ConvertType(context, astTemplateArg));
				}
			}
			return templateArguments;
		}
		
		SEM::Type* ConvertType(Context& context, const AST::Node<AST::Type>& type) {
			switch(type->typeEnum) {
				case AST::Type::UNDEFINED: {
					assert(false && "Cannot convert undefined type.");
					return NULL;
				}
				case AST::Type::BRACKET: {
					return ConvertType(context, type->getBracketTarget());
				}
				case AST::Type::CONST: {
					return ConvertType(context, type->getConstTarget())->createConstType();
				}
				case AST::Type::VOID: {
					return SEM::Type::Void();
				}
				case AST::Type::OBJECT: {
					const AST::Node<AST::Symbol>& symbol = type->objectType.symbol;
					assert(!symbol->empty());
					
					const Name name = symbol->createName();
					const Node objectNode = context.lookupName(name);
					
					const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = GenerateTemplateVarMap(context, symbol);
					
					if(objectNode.isTypeInstance()) {
						SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
						
						assert(templateVarMap.size() == typeInstance->templateVariables().size());
						
						std::vector<SEM::Type*> templateArguments;
						for(size_t i = 0; i < typeInstance->templateVariables().size(); i++){
							templateArguments.push_back(templateVarMap.get(typeInstance->templateVariables().at(i)));
						}
						
						return SEM::Type::Object(SEM::Type::MUTABLE, typeInstance, templateArguments);
					}else if(objectNode.isTemplateVar()) {
						assert(templateVarMap.empty());
						
						SEM::TemplateVar* templateVar = objectNode.getSEMTemplateVar();
						
						return SEM::Type::TemplateVarRef(SEM::Type::MUTABLE, templateVar);
					}else{
						throw TodoException(makeString("Unknown type with name '%s'.", name.toString().c_str()));
					}
				}
				case AST::Type::REFERENCE: {
					SEM::Type* refType = ConvertType(context, type->getReferenceTarget());
					return SEM::Type::Reference(refType);
				}
				case AST::Type::FUNCTION: {
					SEM::Type* returnType = ConvertType(context, type->functionType.returnType);
					
					std::vector<SEM::Type*> parameterTypes;
					
					const AST::Node<AST::TypeList>& astParameterTypes = type->functionType.parameterTypes;
					for (const auto& astParamType: *astParameterTypes) {
						SEM::Type* paramType = ConvertType(context, astParamType);
						
						if(paramType->isVoid()) {
							throw TodoException("Parameter type (inside function type) cannot be void.");
						}
						
						parameterTypes.push_back(paramType);
					}
					
					return SEM::Type::Function(type->functionType.isVarArg, returnType, parameterTypes);
				}
				default:
					assert(false && "Unknown AST::Node<AST::Type> type enum.");
					return NULL;
			}
		}
		
	}
	
}
