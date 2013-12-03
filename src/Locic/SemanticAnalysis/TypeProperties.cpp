#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/MethodPattern.hpp>
#include <Locic/SemanticAnalysis/Node.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		SEM::Value* CallProperty(SEM::Value* value, const std::string& propertyName, const std::vector<SEM::Value*>& args) {
			SEM::Type* type = value->type();
			assert(type->isObject());
			assert(type->getObjectType()->hasProperty(propertyName));
			
			SEM::Function* function = type->getObjectType()->getProperty(propertyName);
			
			SEM::Value* functionRef = SEM::Value::FunctionRef(type, function, type->generateTemplateVarMap());
			SEM::Value* methodRef = SEM::Value::MethodObject(functionRef, value);
			
			return SEM::Value::MethodCall(methodRef, args);
		}
	}
	
}

