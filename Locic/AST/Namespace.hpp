#ifndef LOCIC_AST_NAMESPACE_HPP
#define LOCIC_AST_NAMESPACE_HPP

#include <string>
#include <vector>

#include <Locic/String.hpp>
#include <Locic/AST/Function.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/TypeInstance.hpp>

namespace Locic {

	namespace AST {
	
		struct Namespace;
		
		struct NamespaceData {
			std::vector< AST::Node<Function> > functions;
			std::vector< AST::Node<Namespace> > namespaces;
			std::vector< AST::Node<TypeInstance> > typeInstances;
		};
		
		struct Namespace {
			std::string name;
			AST::Node<NamespaceData> data;
			
			inline Namespace(const std::string& n, AST::Node<NamespaceData> d)
				: name(n), data(d) { }
				
			inline std::string toString() const {
				std::string s = name.empty() ? "RootNamespace(" : makeString("Namespace[name = %s](", name.c_str());
				
				bool isFirst = true;
				
				for(auto node : data->functions) {
					if(!isFirst) s += ", ";
					
					isFirst = false;
					s += node.toString();
				}
				
				for(auto node : data->namespaces) {
					if(!isFirst) s += ", ";
					
					isFirst = false;
					s += node.toString();
				}
				
				for(auto node : data->typeInstances) {
					if(!isFirst) s += ", ";
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
				return s;
			}
		};
		
		typedef std::vector<Node<Namespace>> NamespaceList;
		
	}
	
}

#endif