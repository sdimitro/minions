const std = @import("std");

fn compute_values(file_path: []const u8) !struct { sum: i32, similarity: i32 } {
    var gpa = std.heap.GeneralPurposeAllocator(.{.thread_safe=true}){};
    const allocator = gpa.allocator();
    defer std.debug.assert(gpa.deinit() == .ok);

    const file = try std.fs.cwd().openFile(file_path, .{});
    defer file.close();

    var left_col = std.ArrayList(i32).init(allocator);
    defer left_col.deinit();
    var right_col = std.ArrayList(i32).init(allocator);
    defer right_col.deinit();

    while(try file.reader().readUntilDelimiterOrEofAlloc(allocator, '\n', std.math.maxInt(usize))) |line| {
        defer allocator.free(line);

        var tokens = std.mem.splitSequence(u8, line, "   ");
        const lnum = try std.fmt.parseInt(i32, tokens.next().?, 10);
        try left_col.append(lnum);

        const rnum = try std.fmt.parseInt(i32, tokens.next().?, 10);
        try right_col.append(rnum);
    }

    std.mem.sort(i32, left_col.items, {}, std.sort.asc(i32));
    std.mem.sort(i32, right_col.items, {}, std.sort.asc(i32));

    var sum: i32 = 0;
    for (left_col.items, right_col.items) |lnum, rnum| {
        sum += @intCast(@abs(lnum - rnum));
    }

    var similarity: i32 = 0;
    outer: while (true) {
        // Look on the right column and sum duplicate numbers
        const rnum = right_col.popOrNull();
        if (rnum == null) {
            break;
        }
        var rcount: i32 = 1;
        while (right_col.getLastOrNull() == rnum.?) {
            rcount += 1;
            _ = right_col.pop();
        }

        // Look on the left column
        var lnum: ?i32 = left_col.getLastOrNull();
        if (lnum == null) {
            break;
        }

        //Keep looping if we need to catch up
        while (lnum.? > rnum.?) {
            _ = left_col.pop();
            lnum = left_col.getLastOrNull();
            if (lnum == null) {
                break :outer;
            }
        }
        // If we have a match, keep looping
        while (lnum == rnum) {
            similarity += lnum.? * rcount;
            _ = left_col.pop();
            lnum = left_col.getLastOrNull();
        }
    }
    return . {.sum = sum, .similarity = similarity};
}

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();

    var gpa = std.heap.GeneralPurposeAllocator(.{.thread_safe=true}){};
    const allocator = gpa.allocator();
    defer std.debug.assert(gpa.deinit() == .ok);

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    for(args[1..]) |file_path| {
        const res = compute_values(file_path) catch |err| {
            std.log.err("{s}: {s}", .{file_path, @errorName(err)});
            continue;
        };
        try stdout.print("{s}: sum={} similarity={}\n", .{file_path, res.sum, res.similarity});
    }
}

test "run against example file" {
     const res = try compute_values("input/example_input.txt");
     try std.testing.expectEqual(11, res.sum);
     try std.testing.expectEqual(31, res.similarity);
}

test "run against actual file" {
     const res = try compute_values("input/aoc2024_input.txt");
     try std.testing.expectEqual(1579939, res.sum);
     try std.testing.expectEqual(20351745, res.similarity);
}
