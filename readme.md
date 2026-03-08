LANGUAGE NAME: MINIC

KEYWORDS: int, if, while
IDENTIFIER: x, count, total
NUMBER: 10, 25
OPERATORS: + - * / =
DELIMITERS: ; ( ) { }

//grammar

program → statement_list

statement_list → statement statement_list | ε

statement →
    declaration ;
  | assignment ;
  | if_statement
  | while_statement

declaration → int identifier

assignment → identifier = expression

expression → term ((+|-) term)*

term → factor ((*|/) factor)*

factor → identifier | number


//Architecture

Source Code
   ↓
Lexer (Week 2)
   ↓
Parser (Week 3)
   ↓
Semantic Analyzer (Week 4)
   ↓
Error Generator
   ↓
AI Tutor

