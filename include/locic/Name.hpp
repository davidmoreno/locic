#ifndef LOCIC_NAME_HPP
#define LOCIC_NAME_HPP

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace locic{

	class Name{
		typedef std::vector<std::string> ListType;
		typedef ListType::const_iterator CItType;
	
		public:
			Name();
			
			explicit Name(bool isNameAbsolute);
			
			Name(const Name& name, size_t substrSize);
			
			Name(const Name& prefix, const std::string& suffix);
			
			Name(const Name& prefix, const Name& suffix);
			
			static Name Absolute();
			
			static Name Relative();
			
			bool operator==(const Name& name) const;
			
			Name operator+(const std::string& name) const;
			
			Name makeAbsolute(const Name& name) const;
			
			bool isRelative() const;
			
			bool isAbsolute() const;
			
			bool isPrefixOf(const Name& name) const;
			
			bool isExactPrefixOf(const Name& name) const;
			
			std::string toString(bool addPrefix = true) const;
			
			std::string genString() const;
			
			std::string first() const;
			
			std::string last() const;
			
			std::string onlyElement() const;
			
			bool empty() const;
			
			std::size_t size() const;
			
			std::string at(std::size_t i) const;
			
			std::string revAt(std::size_t i) const;
			
			Name substr(std::size_t substrSize) const;
			
			Name concat(const Name& suffix) const;
			
			Name append(const std::string& suffix) const;
			
			Name getPrefix() const;
			
			ListType::iterator begin();
			
			ListType::iterator end();
			
			ListType::const_iterator begin() const;
			
			ListType::const_iterator end() const;
			
		private:
			bool isAbsolute_;
			ListType list_;
			
	};
	
	std::string CanonicalizeMethodName(const std::string& name);

}

#endif
