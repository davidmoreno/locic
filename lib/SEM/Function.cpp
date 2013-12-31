#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Function* Function::Decl(bool isMethod, bool isStatic, Type* type,
								 const Name& name, const std::vector<Var*>& parameters) {
			const bool defaultImplementation = false;
			return new Function(isMethod, isStatic, defaultImplementation, type, name, parameters, NULL);
		}
		
		Function* Function::Def(bool isMethod, bool isStatic, Type* type,
								const Name& name, const std::vector<Var*>& parameters, Scope* scope) {
			const bool defaultImplementation = false;
			return new Function(isMethod, isStatic, defaultImplementation, type, name, parameters, scope);
		}
		
		Function* Function::DefDefault(bool isStatic, Type* type, const Name& name) {
			// Only methods can have default implementations.
			const bool isMethod = true;
			
			const bool defaultImplementation = true;
			
			// No parameters need to be created, since
			// they're only used for generating the
			// implementation of a function.
			const auto parameters = std::vector<Var*>();
			
			return new Function(isMethod, isStatic, defaultImplementation, type, name, parameters, NULL);	
		}
		
		Function::Function(bool isM, bool isS, bool hasD, Type* t, const Name& n, const std::vector<Var*>& p, Scope* s)
			: isMethod_(isM),
			  isStatic_(isS),
			  hasDefaultImplementation_(hasD),
			  type_(t), name_(n),
			  parameters_(p), scope_(s) {
			assert(type_ != NULL);
			assert(!(hasDefaultImplementation_ && scope_ != NULL));
		}
		
		const Name& Function::name() const {
			return name_;
		}
		
		Type* Function::type() const {
			return type_;
		}
		
		bool Function::isDeclaration() const {
			return !isDefinition();
		}
		
		bool Function::isDefinition() const {
			return hasDefaultImplementation_ || scope_ != NULL;
		}
		
		bool Function::isMethod() const {
			return isMethod_;
		}
		
		bool Function::isStatic() const {
			return isStatic_;
		}
		
		bool Function::hasDefaultImplementation() const {
			return hasDefaultImplementation_;
		}
		
		const std::vector<Var*>& Function::parameters() const {
			return parameters_;
		}
		
		const Scope& Function::scope() const {
			assert(isDefinition());
			return *scope_;
		}
		
		Function* Function::createDecl() const {
			return Decl(isMethod(), isStatic(), type(), name(), parameters());
		}
		
		Function* Function::fullSubstitute(const Name& declName, const Map<TemplateVar*, Type*>& templateVarMap) const {
			assert(isDeclaration());
			
			// Parameter types need to be substituted.
			std::vector<SEM::Var*> substitutedParam;
			
			for (size_t i = 0; i < parameters().size(); i++) {
				SEM::Var* originalParam = parameters().at(i);
				assert(originalParam->kind() == Var::PARAM);
				substitutedParam.push_back(Var::Param(
											   originalParam->type()->substitute(templateVarMap)));
			}
			
			return Decl(isMethod(), isStatic(),
						type()->substitute(templateVarMap),
						declName, substitutedParam);
		}
		
		void Function::setScope(Scope* newScope) {
			assert(scope_ == NULL);
			scope_ = newScope;
			assert(scope_ != NULL);
		}
		
		std::string Function::toString() const {
			if (isDeclaration()) {
				return makeString("FunctionDeclaration(name: %s, isMethod: %s, isStatic: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStatic() ? "Yes" : "No",
								  type()->toString().c_str());
			} else if (hasDefaultImplementation()) {
				return makeString("FunctionDefaultDefinition(name: %s, isMethod: %s, isStatic: %s, type: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStatic() ? "Yes" : "No",
								  type()->toString().c_str());
			} else {
				return makeString("FunctionDefinition(name: %s, isMethod: %s, isStatic: %s, type: %s, scope: %s)",
								  name().toString().c_str(),
								  isMethod() ? "Yes" : "No",
								  isStatic() ? "Yes" : "No",
								  type()->toString().c_str(),
								  scope().toString().c_str());
			}
		}
		
	}
	
}
