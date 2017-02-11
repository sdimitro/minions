lsdb = require("lsdb")

x = lsdb.new("this is a test", "temp_cores/core.669")

-- get executable
print("executable -> " .. x:execfile())

print()

-- print registers 1 to 14
print("--- register sample ---")
for i=1,14 do
	print("%reg" .. i .. " = " .. string.format("%#08x", x:reg(i)))
end

