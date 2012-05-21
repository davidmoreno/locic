#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Locic/AST.h>
#include <Locic/Lexer.h>
#include <Locic/LexerContext.h>
#include <Locic/List.h>
#include <Locic/Parser.h>
#include <Locic/ParserContext.h>
#include <Locic/Token.h>
#include <Locic/CodeGen/CodeGen.h>
#include <Locic/SemanticAnalysis.h>

void * Locic_ParseAlloc(void * (*allocFunc)(size_t));

void Locic_Parse(void * parser, int id, Locic_Token token, Locic_ParserContext * parserContext);

void Locic_ParseFree(void * parser, void (*freeFunc)(void *));

void Locic_ParseTrace(FILE * stream, char * zPrefix);

void * Locic_alloc(size_t size){
	return malloc(size);
}

void Locic_free(void * ptr){
	free(ptr);
}

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Locic: No files provided\n");
		return 0;
	}
	
	char * filename = argv[1];

	FILE * file = fopen(filename, "rb");
	
	if(file == 0){
		printf("Failed to open file\n");
		return 1;
	}
	
	Locic_LexerContext lexerContext;
	lexerContext.lineNumber = 0;
	
	Locic_ParserContext parserContext;
	parserContext.lineNumber = 0;
	parserContext.parseFailed = 0;
	
	AST_ModuleGroup * synModuleGroup = AST_MakeModuleGroup();
	parserContext.moduleGroup = synModuleGroup;
	parserContext.currentFileName = filename;
	
	void * lexer = Locic_LexAlloc(file, &lexerContext);
	void * parser = Locic_ParseAlloc(Locic_alloc);
	
	unsigned int numTokens = 0;
	
	//Locic_ParseTrace(stdout, "==> ");
	
	while(1){
	        int lexVal = Locic_Lex(lexer);
	        
	        parserContext.lineNumber = lexerContext.lineNumber;
	        
	        //printf("Found token at line %d\n", (int) lexerContext.lineNumber);
		
		Locic_Parse(parser, lexVal, lexerContext.token, &parserContext);
		
		if(lexVal == 0){
			break;
		}
		
		numTokens++;
	}
	
	if(parserContext.parseFailed != 1){
		printf("Performing semantic analysis...\n");

		// Try to do semantic analysis...
		SEM_ModuleGroup * semModuleGroup = Locic_SemanticAnalysis_Run(synModuleGroup);
		if(semModuleGroup != NULL){
			printf("Successfully performed semantic analysis.\n");
			
			printf("Generating code...\n");
		
			size_t moduleNameLen = strlen(argv[1]);
			char * moduleName = strcpy(malloc(moduleNameLen + 3), argv[1]);
			moduleName[moduleNameLen] = '.';
			moduleName[moduleNameLen + 1] = 'o';
			moduleName[moduleNameLen + 2] = 0;
		
			void * codeGenContext = Locic_CodeGenAlloc(moduleName);
			Locic_CodeGen(codeGenContext, Locic_List_Begin(semModuleGroup->modules)->data);
			Locic_CodeGenDump(codeGenContext);
			Locic_CodeGenFree(codeGenContext);
		}else{
			printf("Semantic Analysis failed.\n");
		}
	}else{
		printf("Parsing failed.\n");
	}
	
	printf("Used %d tokens\n", numTokens);
	
	Locic_LexFree(lexer);
	Locic_ParseFree(parser, Locic_free);
	
	fclose(file);
	
	return 0;
}
