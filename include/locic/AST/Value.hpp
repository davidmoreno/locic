#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <string>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Name.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		struct Value;
		
		typedef std::vector<Node<Value>> ValueList;
		
		struct Value {
			enum TypeEnum {
				NONE,
				SELF,
				THIS,
				BRACKET,
				LITERAL,
				SYMBOLREF,
				MEMBERREF,
				SIZEOF,
				TERNARY,
				CAST,
				LVAL,
				REF,
				INTERNALCONSTRUCT,
				MEMBERACCESS,
				FUNCTIONCALL
			} typeEnum;
			
			struct {
				std::string specifier;
				Node<Constant> constant;
			} literal;
			
			struct {
				Node<Value> value;
			} bracket;
			
			struct {
				Node<Symbol> symbol;
			} symbolRef;
			
			struct {
				std::string name;
			} memberRef;
			
			struct {
				Node<Type> type;
			} sizeOf;
			
			struct {
				Node<Value> condition, ifTrue, ifFalse;
			} ternary;
			
			enum CastKind {
				CAST_STATIC,
				CAST_CONST,
				CAST_REINTERPRET,
				CAST_DYNAMIC
			};
			
			struct {
				CastKind castKind;
				Node<Type> sourceType;
				Node<Type> targetType;
				Node<Value> value;
			} cast;
			
			struct {
				Node<Type> targetType;
				Node<Value> value;
			} makeLval;
			
			struct {
				Node<Type> targetType;
				Node<Value> value;
			} makeRef;
			
			struct {
				Node<ValueList> parameters;
			} internalConstruct;
			
			struct {
				Node<Value> object;
				std::string memberName;
			} memberAccess;
			
			struct {
				Node<Value> functionValue;
				Node<ValueList> parameters;
			} functionCall;
			
			inline Value() : typeEnum(NONE) { }
			
			inline Value(TypeEnum e) : typeEnum(e) { }
			
			inline static Value* Self() {
				return new Value(SELF);
			}
			
			inline static Value* This() {
				return new Value(THIS);
			}
			
			inline static Value* Bracket(Node<Value> operand) {
				Value* value = new Value(BRACKET);
				value->bracket.value = operand;
				return value;
			}
			
			inline static Value* Literal(const std::string& specifier, const Node<Constant>& constant) {
				Value* value = new Value(LITERAL);
				value->literal.specifier = specifier;
				value->literal.constant = constant;
				return value;
			}
			
			inline static Value* SymbolRef(const Node<Symbol>& symbol) {
				Value* value = new Value(SYMBOLREF);
				value->symbolRef.symbol = symbol;
				return value;
			}
			
			inline static Value* MemberRef(const std::string& name) {
				Value* value = new Value(MEMBERREF);
				value->memberRef.name = name;
				return value;
			}
			
			inline static Value* SizeOf(const Node<Type>& type) {
				Value* value = new Value(SIZEOF);
				value->sizeOf.type = type;
				return value;
			}
			
			inline static Value* Ternary(Node<Value> condition, Node<Value> ifTrue, Node<Value> ifFalse) {
				Value* value = new Value(TERNARY);
				value->ternary.condition = condition;
				value->ternary.ifTrue = ifTrue;
				value->ternary.ifFalse = ifFalse;
				return value;
			}
			
			inline static Value* Cast(CastKind castKind, Node<Type> sourceType, Node<Type> targetType, Node<Value> operand) {
				Value* value = new Value(CAST);
				value->cast.castKind = castKind;
				value->cast.sourceType = sourceType;
				value->cast.targetType = targetType;
				value->cast.value = operand;
				return value;
			}
			
			inline static Value* Lval(const Node<Type>& targetType, const Node<Value>& operand) {
				Value* value = new Value(LVAL);
				value->makeLval.targetType = targetType;
				value->makeLval.value = operand;
				return value;
			}
			
			inline static Value* Ref(const Node<Type>& targetType, const Node<Value>& operand) {
				Value* value = new Value(REF);
				value->makeRef.targetType = targetType;
				value->makeRef.value = operand;
				return value;
			}
			
			inline static Value* InternalConstruct(const Node<ValueList>& parameters) {
				Value* value = new Value(INTERNALCONSTRUCT);
				value->internalConstruct.parameters = parameters;
				return value;
			}
			
			inline static Value* MemberAccess(Node<Value> object, const std::string& memberName) {
				Value* value = new Value(MEMBERACCESS);
				value->memberAccess.object = object;
				value->memberAccess.memberName = memberName;
				return value;
			}
			
			inline static Value* FunctionCall(Node<Value> functionValue, const Node<ValueList>& parameters) {
				Value* value = new Value(FUNCTIONCALL);
				value->functionCall.functionValue = functionValue;
				value->functionCall.parameters = parameters;
				return value;
			}
		};
		
	}
	
}

#endif
