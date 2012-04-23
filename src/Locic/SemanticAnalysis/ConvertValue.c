#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/StringMap.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertType.h>
#include <Locic/SemanticAnalysis/ConvertValue.h>

SEM_Value * Locic_SemanticAnalysis_ConvertGeneralBinaryOperator(SEM_BinaryType opType, SEM_Type * type, SEM_Value * leftOperand, SEM_Value * rightOperand){
	if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
		SEM_BasicTypeEnum leftBasicType, rightBasicType;
		leftBasicType = leftOperand->type->basicType.typeEnum;
		rightBasicType = rightOperand->type->basicType.typeEnum;
		
		if(leftBasicType == rightBasicType){
			if(leftBasicType == SEM_TYPE_BASIC_BOOL){
				return SEM_MakeBinary(opType, SEM_OP_BOOL, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM_TYPE_BASIC_INT){
				return SEM_MakeBinary(opType, SEM_OP_INT, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
				return SEM_MakeBinary(opType, SEM_OP_FLOAT, leftOperand, rightOperand, type);
			}
		}
	}
	printf("Semantic Analysis Error: Comparison between non-identical types.\n");
	return NULL;
}

SEM_Value * Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BinaryType opType, SEM_Type * type, SEM_Value * leftOperand, SEM_Value * rightOperand){
	if(leftOperand->type->typeEnum == SEM_TYPE_BASIC && rightOperand->type->typeEnum == SEM_TYPE_BASIC){
		SEM_BasicTypeEnum leftBasicType, rightBasicType;
		leftBasicType = leftOperand->type->basicType.typeEnum;
		rightBasicType = rightOperand->type->basicType.typeEnum;
		
		if(leftBasicType == rightBasicType){
			if(leftBasicType == SEM_TYPE_BASIC_INT){
				return SEM_MakeBinary(opType, SEM_OP_INT, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM_TYPE_BASIC_FLOAT){
				return SEM_MakeBinary(opType, SEM_OP_FLOAT, leftOperand, rightOperand, type);
			}
		}
	}
	printf("Semantic Analysis Error: Comparison between non-numeric or non-identical types.\n");
	return NULL;
}

SEM_Value * Locic_SemanticAnalysis_ConvertValue(Locic_SemanticContext * context, AST_Value * value){
	if(value == NULL){
		printf("Internal compiler error: Cannot convert NULL AST_Value.\n");
		return NULL;
	}

	switch(value->type){
		case AST_VALUE_CONSTANT:
		{
			switch(value->constant.type){
				case AST_CONSTANT_BOOL:
					return SEM_MakeBoolConstant(value->constant.boolConstant);
				case AST_CONSTANT_INT:
					return SEM_MakeIntConstant(value->constant.intConstant);
				case AST_CONSTANT_FLOAT:
					return SEM_MakeFloatConstant(value->constant.floatConstant);
				case AST_CONSTANT_NULL:
					return SEM_MakeNullConstant();
				default:
					printf("Internal Compiler Error: Unknown constant type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_VAR:
		{
			AST_Var * synVar = value->varValue.var;
			switch(synVar->type){
				case AST_VAR_LOCAL:
				{
					SEM_Var * semVar = Locic_SemanticContext_FindLocalVar(context, synVar->localVar.name);
					if(semVar != NULL){
						return SEM_MakeVarValue(semVar);
					}
					
					// Not a variable - try looking for functions.
					SEM_FunctionDecl * decl = Locic_StringMap_Find(context->functionDeclarations, synVar->localVar.name);
					
					if(decl != NULL){
						Locic_StringMap_Insert(context->module->functionDeclarations, synVar->localVar.name, decl);
						return SEM_MakeFunctionRef(decl, decl->type);
					}
					
					printf("Semantic Analysis Error: Local variable '%s' was not found\n", synVar->localVar.name);
					return NULL; 
				}
				case AST_VAR_THIS:
				{
					printf("Semantic Analysis Error: Member variables not implemented.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown AST_Var type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_UNARY:
		{
			SEM_Value * operand = Locic_SemanticAnalysis_ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST_UNARY_PLUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_PLUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_MINUS:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						SEM_BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM_TYPE_BASIC_INT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_INT, operand, typeCopy);
						}else if(basicType == SEM_TYPE_BASIC_FLOAT){
							return SEM_MakeUnary(SEM_UNARY_MINUS, SEM_OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type.\n");
					return NULL;
				}
				case AST_UNARY_ADDRESSOF:
				{
					if(operand->type->isLValue == SEM_TYPE_LVALUE){
						return SEM_MakeUnary(SEM_UNARY_ADDRESSOF, SEM_OP_PTR, operand, SEM_MakePtrType(SEM_TYPE_MUTABLE, SEM_TYPE_RVALUE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
					return NULL;
				}
				case AST_UNARY_DEREF:
				{
					if(operand->type->typeEnum == SEM_TYPE_PTR){
						return SEM_MakeUnary(SEM_UNARY_DEREF, SEM_OP_PTR, operand, operand->type->ptrType.ptrType);
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
					return NULL;
				}
				case AST_UNARY_NOT:
				{
					if(operand->type->typeEnum == SEM_TYPE_BASIC){
						SEM_Type * typeCopy = SEM_CopyType(operand->type);
						typeCopy->isMutable = SEM_TYPE_MUTABLE;
						typeCopy->isLValue = SEM_TYPE_RVALUE;
						if(typeCopy->basicType.typeEnum == SEM_TYPE_BASIC_BOOL){
							return SEM_MakeUnary(SEM_UNARY_NOT, SEM_OP_BOOL, operand, typeCopy);
						}
					}
					
					printf("Semantic Analysis Error: Unary NOT on non-bool type.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown unary value type enum.\n");
					return NULL;
			}
		}
		case AST_VALUE_BINARY:
		{
			SEM_Value * leftOperand, * rightOperand;
			leftOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.left);
			if(leftOperand == NULL) return NULL;
			
			rightOperand = Locic_SemanticAnalysis_ConvertValue(context, value->binary.right);
			if(rightOperand == NULL) return NULL;
			
			switch(value->binary.type){
				case AST_BINARY_ADD:
				{
					SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
					typeCopy->isLValue = SEM_TYPE_RVALUE;
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_ADD, typeCopy, leftOperand, rightOperand);
				}
				case AST_BINARY_SUBTRACT:
				{
					SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
					typeCopy->isLValue = SEM_TYPE_RVALUE;
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_SUBTRACT, typeCopy, leftOperand, rightOperand);
				}
				case AST_BINARY_MULTIPLY:
				{
					SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
					typeCopy->isLValue = SEM_TYPE_RVALUE;
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_MULTIPLY, typeCopy, leftOperand, rightOperand);
				}
				case AST_BINARY_DIVIDE:
				{
					SEM_Type * typeCopy = SEM_CopyType(leftOperand->type);
					typeCopy->isLValue = SEM_TYPE_RVALUE;
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_DIVIDE, typeCopy, leftOperand, rightOperand);
				}
				case AST_BINARY_ISEQUAL:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertGeneralBinaryOperator(SEM_BINARY_ISEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST_BINARY_NOTEQUAL:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertGeneralBinaryOperator(SEM_BINARY_NOTEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST_BINARY_LESSTHAN:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_LESSTHAN, boolType, leftOperand, rightOperand);
				}
				case AST_BINARY_GREATERTHAN:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_GREATERTHAN, boolType, leftOperand, rightOperand);
				}
				case AST_BINARY_GREATEROREQUAL:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_GREATEROREQUAL, boolType, leftOperand, rightOperand);
				}
				case AST_BINARY_LESSOREQUAL:
				{
					SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
					return Locic_SemanticAnalysis_ConvertNumericBinaryOperator(SEM_BINARY_LESSOREQUAL, boolType, leftOperand, rightOperand);
				}
				default:
					printf("Internal Compiler Error: Unknown binary value type enum.\n");
					return NULL;
			}
			printf("Internal Compiler Error: Unimplemented binary operator.\n");
			return NULL;
		}
		case AST_VALUE_TERNARY:
		{
			SEM_Type * boolType = SEM_MakeBasicType(SEM_TYPE_CONST, SEM_TYPE_RVALUE, SEM_TYPE_BASIC_BOOL);
			
			SEM_Value * cond = Locic_SemanticAnalysis_ConvertValue(context, value->ternary.condition);
			
			SEM_Value * boolValue = Locic_SemanticAnalysis_CastValueToType(context, cond, boolType);
			
			if(boolValue == NULL){
				printf("Semantic Analysis Error: Cannot cast or copy condition type (");
				SEM_PrintType(cond->type);
				printf(") to bool type in ternary operator.\n");
				return NULL;
			}
			
			SEM_Value * ifTrue = Locic_SemanticAnalysis_ConvertValue(context, value->ternary.ifTrue);
			SEM_Value * ifFalse = Locic_SemanticAnalysis_ConvertValue(context, value->ternary.ifFalse);
			
			SEM_Type * ifTrueType = SEM_CopyType(ifTrue->type);
			SEM_Type * ifFalseType = SEM_CopyType(ifFalse->type);
			
			// Can only result in an lvalue if both possible results are lvalues.
			if(ifTrueType->isLValue == SEM_TYPE_RVALUE || ifFalseType->isLValue == SEM_TYPE_RVALUE){
				ifTrueType->isLValue = SEM_TYPE_RVALUE;
				ifFalseType->isLValue = SEM_TYPE_RVALUE;
			}
			
			SEM_Value * castIfTrue = Locic_SemanticAnalysis_CastValueToType(context, ifTrue, ifFalseType);
			if(castIfTrue != NULL){
				return SEM_MakeTernary(boolValue, castIfTrue, ifFalse, ifFalseType);
			}
			
			SEM_Value * castIfFalse = Locic_SemanticAnalysis_CastValueToType(context, ifFalse, ifTrueType);
			if(castIfFalse != NULL){
				return SEM_MakeTernary(boolValue, ifTrue, castIfFalse, ifTrueType);
			}
			
			printf("Semantic Analysis Error: Can't cast result expressions to matching type in ternary operator.\n");
			return NULL;
		}
		case AST_VALUE_CAST:
		{
			AST_Cast * cast = &(value->cast);
			SEM_Type * type = Locic_SemanticAnalysis_ConvertType(context, cast->type, SEM_TYPE_RVALUE);
			SEM_Value * val = Locic_SemanticAnalysis_ConvertValue(context, cast->value);
			
			if(type == NULL || val == NULL){
				return NULL;
			}
			
			if(Locic_SemanticAnalysis_CanDoExplicitCast(context, val->type, type) == 0){
				printf("Semantic Analysis Error: Can't perform explicit cast.\n");
				return NULL;
			}
			
			return SEM_MakeCast(type, val);
		}
		case AST_VALUE_CONSTRUCT:
		{
			printf("Internal Compiler Error: Unimplemented constructor call.\n");
			return NULL;
		}
		case AST_VALUE_MEMBERACCESS:
		{
			printf("Internal Compiler Error: Unimplemented member access.\n");
			return NULL;
		}
		case AST_VALUE_FUNCTIONCALL:
		{
			SEM_Value * functionValue = Locic_SemanticAnalysis_ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL){
				return NULL;
			}
			
			if(functionValue->type->typeEnum != SEM_TYPE_FUNC){
				printf("Semantic Analysis Error: Can't call non-function type.\n");
				return NULL;
			}
			
			Locic_List * typeList = functionValue->type->funcType.parameterTypes;
			Locic_List * synValueList = value->functionCall.parameters;
			
			if(Locic_List_Size(typeList) != Locic_List_Size(synValueList)){
				printf("Semantic Analysis Error: Function called with %lu number of parameters; expected %lu.\n", Locic_List_Size(synValueList), Locic_List_Size(typeList));
				return NULL;
			}
			
			Locic_List * semValueList = Locic_List_Alloc();
			
			Locic_ListElement * typeIt = Locic_List_Begin(typeList);
			Locic_ListElement * valueIt = Locic_List_Begin(synValueList);
			
			while(valueIt != Locic_List_End(synValueList)){
				SEM_Value * param = Locic_SemanticAnalysis_CastValueToType(context, Locic_SemanticAnalysis_ConvertValue(context, valueIt->data), typeIt->data);
				
				if(param == NULL){
					return NULL;
				}
				
				Locic_List_Append(semValueList, param);
				
				typeIt = typeIt->next;
				valueIt = valueIt->next;
			}
			
			return SEM_MakeFunctionCall(functionValue, semValueList, functionValue->type->funcType.returnType);
		}
		default:
			printf("Internal Compiler Error: Unknown AST_Value type enum.\n");
			return NULL;
	}
}

