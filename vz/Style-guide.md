Coding Style Guidelines for lmi
===============================

Syntax Rules
============

Files
-----

This section covers organization of the source code in files.

### Don't use subdirectories

Use flat file organization, with all source files located in the same root
directory.

### Use lower case file names with underscore separators between words

All files names must use lower case ASCII letters only, with underscores to
separate multiple words if necessary.

### Use `.?pp` extensions

Use `.cpp` extension for the source file, `.hpp` extension for the headers and
`.?pp` for the other kinds of source files, e.g. currently `.tpp` is used for
template definitions which don't need to be included by all the class clients
and `.xpp` is used for the [X-Macros](https://en.wikipedia.org/wiki/X_Macro).

### Use Unix end of line characters

All text files should use Unix EOL format, i.e. `\n` (ASCII LF).

### Use maximal line length of 80

Wrap lines longer than 80 characters.

Exception: Do not wrap copyright lines, containing long list of years.


Indentation and Spaces
----------------------

### Use spaces, not TABs

The indentation unit is 4 spaces, hard TABs must only be used in the makefiles.

### Use half indent for access specifiers

Use 2 spaces indent for `public:`, `protected:` and `private:`.

### Do not insert spaces after keywords or inside parentheses

Write `if(condition)`, without any extra spaces.

### Do insert a space after comma

Write `call_function(arg1, arg2, arg3)`, with a single space after each comma.

Exception: Do not insert space in template instantiations such as
`std::pair<T,U>` or specializations to keep the type a single syntactic unit.

### Put separators on a new line in multi-line statements

When a line needs to be wrapped, indent and vertically align the separators.

For the wrapped function calls, indent the opening parentheses and align it
with the subsequent commas, e.g.:
```
call_function
    (arg1
    ,arg2
    ,arg3
    );
```

For the other expressions, indent and align the separator between the terms:
```
std::cout
    << "Hello "
    << "world"
    << "!"
    << "\n"
    ;
```

### Indent the braces of compound statements

The opening brace of compound statements (`if`, `while`, `switch`, ...) must be
on its own line and indented relatively to the statement line itself. The
closing brace must be vertically aligned with the opening brace.

Notice that this means that the compound statement body has the same
indentation level as the braces surrounding it.

Example:

```
if(condition)
    {
    do_something();
    do_something_else();
    }
```

### Use braces even for compound statement containing a single line

Always use braces for compound statements, even in the situations where the
language rules allow omitting them, e.g.
```
for(;;)
    {
    wait();
    }
```

### Use space around arithmeric operators

E.g. `(x + y) * z` instead of `(x+y)*z`.


### Headers conventions

- One blank line below `config.hpp` because it's so special.
- Comments on `#include` line begin in column 41.

In `*_test.cpp` unit tests:

    #include "pchfile.hpp"
    [blank line]
    #include "whatever_class_is_tested_here.hpp"
    [blank line]
    #include [other_headers_needed_here]

Surround the middle line with blank lines, to distinguish the header
that is to be tested from headers used by the tests. That emphasis
makes the purpose of the test clearer, and that order ensures that
the incidental headers that follow do not mask a defect in the
physical design of the tested header.


Naming Conventions
------------------

### Use `snake_case_convention` for identifiers

Classes, enums, functions and variables use the same naming convention.

Notice that many existing classes use legacy `CamelCase` naming convention
instead, they should not be renamed, but new classes must not use camel case.

### Use trailing underscore for member variables

Member variables are distinguished from local variables and function parameters
by the presence of `_` at the end.

### Don't use `a_` prefix for function arguments

Although some legacy code does use this prefix, it shouldn't be used in new
code if possible.

### Standard variable names

Use `z` for local variables whose names are unimportant (similar to `_` in some
other languages).


Other
-----

### Explicitly write fractional part of floating point numbers as 0

I.e. `1.0` instead of just `1.`.

### Declare anonymous namespaces consistently

Use the following style for anonymous namespaces:
```cpp
namespace
{
} // Unnamed namespace.

```

Semantic Rules
==============

Class Declarations
------------------

### Put `friend` declarations at the top

Also use elaborated-type-specifiers in the friend declarations:

```cpp
class X {
    friend class Y; // Do use class keyword.

public:
    ...
};
```

See 1930b4b0cf494341e3a3b571085ecf29002a2488.

### Declare mutators before accessors

Declare non-const functions before const functions in each of public and
private sections. Withing each subsection, put higher level functions before
the lower level ones.

Rationale: accessors are generally less interesting, and often trivial; they
get in the way unless they're grouped at the end.

Also, use the same order for the definitions in the implementation file, as for
the declarations.

See f6390fe69a768246deba65e5bef18a9c2f462c14.

### Use consistent names for constructor arguments, accessors and variables

Use exactly the same name in all of these contexts, e.g.:

```cpp
class xyzzy
{
  public:
    xyzzy(some_kind_of_thing const& thing)
        :thing_ {thing}
    {}

    some_kind_of_thing const& thing() {return thing_;}

  private:
    some_kind_of_thing thing_;
};
```

See ee96985b83067d9a6a9219f68fa6c0a6eabbb697.


Miscellaneous
-------------

### Initialize all non-static data members at the point of declaration

Use braced-initializer-lists in the class definition, e.g.:

```cpp
  int         a {3};
  double      b {1.7321};
  std::string c {"explicit default value"};
  std::string d {}; // when an empty string is wanted
```

Rationale: This makes it impossible to use an uninitialized field and avoids
duplication of the same values among different constructors.


Preprocessor Conditions
-----------------------

### Prefer `#if defined FOO` to `#ifdef FOO`

Similarly, use `#if !defined FOO` rather than `#ifndef Foo`.

Also write the comment after `#else` (if any) or `#endif` as `// defined FOO`
for symmetry (rather than just `// FOO` which would match `#if FOO`).


Special Member Functions
------------------------

### Use `= default` instead of `{}` to define special member functions

Avoid writing default constructors, copy constructors and assignment operators
or destructors with an empty body, just replace the body with `= default`
instead.

Rationale: This makes the code clearer, and lets the compiler optimize
better.

### `= delete` copy constructor and assignment operator in non-copyable classes

If a class can't be copied, e.g. because it contains reference members or its
destructor has side effects or even if just because it doesn't make sense to
copy objects of this type, explicitly declare the copy constructor and
assignment operator as deleted.

If it does make sense to return objects from the class from functions, define
move constructor (and, usually, move assignment operator too for symmetry) for
it to allow doing this.

### Define special member functions inline if possible

Define the special member functions inline, directly in the class declaration
whenever possible.

One common case when it is not possible to do it is for the class destructor
in one of the two following situations:

 * The destructor is pure virtual and cannot be defined at the point of
   declaration.
 * The class has smart-pointer members of forward-declared types.

Define the destructor (still using `= default`, if appropriate) in the source
file, out of line, in this case and, in the latter case, use the standard
comment explaining why is it done so:

```cpp
/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.
```


Comparisons
-----------

### Write comparison operations in number line order

I.e. always use `<` or `<=` instead of `>` or `>=`.
