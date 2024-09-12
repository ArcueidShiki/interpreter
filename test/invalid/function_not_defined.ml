x <- 10
print x + y() # y not defined
print x + z(10 + y()) # z and y is a functional call, but not define

function y
    return 1
function z a b
    return y() + 1 + a + b