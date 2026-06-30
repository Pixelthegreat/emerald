# Emerald Programming Language Overview
This is an overview of the Emerald programming language.

## Contents
- [Variables](#variables)
- [Numbers](#numbers)
- [Strings](#strings)
- [Lists](#lists)
- [Maps](#maps)
- [Conditional Logic](#conditional-logic)
- [Loops](#loops)
- [Functions](#functions)
- [Classes](#classes)
- [Errors](#errors)
- [Miscellaneous](#miscellaneous)

## Variables
Variables can be assigned using the `let` keyword:
```
let x = 10
```
They can be accessed by their name:
```
puts x
```

## Numbers
Numbers can be integers or floating point (decimal):
```
1   # Integer
1.0 # Floating point
```
Arithmetic can be performed on numbers of any kind:
```
1 + 2
1 - 2
1 * 2
1 / 2
1 % 2
```
Bitwise operations can be performed on integers:
```
1 | 2
1 & 2
1 >> 2
1 << 2
1 ^ 2
```
A number can be converted from a float to an integer using `toInteger`:
```
let f = 3.14
let i = toInteger(f) # 3
```

## Strings
Strings can be represented in single quotes or double quotes:
```
'Hello, world!'
"Hello, world!"
```
You can add strings together:
```
'A' + 'B' # 'AB'
```
You can repeat strings:
```
'A' * 3 # 'AAA'
```
You can convert a string to an integer using `toInteger`:
```
toInteger('3') # 3
toInteger('A') # Error
```
You can convert any value to a string using `toString`:
```
toString(3) # '3'
```

## Lists
You can make a list like so:
```
[]        # Empty list
[1, 2, 3] # List with values
```
You can add items to lists:
```
append(list, 4) # Now list is [1, 2, 3, 4]
```
You can get items from lists:
```
puts list[0]
puts list[2]
```
You can set items in lists:
```
let list[0] = 10
let list[2] = 30
```

## Maps
You can make a map (akin to dictionaries in Python and tables in Lua) like so:
```
{}               # Empty map
{'a': 1, 'b': 2} # Map with items
```
You can get items from maps:
```
map.a
map['a'] # Same thing
```
You can set items in maps:
```
let map.a = 10
let map['a'] = 10 # Same thing
```

## Conditional Logic
You can use `if` statements to run code conditionally:
```
if x == y then
    doStuff()
end
```
You can compare values in different ways:
```
a == b
a != b
a < b
a > b
a <= b
a >= b
```
You can use `or` and `and` in conditions:
```
if x or y then
    doStuff()
end
if x and y then
    doStuff()
end
```
You can use an `if` statement to produce a value:
```
let number = if magic > 10 then 10 else then 12 end
```
You can chain other conditions to an `if` statement using `elif` and `else`:
```
if x then
    doStuff()
elif y then
    doStuff()
else then
    doStuff()
end
```

## Loops
You can use `while` loops to loop until a condition is no longer met:
```
while true then
    doStuff()
end
```
You can use `for` loops to loop for a set number of times:
```
for i = 0 to 10 then
    puts i
end
```
You can use `foreach` loops to iterate over an object (like a list or a string):
```
foreach i in [1, 2, 3] then
    puts i
end
```

## Functions
You can define functions like so:
```
func doStuff() then
    ...
end
```
You can call functions:
```
doStuff()
```
You can give functions arguments:
```
func doStuff(x, y) then
    ...
end
doStuff(10, 11)
```
You can return values from functions:
```
func add(x, y) then
    return x + y
end
```
You can define functions anonymously (without a name):
```
let doStuff = func() then
    ...
end
```

## Classes
You can define classes like so:
```
class Animal then
    ...
end
```
You can define methods in classes:
```
class Animal then
    func _initialize(this, name, sound) then

        let this.name = name
        let this.sound = sound
    end
end
```
You can create instances of classes:
```
let animal = Animal('Cow', 'Moo')
```
You can call methods on classes:
```
animal.speak()
```
You can subclass a class, inheriting its attributes while adding your own:
```
class SpecialAnimal of Animal then
    func doSpecialStuff(this) then
        ...
    end
end
```

## Errors
If need be, you can catch errors before they terminate the program:
```
try then
    doInvalidStuff()
catch then
    ...
end
```
You can catch errors of specific types:
```
try then
    ...
catch e = RuntimeError then
    ...
end
```
You can raise your own errors:
```
raise Error('Hello, world!')
```
You can subclass `Error` to create your own custom error type:
```
class MyError of Error then
    func _initialize(this, message) then
        Error._initialize(this, message)
    end
end
```

## Miscellaneous
You can print values in three ways:
```
puts a, b, c
println(a)
print(a)   # Does not add a newline at the end
```
You can write comments like so:
```
doStuff() # This comment persists until the end of the line
```
