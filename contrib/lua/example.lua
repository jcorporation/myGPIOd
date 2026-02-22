function changeMPDvolume()
    -- This example function can be used to change the MPD volume with a rotary encoder
    -- get value of the GPIOs
    clk = gpioGet(4)
    dt = gpioGet(5)
    -- Check rotation direction
    if clk == dt then
        mpc("volume 5")
    else
        mpc("volume -5")
    end
end

function testFunc(arg1)
    -- get and print the argument
    print("Arg1:"..arg1)
    -- set value of GPIO 5 to high
    gpioSet(5, 1)
end

-- Check if the left shift key was pressed
function keyPress(key)
    local _, value = inputEvGet("/dev/input/event3", "KEY_LEFTSHIFT")
    if value == 1 then
        print("Left shift key was pressed with key " .. key)
    else
        print("Left shift key was not pressed with key " .. key)
    end
end
