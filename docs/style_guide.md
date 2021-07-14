# Style Guide

Common code style and proper formatting are important for making a codebase that
several people will be working on simultaneously. In general:

> When in doubt about code style, look at how existing code is styled.

However, there are several style patterns that need to be explicitly explained.

# Licensing

TODO...

# Language Choice

When possible, use C99, and not C++. C is a far simpler language than C++, and
so this is a key part of complexity management. However... enforcing C
globally can be counter-productive since a lot of useful libraries only have
C++ APIs, so there need to be some exceptions. So the rule is:

> When possible, use C99 in your header and source files. C++ code is alright to
> use in source files if C would be inconvenient/impossible to work with in
> context. However, all C++ definitions must be kept out of public headers.

Some cases of C++ being more appropriate than C may be:
- a library being used in a source file only has a C++ interface
- a C++ templated container type is easier to work with than C

# Whitespace and Formatting

Try to stick with the [GNU C style guide](https://www.gnu.org/prep/standards/html_node/Writing-C.html).
There is a `.clang-format` file in the root path of the repository that simply
inherits the GNU C style guide, and can be used by whatever plugin you use in
your IDE to `clang-format`.

# Boilerplate

## Javadoc-Style Comments

Please place Javadoc-style comments before every public typedef and function.
They don't necessarily have to provide a full description of the code being
described, but they do need to provide a place to insert that information later.

Note the double-asterisks, @ prefixes, and single asterisk on every line.

```c
/** @typedef blah_t
 * @blah blahblahblahblah.
 */
```

## File Docs

Put a properly formatted comment at the top of each new file (headers/sources):

```c
/** @file my_file.h
 * @license TODO...
 */
```

## Include Guards

`#pragma once` should be the first non-comment contents of a header file.
It's supported on all major C compilers, and so it's alright to use even if it's
not technically part of the C standard.

```c
#pragma once
```

# Authoring

## Function Naming

TODO...

## Return Codes

Use `int` return values for functions that can fail. Zero represents success,
non-zero represents an error. Because non-zero evaluates to `true` in C `if`
statements, do not use the `!` operator before a function call if you want to
catch error codes.

Incorrect:

```c
if (!my_function (params))
  {
    LOG_ERR ("a false-positive error occurred!");
    return 0;
  }

LOG_DBG ("an error occurred but we didn't catch it...");
return 1;
```

Correct:

```c
if (my_function (params))
  {
    LOG_ERR ("an error occurred!");
    return 1;
  }

LOG_DBG ("no error occurred");
return 0;
```

# Creating New Objects

Although Mondradiko is mainly written in C and not C++, object-oriented design
patterns are still appropriate in a lot of cases. This section details how to
create a new object in C that plays nice with the coding style.

## Opaque Types

Instead of needing to define the contents of an object in the header file, like
how C++ classes are defined, we can instead define "opaque" types and pass
pointers around to them.

These are defined in C with `typedef`, in the object's header:

```c
/** @typedef example_object_t
 * A Mondradiko-friendly example object.
 */
typedef struct example_object_s example_object_t;
```

Note the use of `_s` and `_t` postfixes to distinguish the opaque object
structure from the object handle type.

The contents of the object structure are defined in a source file:

```c
struct example_object_s
{
  int member1;
  float member2;
  example_object_t *another_object;
};
```

## Object Methods

"Methods" are just a type of function that operate on an object, and have a view
into that object's members. C object methods are plain functions.

See [Javadoc-style Comments](#javadoc-style-comments).

C doesn't have namespaces or scoped methods like C++, so method names need to be
prepended with the name of the object.

```c
/** @function example_object_method
 */
void example_object_method (example_object_t *);
```

## Object Lifetime

Object lifetimes are managed with `*_new` and `*_delete` methods. Because opaque
C structures have no public definitions, they can't be created as simple local
variables, so the object needs to be allocated and freed via public methods.

### Object Constructor

See [Return Codes](#return-codes).

The pointer-to-a-pointer parameter is used to initialize a pointer to the opaque
object.

Header file declaration:

```c
/** @function example_object_new
 */
int example_object_new (example_object_t **);
```

Usage:

```c
example_object_t *my_object;
if (example_object_new (&my_object))
  {
    LOG_ERR ("failed to create example object");
    return 1;
  }
```

### Object Destructor

TODO...

Header file declaration:

```c
/** @function example_object_delete
 */
void example_object_delete (example_object_t *);
```
