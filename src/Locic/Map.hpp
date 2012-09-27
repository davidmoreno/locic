#ifndef LOCIC_MAP_HPP
#define LOCIC_MAP_HPP

#include <cassert>
#include <map>
#include <string>
#include <boost/utility.hpp>
#include <Locic/Optional.hpp>

namespace Locic{

	template <typename Key, typename Value>
	class Map: boost::noncopyable{
		private:
			typedef std::map<Key, Value> MapType;
			typedef typename MapType::const_iterator CItType;
			typedef typename MapType::iterator ItType;
	
		public:
			class Pair{
				public:
					Pair(const Key& key, const Value& value)
						: key_(key), value_(value){ }
					
					Key key() const{
						return key_;
					}
					
					Value value() const{
						return value_;
					}
					
				private:
					Key key_;
					Value value_;
				
			};
			
			class Range{
				public:
					Range(const CItType& begin,
						const CItType& end)
						: begin_(begin), end_(end){ }
					
					bool empty() const{
						return begin_ == end_;
					}
					
					Pair front() const{
						assert(!empty());
						return Pair(begin_->first, begin_->second);
					}
					
					Pair back() const{
						assert(!empty());
						CItType end = end_;
						--end;
						return Pair(end->first, end->second);
					}
				
					void popFront(){
						assert(!empty());
						++begin_;
					}
					
					void popBack(){
						assert(!empty());
						--end_;
					}
			
				private:
					CItType begin_, end_;
			};
			
			Range range() const{
				return Range(map_.begin(), map_.end());
			}
		
			bool tryInsert(const Key& key, const Value& value){
				std::pair<ItType, bool> p = map_.insert(std::make_pair(key, value));
				return p.second;
			}
			
			void insert(const Key& key, const Value& value){
				assert(tryInsert(key, value));
			}
			
			Optional<Value> tryGet(const Key& key) const{
				CItType it = map_.find(key);
				if(it != map_.end()){
					return make_optional(it->second);
				}else{
					return Optional<Value>();
				}
			}
			
			Value get(const Key& key) const{
				Optional<Value> value = tryGet(key);
				assert(value);
				return value.get();
			}
			
			bool has(const Key& key) const{
				return tryGet(key);
			}
			
			std::size_t size() const{
				return map_.size();
			}
			
			void insertRange(const Range& range){
				map_.insert(range.begin_, range.end_);
			}
			
		private:
			MapType map_;
		
	};
	
	template <typename Value>
	class StringMap: public Map<std::string, Value>{ };

}

#endif