# Documentation

## Functions

- In order to keep your code maintainable and readable, you'll be asked to document every single function of all your source files.


### How to document functions

- To document a function, you simply need to insert a comment block above it.

- Instead of a regular C multiline comment, the comment block must begin with the following line:

	```
	/**
	```

	with two stars.  
	Then, each line of the block must start with a star, followed by a space:

	```
	 * 
	```

	The block must end exactly like a C multiline comment, with a multiline comment closer:

	```
	 */
	```

- The format of a documentation block is the following one:

	```C
	/**
	 * function_name - Short description, single line
	 * @parameterx: Description of parameter x
	 * a blank line			(optional)
	 * Description: Longer description of the function		(if needed)
	 * Return: Description of the returned value
	 */
	```

- Example:
	```C
	/**
	 * op_add - Makes the sum of two numbers
	 * @arg1: First operand
	 * @arg2: Second operand
	 *
	 * Return: The sum of the two parameters
	 */
	int op_add(int arg1, int arg2)
	{
		return (arg1 + arg2);
	}
	```

- **Note:** The return statement could be removed if the function doesn't return anything (void).

## Data structures

- Besides functions you can also write documentation for structs, unions, enums and typedefs.
Instead of the function name you must write the name of the declaration.
Example:
	```C
	/**
	 * struct my_struct - Short description
	 * @a: First member
	 * @b: Second member
	 * @c: Third member
	 *
	 * Description: Longer description
	 */
	struct my_struct
	{
		int a;
		int b;
		int c;
	};
	```


# Coding style


## Indentation

- Use `tabs` to indent your code.

- **Don't** put multiple statements on a single line:
	```C
		if (condition) do_this;
		do_something_everytime;
	```
- **Don't** put multiple assignments on a single line either.

#### Switch Statement

- The preferred way to ease multiple indentation levels in a switch statement is to align the `switch` and its subordinate `case` labels in the same column instead of `double-indenting` the `case` labels.  
**E.g.:**

	```C
	int sample_func(char suffix)
	{
		int var;

		var = 0;
		switch (suffix)
		{
		case 'G':
		case 'g':
			var = 30;
			break;
		case 'M':
		case 'm':
			var = 20;
			break;
		case 'K':
		case 'k':
			var = 10;
		default:
			break;
		}
		return (var);
	}
	```

## Breaking-long-lines

- The limit on the length of lines is **80 columns (chars)** and this is a strongly preferred limit.
- unless exceeding 80 columns significantly increases readability and does not hide information, **don't** break this rule.

## Placing-braces

### The Rule

- The preferred way, is to put both the opening and the closing braces first, thusly:
	```C
		if (x == 1)
		{
			some_code = here;
		}
	```
- This applies to almost all non-function statement blocks: `if, switch, for, while`.  
**E.g.:**
	```C
		switch (action)
		{
		case CASE_ADD:
			return ("add");
		case CASE_REMOVE:
			return ("remove");
		case CASE_CHANGE:
			return ("change");
		default:
			return (NULL);
		}
	```

	and

	```C
		if (x == y)
		{
			...
		}
		else if (x > y)
		{
			...
		}
		else
		{
			....
		}
	```

- This applies to functions too: they have the opening brace at the beginning of the next line, thus:

	```C
		int function(int x)
		{
			body of function
		}
	```

### Exceptions

- Note that the closing brace is empty on a line of its own, **except** in the cases where it is followed by a continuation of the same statement, ie a `while` in a `do-statement`, like this:

	```C
		do {
			body of do-loop
		} while (condition);
	```

- Do not unnecessarily use braces where a single statement will do.

	```C
		if (condition)
			action();
	```

	and

	```C
		if (condition)
			do_this();
		else
			do_that();
	```

	This does not apply if only one branch of a conditional statement is a single statement; in the latter case use braces in both branches:

	```C
		if (condition)
		{
			do_this();
			do_that();
		}
		else
		{
			otherwise();
		}
	```

## Placing-spaces

- Use a space after (most) keywords.  
The notable exceptions are `sizeof`, `typeof`, `alignof`, and `__attribute__`, which look somewhat like functions.

	Keyword|Space After|Example
	---:|:---:|---
	`if`|Yes|`if (condition)`
	`else if`|Yes|`else if (condition)`
	`switch`|Yes|`switch (variable)`
	`case`|Yes|`case value:`
	`for`|Yes|`for (i = 0; i < 10; ++i)`
	`while`|Yes|`while (condition)`
	`return`|Yes|`return (1);`
	`sizeof`|No|`sizeof(struct file)`
	`typeof`|No|`typeof(variable)`
	`alignof`|No|`alignof(variable)`
	`__attribute__`|No|`__attribute__((unused))`

- Do not add spaces around (inside) parenthesized expressions.  
This example is **bad**:
	```C
		s = sizeof( struct file );
	```

- When dealing with pointers, the preferred use of `*` is adjacent to the data name or function name. 
**Examples:**
	```C
		char *str;
		unsigned int sample(char *ptr, char **retptr);
		char *match_strdup(substring_t *s);
	```

## Naming-conventions

### Variables

- Use **snake case** with local variables.
	```C
	int age;
	char *first_name;
	```

- Global variables should be rare, but if you use one, prefix it with g_ to easily distinguish it from local variables.


### File names

- Filenames should be all lowercase with words separated by underscores (_).
- In general, make your filenames very specific. For example, use http_server_logs.h rather than logs.h .

### Structs & typedef

- Structs names should end with `_s`, while type names should end with `_t`.
	```C
	typedef struct foo_s {
 		char *name;
 	} foo_t;
	```

### Functions

- Functions follow rules similar to variable names. They are all lowercase, and separated by underscores.
	```C
	int foo_measure_string(char *);
	```

## Functions

- Functions should be short and sweet, and do just one thing.
They must fit on **40** lines, and do one thing and do that well.

## Comments

- **Don't over-comment**
- **NEVER** try to explain **HOW** your code works in a comment: it's much better to write the code so that the working is obvious.
- Try to avoid putting comments inside functions, rather write a good description for the function.
- You can make small comments to note or warn about something particularly clever (or ugly), but try to avoid excess.

- Use this format when writing comments.
	```C
	/* Use this */
	```

- The preferred style for long (multi-line) comments is:
	```C
		/**
		 * This is the preferred style for multi-line
		 * comments in C source code.
		 * Please use it consistently.
		 *
		 * Description:  A column of asterisks on the left side,
		 * with beginning and ending almost-blank lines.
		 */
	```

## Macros-and-Enums

- Names of `macros` defining constants and labels in `enums` are capitalized.

	```C
	#define CONSTANT 0x12345
	```

	and

	```C
	enum sample
	{
		FIRST = 1,
		SECOND,
		THIRD
	};
	```

- Enums are preferred when defining several related constants.


## Header-Files

### Function prototypes

- All your function prototypes must be declared in header files.

	```C
	/* this prototype has to be declared in a header file */
	void sample_func(int);
	```

### Structs, Enum, Unions definitions

- All your structs, enums and union must be defined in header files.
  
### Typedefs

- All your typedefs must be defined in header files.
  
### Include-guards

- To prevent double inclusion, use include guards.
- Macros used with include guards must be capitalized, and also start and end with `_`.
- Example for a file named `sample_header.h`:

	```C
	#ifndef _SAMPLE_HEADER_H_
	#define _SAMPLE_HEADER_H_

	/*
	 * Structs, enums and unions definitions
	 * Typedefs
	 * Function prototypes
	 */

	#endif /* _SAMPLE_HEADER_H_ */
	```