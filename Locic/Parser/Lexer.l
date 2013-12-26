%{

#include <string>
#include <Locic/AST.hpp>
#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/LocationInfo.hpp>
#include <Locic/Parser/Token.hpp>

#define YY_EXTRA_TYPE Locic::Parser::Context *
#define YYSTYPE Locic::Parser::Token
#define YYLTYPE Locic::Parser::LocationInfo
#define YY_USER_ACTION yylloc->first_line = yylloc->last_line = yylineno; \
	yylloc->first_column = yyextra->columnPosition(); yylloc->last_column = yyextra->columnPosition() + yyleng; \
	yyextra->advanceColumn(yyleng);

%}

%option prefix="Locic_Parser_GeneratedLexer_"
%option reentrant
%option noyywrap
%option noinput
%option nounput
%option yylineno
%option bison-bridge bison-locations

%x COMMENT_STATE
%x STRING_STATE

%%

"/*"                          { BEGIN(COMMENT_STATE); }
<COMMENT_STATE>[^*\n]*        { }
<COMMENT_STATE>"*"+[^*/\n]*   { }
<COMMENT_STATE>\n             { yyextra->resetColumn(); }
<COMMENT_STATE>"*"+"/"        { BEGIN(INITIAL); }

"\""                          { yyextra->resetStringConstant(); BEGIN(STRING_STATE); }
<STRING_STATE>"\\\""          { yyextra->appendStringConstant("\""); }
<STRING_STATE>"\\\\"          { yyextra->appendStringConstant("\\"); }
<STRING_STATE>"\\n"           { yyextra->appendStringConstant("\n"); }
<STRING_STATE>"\\t"           { yyextra->appendStringConstant("\t"); }
<STRING_STATE>"\"C"           { yylval->lexer_constant = Locic::Constant::String(Locic::Constant::CSTRING, yyextra->getStringConstant());
				BEGIN(INITIAL);
				return CONSTANT; }
<STRING_STATE>"\""            { yylval->lexer_constant = Locic::Constant::String(Locic::Constant::LOCISTRING, yyextra->getStringConstant());
				BEGIN(INITIAL);
				return CONSTANT; }
<STRING_STATE>.               { yyextra->appendStringConstant(yytext); }

if                      { return IF; }
else                    { return ELSE; }
while                   { return WHILE; }
for                     { return FOR; }
return                  { return RETURN; }
switch                  { return SWITCH; }
case                    { return CASE; }
default                 { return DEFAULT; }
continue                { return CONTINUE; }
break                   { return BREAK; }

exception               { return EXCEPTION; }
throw                   { return THROW; }
try                     { return TRY; }
catch                   { return CATCH; }

template                { return TEMPLATE; }
typename                { return TYPENAME; }
virtual                 { return VIRTUAL; }

struct                  { return STRUCT; }
enum                    { return ENUM; }
union                   { return UNION; }
class                   { return CLASS; }
interface               { return INTERFACE; }
namespace               { return NAMESPACE; }
datatype                { return DATATYPE; }
__primitive             { return PRIMITIVE; }

auto                    { return AUTO; }
const                   { return CONST; }

using                   { return USING; }
static                  { return STATIC; }
import                  { return IMPORT; }
export                  { return EXPORT; }

new                     { return NEW; }
delete                  { return DELETE; }
move                    { return MOVE; }
lval                    { return LVAL; }
sizeof                  { return SIZEOF; }

const_cast              { return CONST_CAST; }
static_cast             { return STATIC_CAST; }
reinterpret_cast        { return REINTERPRET_CAST; }
dynamic_cast            { return DYNAMIC_CAST; }

typeof                  { return TYPEOF; }
typeid                  { return TYPEID; }

void                    { return VOIDNAME; }

null                    { yylval->lexer_constant = Locic::Constant::Null(); return CONSTANT; }
true                    { yylval->lexer_constant = Locic::Constant::True(); return CONSTANT; }
false                   { yylval->lexer_constant = Locic::Constant::False(); return CONSTANT; }
[0-9]+                  { yylval->lexer_constant = Locic::Constant::SignedInt(Locic::Constant::INT, atol(yytext)); return CONSTANT; }
[0-9]+l                 { yylval->lexer_constant = Locic::Constant::SignedInt(Locic::Constant::LONG, atol(yytext)); return CONSTANT; }
[0-9]+u                 { yylval->lexer_constant = Locic::Constant::UnsignedInt(Locic::Constant::UINT, atol(yytext)); return CONSTANT; }
[0-9]+ul                { yylval->lexer_constant = Locic::Constant::UnsignedInt(Locic::Constant::ULONG, atol(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+f         { yylval->lexer_constant = Locic::Constant::Float(Locic::Constant::FLOAT, atof(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+          { yylval->lexer_constant = Locic::Constant::Float(Locic::Constant::DOUBLE, atof(yytext)); return CONSTANT; }
[0-9]+\.[0-9]+L         { yylval->lexer_constant = Locic::Constant::Float(Locic::Constant::LONGDOUBLE, atof(yytext)); return CONSTANT; }

"_"                     { return UNDERSCORE; }

[A-Za-z_][A-Za-z0-9_]*  { yylval->lexer_str = new std::string(yytext); return NAME; }

"=="                    { return ISEQUAL; }
"!="                    { return NOTEQUAL; }
">="                    { return GREATEROREQUAL; }
"<="                    { return LESSOREQUAL; }
"->"                    { return PTRACCESS; }

"+="                    { return ADDEQUAL; }
"-="                    { return SUBEQUAL; }
"*="                    { return MULEQUAL; }
"/="                    { return DIVEQUAL; }
"%="                    { return PERCENTEQUAL; }

\/\/(.*?)\n             { }

"%"                     { return PERCENT; }
"="                     { return SETEQUAL; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return FORWARDSLASH; }
"!"                     { return EXCLAIMMARK; }
"&&"                    { return DOUBLE_AMPERSAND; }
"&"                     { return AMPERSAND; }
"||"                    { return DOUBLE_VERTICAL_BAR; }
"|"                     { return VERTICAL_BAR; }
"?"                     { return QUESTIONMARK; }
"@"                     { return AT; }
","                     { return COMMA; }
":"                     { return COLON; }
";"                     { return SEMICOLON; }
"("                     { return LROUNDBRACKET; }
")"                     { return RROUNDBRACKET; }
"{"                     { return LCURLYBRACKET; }
"}"                     { return RCURLYBRACKET; }
"["                     { return LSQUAREBRACKET; }
"]"                     { return RSQUAREBRACKET; }
"."                     { return DOT; }
"<<"                    { return DOUBLE_LTRIBRACKET; }
"<"                     { return LTRIBRACKET; }
">"                     { return RTRIBRACKET; }
"~"                     { return TILDA; }
\n                      { yyextra->resetColumn(); }
[ \t\r]+                { }
.                       { return ERROR; }

%%

namespace Locic{
	
	namespace Parser{
		
		void * LexAlloc(FILE * file, Context * context){
			void * lexer;
			Locic_Parser_GeneratedLexer_lex_init(&lexer);
			Locic_Parser_GeneratedLexer_set_in(file, lexer);
			Locic_Parser_GeneratedLexer_set_extra(context, lexer);
			return lexer;
		}
		
		int LexGetToken(Token* token, LocationInfo* locationInfo, void * lexer){
			return Locic_Parser_GeneratedLexer_lex(token, locationInfo, lexer);
		}
		
		void LexFree(void * lexer){
			Locic_Parser_GeneratedLexer_lex_destroy(lexer);
		}
		
	}
	
}

