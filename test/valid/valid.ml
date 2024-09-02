# Comment at beginning
#
c <- d + e # Comment at middle d == 0.0, e == 0.0 c = 0.0
# -------------------------------------------------
# an assignment statement, nothing is printed
x <- 2.3
# -------------------------------------------------
# an assignment statement, 2.500000 is printed
x <- 2.5
print x
# -------------------------------------------------
# 3.500000 is printed
print 3.5
# -------------------------------------------------
# 24 is printed
x <- 8
y <- 3
print x * y
# -------------------------------------------------
# 18 is printed
#
function printsum a b
	print a + b
#
printsum (12, 6)
# -------------------------------------------------
# 72 is printed
#
function multiply a b
	return a * b
#
print multiply(12, 6)
# -------------------------------------------------
# 50 is printed
#
function multiply a b
	x <- a * b
	return x
#
print multiply(10, 5)
# -------------------------------------------------
# 9 is printed
#
one <- 1
#
function increment value
	return value + one
#
print increment(3) + increment(4)
# -------------------------------------------------
function add a b
    return a * b
print add(a, b) + arg0 - arg1
# -------------------------------------------------


print 10
print add(10, 2)
function add a b
	print a
	print b
	return a + b

print add(10, 2)

function add a b
	print a
	print b
	return a + b

print add(10, 2)