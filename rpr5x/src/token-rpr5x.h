
/* Tokens.  */
#define MOVE 258
#define SHIFT 259
#define VAR 260
#define JUMP 261
#define FOR 262
#define STEP 263
#define TO 264
#define DO 265
#define COPEN 266
#define CCLOSE 267
#define SIN 268
#define COS 269
#define SQRT 270
#define FLOAT 271
#define ID 272
#define INTEGER 273
#define SEMICOLON 274
#define PLUS 275
#define MINUS 276
#define TIMES 277
#define DIV 278
#define OPEN 279
#define CLOSE 280
#define ASSIGN 281
#define LEFTBRACE 282
#define RIGHTBRACE 283
#define LESS 284
#define GREATER 285
#define LESSOREQUAL 286
#define GREATEROREQUAL 287
#define EQUAL 288
#define PROCEDURE 289
#define PARAM 290
#define CALL 291
#define IF 292
#define THEN 293
#define ELSE 294
#define WHILE 295
#define COMMA 296
#define ERROR 297
#define LEFTBRACKET 298
#define RIGHTBRACKET 299






typedef union YYSTYPE
{ int i; node *n; double d;}
        YYSTYPE;
extern YYSTYPE yylval;

