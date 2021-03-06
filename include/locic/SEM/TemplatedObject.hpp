#ifndef LOCIC_SEM_TEMPLATEDOBJECT_HPP
#define LOCIC_SEM_TEMPLATEDOBJECT_HPP

#include <vector>

namespace locic {
	
	namespace SEM {
		
		class Predicate;
		class TemplateVar;
		
		class TemplatedObject {
			public:
				virtual std::vector<TemplateVar*>& templateVariables() = 0;
				
				virtual const std::vector<TemplateVar*>& templateVariables() const = 0;
				
				virtual const Predicate& requiresPredicate() const = 0;
				
			protected:
				~TemplatedObject() { }
				
		};
		
	}
	
}

#endif
