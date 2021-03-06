#ifndef LOCIC_CODEGEN_VTABLE_HPP
#define LOCIC_CODEGEN_VTABLE_HPP

#include <stdint.h>

#include <list>
#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Map.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		const size_t VTABLE_SIZE = 10;
		
		/**
		 * All method names are 'hashed' to a 64 bit
		 * unsigned integer, for use in virtual method
		 * tables.
		 */
		typedef uint64_t MethodHash;
		
		MethodHash CreateMethodNameHash(const std::string& methodName);
		
		llvm::Value* CreateHashValue(Module& module, const std::string& methodName);
		
		class VirtualTable {
			public:
				static VirtualTable CalculateFromHashes(const std::vector<MethodHash>& methods);
				
				static VirtualTable CalculateFromNames(const std::vector<std::string>& methods);
				
				const std::vector< std::list<MethodHash> >& table() const;
				
				std::string toString() const;
				
				std::string toStringWithMapping(const Map<MethodHash, std::string>& mapping) const;
				
			private:
				VirtualTable();
				
				std::vector< std::list<MethodHash> > table_;
				
		};
		
	}
	
}

#endif
