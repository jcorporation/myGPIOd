function testFunc(arg1)
    -- get the argument
    print("Arg1:"..arg1)
    -- get value of GPIO 4
    v = gpioGet(4)
    -- set value of GPIO 5 to high
    gpioSet(5, 1)
end
