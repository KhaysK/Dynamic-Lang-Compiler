### `NoNameLang` - dynamic interpreted language designed for Compilers course in Innopolis University

#### Language features

- Dynamic type assignment, object type can be changed during execution

- The language is **interpreted**

- Objects can be mutable (`var`) and literal (`const`)

- Primitive types: `int`, `real`,  `bool`, `string`

- User-defined types: `array`, `tuple`, `func`

- Language supports **implicit** type conversion

  

#### Here are some syntax snippets:

1. Variable declaration

   ```nnlang
   # btw, this is comment ;)
   
   # create mutable variable
   var x = 1.23;
   
   # reassign mutable variable to another type
   x = "Hello"; # OK
   
   # assign multiple variables in one line
   var a = 30, b = false;
   
   # can create variable via expression
   var g = 10, h = 20;
   var j = g * h / 2;
   
   # initialize empty variable (it has special type `null`)
   var c; # c is null
   
   # create literal variable 
   const y = "This is string";
   
   # can check type of variable
   var isYString = (y is string); # isYString = true
   
   # can check for null
   var isYNull = (y is null); # isYNull = false
   ```

2.  I/O

   ```nnlang
   # we can read variables from standard input
   var a = readInt,
   	b = readRead,
   	c = readString; # readString reads the whole line
   
   # we can print variable (or expression)
   print a;
   print a + b; # here implicit conversion to `real` happens
   print "I can concatenate strings! look: " + c;
   ```

3. `if` statement

   ```nnlang
   var a = readInt, b = readInt, c = readReal;
   
   # simple if statement
   # available relations: <, >, <=, >=, ==, !=
   if a <= b
   then
   	print "a is greated than b"
   else
   	print "a is less than b"
   end
   	
   # combined if statement
   # available operators: and, or, xor
   if a > b and a > c
   then
   	print "a is greated than b and c"
   end
   ```

   