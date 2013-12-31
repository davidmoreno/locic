#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>

#include <locic/Map.hpp>
#include <locic/Name.hpp>

namespace locic {
	
	namespace SEM {
	
		class Function;
		class TemplateVar;
		class Var;
		
		class TypeInstance {
			public:
				enum Kind {
					PRIMITIVE,
					STRUCT,
					CLASSDECL,
					CLASSDEF,
					DATATYPE,
					INTERFACE,
					TEMPLATETYPE
				};
				
				TypeInstance(const Name& n, Kind k);
					
				const Name& name() const;
				
				Kind kind() const;
				
				bool isPrimitive() const;
				
				bool isStruct() const;
				
				bool isClassDecl() const;
				
				bool isClassDef() const;
				
				bool isClass() const;
				
				bool isDatatype() const;
				
				bool isInterface() const;
				
				bool isTemplateType() const;
				
				bool isName() const;
				
				// Queries whether all methods are const.
				bool isConstType() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				std::vector<Function*>& functions();
				const std::vector<Function*>& functions() const;
				
				std::vector<Type*>& constructTypes();
				const std::vector<Type*>& constructTypes() const;
				
				// TODO: 'type properties' should be moved out of SEM tree
				//       representation into Semantic Analysis nodes.
				bool hasProperty(const std::string& propertyName) const;
				
				Function* getProperty(const std::string& propertyName) const;
				
				void addProperty(const std::string& propertyName, Function* function);
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				Name name_;
				Kind kind_;
				
				Map<std::string, Function*> typeProperties_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::vector<Var*> variables_;
				std::vector<Function*> functions_;
				std::vector<Type*> constructTypes_;
				
		};
		
	}
	
}

#endif