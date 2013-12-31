#ifndef LOCIC_SEM_SCOPE_HPP
#define LOCIC_SEM_SCOPE_HPP

#include <string>
#include <vector>

namespace locic {

	namespace SEM {
		
		class Statement;
		class Var;
	
		class Scope {
			public:
				Scope();
				
				std::vector<Var*>& localVariables();
				const std::vector<Var*>& localVariables() const;
				
				std::vector<Statement*>& statements();
				const std::vector<Statement*>& statements() const;
				
				std::string toString() const;
				
			private:
				std::vector<Var*> localVariables_;
				std::vector<Statement*> statementList_;
				
		};
		
	}
	
}

#endif