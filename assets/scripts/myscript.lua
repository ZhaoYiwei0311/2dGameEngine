-- lua global variable
some_variable = 7 * 6;

config = {
    title = "My Game Engine",
    fullscreen = false,
    resolution = {
        width = 800,
        height = 600
    }
}

function factorial(n)
    i = n;
    result = 1;
    while i >= 1 do
        result = result * i;
        i = i - 1;
    end
    return result
end

factorial(5)
print("lua cube "..cube(3))