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

2. I/O

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

4. loops

   ```nnlang
   # while loops
   var t = 0;
   while t < 10
   loop
   	print t
   	t += 1
   end
   
   # for loop
   for var i = 0; i < 10; i += 1
   loop
   	print i * i
   end
   ```

5. functions

   ```nlang
   # functions are objects too
   
   var f = func(a, b) begin
   	var c = a + b;
   	return c;
   end
   
   print f(10, 20) # 30
   ```

6. arrays

   ```nnlang
   # arrays behave like `real` associative array with integers as keys
   # also arrays are indexed starting from 1
   
   var arr = [];
   arr[1] = "hi";
   arr[15] = 3.14;
   
   print arr; # {1: "hi", 15: 3.14}
   
   ```

7.  tuples

   ```nnlang
   # tuples behave like structures
   
   var t = {a=10, b=20};
   t.a += 10;
   print t; # {a=20, b=20}
   
   # to change tuple structure we should add new tuple to it
   t += {a=10, c=30};
   print t; # {a=30, b=20, c=30}
   
   # we can access tuple elements by order (starting from 1)
   print t.1; # will print t.a value
   print t.2; # will print t.b value
   print t.3; # will print t.c value
   ```

   