a <- 10
b <- 2
a # syntax error. not meet the requirement of a statement.

function a b # identifier name conflict.
    return a + b

function x y y # identifier name conflict
    return x + y # x is function, use without () invoke.

function x z # identifier concflict, refine.
    return z + 1