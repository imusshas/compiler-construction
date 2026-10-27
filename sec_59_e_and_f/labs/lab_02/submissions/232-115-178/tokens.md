
# Lab 02 - Lexical Analyzer

- Student ID: 232-115-178
- Section: 59 E

## Token Inventory

| Token Type | Lexemes / Rule |
|---|---|
| KEYWORD | int, if, else, while, return |
| IDENTIFIER | [A-Za-z_][A-Za-z0-9_]*, excluding keywords |
| INTEGER | [0-9]+ |
| ASSIGNMENT_OPERATOR | = |
| ARITHMETIC_OPERATOR | +, -, *, /, % |
| RELATIONAL_OPERATOR | ==, !=, <, <=, >, >= |
| LOGICAL_OPERATOR | &&, ||, ! |
| DELIMITER | ( ) { } [ ] ; , |
| UNKNOWN | Any other single non-whitespace character |

Whitespace, // line comments, and /* block comments */ are ignored.
Keywords are case-sensitive. The longest valid operator is matched first.
