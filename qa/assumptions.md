# Assumptions

## Error Cases

### Syntax Error report

```ml
print print
return return
print return
return print

print ()
print 1)
print (1
print 2)))
print 3,
print 2, 3,
print 13.0 !
print printf - 1 ** 3 // 1 - - 1 + +
```

### Function Args mismatch

```ml
function f a b
    return a + b

print f(1, 2)
print f(1)
print f()
print f(1, 2, 3)
```

### Function not defined

```ml
x <- 10
print x + y() # y not defined
print x + z(10 + y()) # z and y is a functional call, but not define

function y
    return 1
function z a b
    return y() + 1 + a + b
```

### Duplicate parameter

```ml
function x y y # identifier name conflict
    return x + y # x is function, use without () invoke.

function x z
    return z + 1
```

### Invalid Identifiers

```ml
1xas2 <- 10
d2zx. <- 3
123.zsda <- 1
._123asd <- 0
```

### Not commence global statments without indentation

```ml
	print a
```

### Divide by zero is a runtime exception

## Valid Cases

### Accept cmdline args

```ml
# 9 is printed
#
one <- 1
#
function increment value
	print one - value
	return value + one
#
print increment(3) + increment(4) + arg0
```

`./runml progam.ml 2`

```ml
print arg0 * arg1
```

`./runml progam.ml 2 10`

### Variable not defined, initialize to 0

```ml
print x - y
```