#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <locic/Name.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class Scope;
		class Type;
		class TypeInstance;
		class Var;
		
		class Function: public TemplatedObject {
			public:
				Function(Name pName, ModuleScope pModuleScope);
				
				const Name& name() const;
				
				void setType(const Type* pType);
				const Type* type() const;
				
				const ModuleScope& moduleScope() const;
				
				bool isDeclaration() const;
				
				bool isDefinition() const;
                                
                                void setPrimitive(bool pIsPrimitive);
                                bool isPrimitive() const;
				
				void setMethod(bool pIsMethod);
				bool isMethod() const;
				
				void setStaticMethod(bool pIsStaticMethod);
				bool isStaticMethod() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::map<std::string, TemplateVar*>& namedTemplateVariables();
				const std::map<std::string, TemplateVar*>& namedTemplateVariables() const;
				
				const Predicate& constPredicate() const;
				void setConstPredicate(Predicate predicate);
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				void setParameters(std::vector<Var*> pParameters);
				const std::vector<Var*>& parameters() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				void setScope(std::unique_ptr<Scope> newScope);
				const Scope& scope() const;
				
				std::string toString() const;
				
			private:
				bool isPrimitive_;
				bool isMethod_, isStaticMethod_;
				const Type* type_;
				Name name_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::map<std::string, TemplateVar*> namedTemplateVariables_;
				Predicate constPredicate_;
				Predicate requiresPredicate_;
				std::vector<Var*> parameters_;
				std::map<std::string, Var*> namedVariables_;
				
				ModuleScope moduleScope_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
