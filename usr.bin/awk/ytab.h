
/* $Id: ytab.h,v 1.2 1998/04/07 16:14:20 tribby Exp $ */

typedef union  {
	Node	*p;
	Cell	*cp;
	int	i;
	char	*s;
} YYSTYPE;
extern YYSTYPE yylval;
# define FIRSTTOKEN 257
# define PROGRAM 258
# define PASTAT 259
# define PASTAT2 260
# define XBEGIN 261
# define XEND 262
# define NL 263
# define ARRAY 264
# define MATCH 265
# define NOTMATCH 266
# define MATCHOP 267
# define FINAL 268
# define DOT 269
# define ALL 270
# define CCL 271
# define NCCL 272
# define CHAR 273
# define OR 274
# define STAR 275
# define QUEST 276
# define PLUS 277
# define AND 278
# define BOR 279
# define APPEND 280
# define EQ 281
# define GE 282
# define GT 283
# define LE 284
# define LT 285
# define NE 286
# define IN 287
# define ARG 288
# define BLTIN 289
# define BREAK 290
# define CLOSE 291
# define CONTINUE 292
# define DELETE 293
# define DO 294
# define EXIT 295
# define FOR 296
# define FUNC 297
# define SUB 298
# define GSUB 299
# define IF 300
# define INDEX 301
# define LSUBSTR 302
# define MATCHFCN 303
# define NEXT 304
# define NEXTFILE 305
# define ADD 306
# define MINUS 307
# define MULT 308
# define DIVIDE 309
# define MOD 310
# define ASSIGN 311
# define ASGNOP 312
# define ADDEQ 313
# define SUBEQ 314
# define MULTEQ 315
# define DIVEQ 316
# define MODEQ 317
# define POWEQ 318
# define PRINT 319
# define PRINTF 320
# define SPRINTF 321
# define ELSE 322
# define INTEST 323
# define CONDEXPR 324
# define POSTINCR 325
# define PREINCR 326
# define POSTDECR 327
# define PREDECR 328
# define VAR 329
# define IVAR 330
# define VARNF 331
# define CALL 332
# define NUMBER 333
# define STRING 334
# define REGEXPR 335
# define GETLINE 336
# define RETURN 337
# define SPLIT 338
# define SUBSTR 339
# define WHILE 340
# define CAT 341
# define NOT 342
# define UMINUS 343
# define POWER 344
# define DECR 345
# define INCR 346
# define INDIRECT 347
# define LASTTOKEN 348
