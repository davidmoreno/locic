#ifndef LOCIC_SEMANTICANALYSIS_METHODSET_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSET_HPP

#include <map>
#include <string>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		class Context;
		
		class MethodSetElement {
			public:
				MethodSetElement(bool isConst, bool isNoExcept, bool isStatic, const SEM::Type* returnType, const std::vector<const SEM::Type*>& parameterTypes);
				
				MethodSetElement(const MethodSetElement&) = default;
				MethodSetElement(MethodSetElement&&) = default;
				MethodSetElement& operator=(const MethodSetElement&) = default;
				MethodSetElement& operator=(MethodSetElement&&) = default;
				
				bool isConst() const;
				bool isNoExcept() const;
				bool isStatic() const;
				const SEM::Type* returnType() const;
				const std::vector<const SEM::Type*>& parameterTypes() const;
				
				const SEM::Type* createFunctionType(bool isTemplated) const;
				
				bool operator<(const MethodSetElement& methodSetElement) const;
				
			private:
				bool isConst_, isNoExcept_, isStatic_;
				const SEM::Type* returnType_;
				std::vector<const SEM::Type*> parameterTypes_;
				
		};
		
		class MethodSet {
			public:
				enum FilterReason {
					NotFiltered,
					NotFound,
					IsMutator
				};
				
				typedef std::pair<std::string, MethodSetElement> Element;
				typedef std::vector<Element> ElementSet;
				typedef ElementSet::const_iterator iterator;
				
				typedef std::pair<std::string, FilterReason> Filter;
				typedef std::vector<Filter> FilterSet;
				
				static const MethodSet* getEmpty(const Context& context);
				
				static const MethodSet* get(const Context& context, ElementSet elements, FilterSet filters);
				
				MethodSet(MethodSet&&) = default;
				MethodSet& operator=(MethodSet&&) = default;
				
				const Context& context() const;
				
				iterator begin() const;
				iterator end() const;
				
				size_t size() const;
				
				iterator find(const std::string& name) const;
				
				bool hasMethod(const std::string& name) const;
				
				const FilterSet& filterSet() const;
				FilterReason getFilterReason(const std::string& name) const;
				
				const MethodSet* substitute(const SEM::TemplateVarMap& templateAssignments) const;
				
				bool operator<(const MethodSet& methodSet) const;
				
			private:
				// Non-copyable.
				MethodSet(const MethodSet&) = delete;
				MethodSet& operator=(const MethodSet&) = delete;
				
				MethodSet(const Context& context, ElementSet elements, FilterSet filters);
				
				const Context& context_;
				ElementSet elements_;
				FilterSet filters_;
				
		};
		
		const MethodSet* getMethodSetForRequiresPredicate(SEM::TemplateVar* templateVar, const SEM::Predicate& requiresPredicate);
		
		const MethodSet* getMethodSetForObjectType(Context& context, const SEM::Type* objectType);
		
		const MethodSet* getTypeMethodSet(Context& context, const SEM::Type* type);
		
		const MethodSet* intersectMethodSets(const MethodSet* setA, const MethodSet* setB);
		
		const MethodSet* unionMethodSets(const MethodSet* setA, const MethodSet* setB);
		
		bool methodSetSatisfiesRequirement(const MethodSet* checkSet, const MethodSet* requireSet);
		
	}
	
}

#endif