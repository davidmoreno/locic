#include <string>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/ScopeElement.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		ScopeElement ScopeElement::Namespace(SEM::Namespace* nameSpace) {
			ScopeElement element(NAMESPACE);
			element.data_.nameSpace = nameSpace;
			return element;
		}
		
		ScopeElement ScopeElement::TypeAlias(SEM::TypeAlias* typeAlias) {
			ScopeElement element(TYPEALIAS);
			element.data_.typeAlias = typeAlias;
			return element;
		}
		
		ScopeElement ScopeElement::TypeInstance(SEM::TypeInstance* typeInstance) {
			ScopeElement element(TYPEINSTANCE);
			element.data_.typeInstance = typeInstance;
			return element;
		}
		
		ScopeElement ScopeElement::Function(SEM::Function* function) {
			ScopeElement element(FUNCTION);
			element.data_.function = function;
			return element;
		}
		
		ScopeElement ScopeElement::Scope(SEM::Scope* scope) {
			ScopeElement element(SCOPE);
			element.data_.scope = scope;
			return element;
		}
		
		ScopeElement ScopeElement::SwitchCase(SEM::SwitchCase* switchCase) {
			ScopeElement element(SWITCHCASE);
			element.data_.switchCase = switchCase;
			return element;
		}
		
		ScopeElement ScopeElement::CatchClause(SEM::CatchClause* catchClause) {
			ScopeElement element(CATCHCLAUSE);
			element.data_.catchClause = catchClause;
			return element;
		}
		
		ScopeElement ScopeElement::Loop() {
			return ScopeElement(LOOP);
		}
		
		ScopeElement ScopeElement::ScopeAction(const std::string& state) {
			ScopeElement element(SCOPEACTION);
			element.scopeActionState_ = state;
			return element;
		}
		
		ScopeElement ScopeElement::TryScope() {
			return ScopeElement(TRYSCOPE);
		}
		
		ScopeElement::Kind ScopeElement::kind() const {
			return kind_;
		}
		
		bool ScopeElement::isNamespace() const {
			return kind() == NAMESPACE;
		}
		
		bool ScopeElement::isTypeAlias() const {
			return kind() == TYPEALIAS;
		}
		
		bool ScopeElement::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		bool ScopeElement::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool ScopeElement::isScope() const {
			return kind() == SCOPE;
		}
		
		bool ScopeElement::isSwitchCase() const {
			return kind() == SWITCHCASE;
		}
		
		bool ScopeElement::isCatchClause() const {
			return kind() == CATCHCLAUSE;
		}
		
		bool ScopeElement::isLoop() const {
			return kind() == LOOP;
		}
		
		bool ScopeElement::isScopeAction() const {
			return kind() == SCOPEACTION;
		}
		
		bool ScopeElement::isTryScope() const {
			return kind() == TRYSCOPE;
		}
		
		SEM::Namespace* ScopeElement::nameSpace() const {
			assert(isNamespace());
			return data_.nameSpace;
		}
		
		SEM::TypeAlias* ScopeElement::typeAlias() const {
			assert(isTypeAlias());
			return data_.typeAlias;
		}
		
		SEM::TypeInstance* ScopeElement::typeInstance() const {
			assert(isTypeInstance());
			return data_.typeInstance;
		}
		
		SEM::Function* ScopeElement::function() const {
			assert(isFunction());
			return data_.function;
		}
		
		SEM::Scope* ScopeElement::scope() const {
			assert(isScope());
			return data_.scope;
		}
		
		SEM::SwitchCase* ScopeElement::switchCase() const {
			assert(isSwitchCase());
			return data_.switchCase;
		}
		
		SEM::CatchClause* ScopeElement::catchClause() const {
			assert(isCatchClause());
			return data_.catchClause;
		}
		
		const std::string& ScopeElement::scopeActionState() const {
			assert(isScopeAction());
			return scopeActionState_;
		}
		
		bool ScopeElement::hasName() const {
			return (isNamespace() && !nameSpace()->name().empty()) || isTypeAlias() || isTypeInstance() || isFunction();
		}
		
		std::string ScopeElement::name() const {
			assert(hasName());
			if (isNamespace()) {
				return nameSpace()->name().last();
			} else if (isTypeAlias()) {
				return typeAlias()->name().last();
			} else if (isTypeInstance()) {
				return typeInstance()->name().last();
			} else if (isFunction()) {
				return function()->name().last();
			} else {
				return "";
			}
		}
		
		ScopeElement::ScopeElement(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
	}
	
}

