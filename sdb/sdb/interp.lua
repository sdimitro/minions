lsdb = require("libsdb")

targets = {}

current_target = nil

-- TODO:
-- Need to break everything up to modules!

function list_targets()
	for k, v in pairs(targets) do
		if current_target == v then
			print(k .. "*")
		else
			print(k)
		end
	end
end

function load_core_impl(path)
	-- check if target is already present
	if targets[path] ~= nil then
		return -1
	end
	current_target = lsdb.new(path)
	current_target:load_core(path, "", 0)
	targets[path] = current_target
	return 0
end

-- load core and add it to the table of targets
-- TODO: Should give more options based on flag
--       Maybe optional args
function load_core(path)
	if load_core_impl(path) ~= 0 then
		print("SDB has already a target at " .. path)
		return
	end
	print("loaded " .. path)
end

-- TODO: maybe merge all implementation and pass a lambda
--       or have a if/else or something.
function load_file_impl(path)
	-- check if target is already present
	if targets[path] ~= nil then
		return -1
	end
	current_target = lsdb.new(path)
	current_target:load_file(path)
	targets[path] = current_target
	return 0
end

-- load file and add it to the table of targets
function load_file(path)
	if load_file_impl(path) ~= 0 then
		print("SDB has already a target at " .. path)
		return
	end
	print("loaded " .. path)
end

function attach_proc_impl(pid)
	-- check if target is already present
	if targets[pid] ~= nil then
		return -1
	end
	current_target = lsdb.new("pid" .. pid)
	current_target:attach_proc(pid, 0, 0)
	targets["pid" .. pid] = current_target
	return 0
end

-- attach to process and add it to the table of targets
-- TODO: Should give more options based on flag
--       Maybe optional args
function attach_proc(pid)
	if attach_proc_impl(pid) ~= 0 then
		print("SDB has already a target with pid=" .. pid)
		return
	end
	print("attached to " .. pid)
end

function swtch_impl(target)
	-- check if target exists
	if targets[path] == nil then
		return -1
	end
	current_target = targets[target]
	return 0
end

-- switch to different target
function swtch(target)
	if swtch_impl(target) ~= 0 then
		print(target .. " does not exist within SDB")
		return
	end
	print("switched to " .. current_target)
end

-- TODO: Figure out what to do with this
function regs()
	if current_target == nil then
		print("there is no current target.")
		return
	end
	for i=1,14 do
		print("%reg" .. i .. " = " ..
		    string.format("%#08x", current_target:reg(i)))
	end
end

function execfile()
	if current_target == nil then
		print("there is no current target.")
		return
	end
	print(current_target:execfile())
end

function contents()
	if current_target == nil then
		print("there is no current target.")
		return
	end

	local content_bitmap = current_target:contents()
	local contents = {}

	-- TODO: Store crap on a table and iterate through that
	-- TODO: put in separate file
	if content_bitmap & 0x0001 ~= 0 then table.insert(contents, "stack") end
	if content_bitmap & 0x0002 ~= 0 then table.insert(contents, "heap") end
	if content_bitmap & 0x0004 ~= 0 then table.insert(contents, "shfile") end
	if content_bitmap & 0x0008 ~= 0 then table.insert(contents, "shanon") end
	if content_bitmap & 0x0010 ~= 0 then table.insert(contents, "text") end
	if content_bitmap & 0x0020 ~= 0 then table.insert(contents, "data") end
	if content_bitmap & 0x0040 ~= 0 then table.insert(contents, "rodata") end
	if content_bitmap & 0x0080 ~= 0 then table.insert(contents, "map_anon") end
	if content_bitmap & 0x0100 ~= 0 then table.insert(contents, "shm") end
	if content_bitmap & 0x0200 ~= 0 then table.insert(contents, "ism") end
	if content_bitmap & 0x0400 ~= 0 then table.insert(contents, "dism") end
	if content_bitmap & 0x0800 ~= 0 then table.insert(contents, "ctf") end
	if content_bitmap & 0x1000 ~= 0 then table.insert(contents, "symtab") end
	if content_bitmap & 0x1fff ~= 0 then table.insert(contents, "all") end
	if content_bitmap == 0 then table.insert(contents, "none") end

	print(table.concat(contents, ","))
end

function release()
	if current_target == nil then
		print("there is no current target.")
		return
	end
	current_target:release(0)
	-- TODO:
	-- Empty current_target and empty its entry from the targets
	-- table!

	--current_target:release(0)
	-- TODO: On purpose just to check for errors of double call
	--       Finalize on how to handle this!
end

-- TODO: Should give more options based on flag
--       Maybe optional args
function quit()
	-- release everything
	for k, v in pairs(targets) do
		v:release(0)
		k = nil
	end
	os.exit()
end

