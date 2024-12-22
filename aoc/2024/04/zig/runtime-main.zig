const std = @import("std");
const assert = std.debug.assert;

const CeresResult = struct {
    ceres_count: u64,
    x_count: u64,
};

const Grid = struct {
    data: []const u8,
    width: usize,
    height: usize,

    var gpa = std.heap.GeneralPurposeAllocator(.{ .thread_safe = true }){};
    const allocator = gpa.allocator();

    pub fn init(file_path: []const u8) !Grid {
        const file = try std.fs.cwd().openFile(file_path, .{});
        defer file.close();

        const stat = try file.stat();
        const data = try allocator.alloc(u8, stat.size);
        const bytes_read = try file.readAll(data);
        assert(bytes_read == stat.size);

        var width: usize = 0;
        while (data[width] != '\n') {
            width += 1;
        }
        width += 1; // include the newline in width
        return Grid{
            .data = data,
            .width = width,
            .height = data.len / width,
        };
    }

    fn ceres_check(self: Grid, starting_index: usize, step: usize, needle: []const u8) u8 {
        var score: u8 = 0;

        // normal
        var matched: usize = 0;
        var index = starting_index;
        for (needle) |character| {
            if (self.data[index] != character) {
                break;
            }
            matched += 1;
            if (matched == needle.len) {
                score += 1;
                break;
            }
            const res = @addWithOverflow(index, step);
            if (res[1] != 0 or res[0] >= self.data.len) {
                break;
            }
            index = res[0];
        }

        // reverse
        matched = 0;
        index = starting_index;
        for (needle) |character| {
            if (self.data[index] != character) {
                break;
            }
            matched += 1;
            if (matched == needle.len) {
                score += 1;
                break;
            }
            const res = @subWithOverflow(index, step);
            if (res[1] != 0) {
                break;
            }
            index = res[0];
        }
        return score;
    }

    fn x_check(self: Grid, index: usize) bool {
        if (self.data[index] != 'A') {
            return false;
        }
        if (index < self.width or index > (self.data.len - self.width)) {
            return false;
        }
        const linePos = index % self.width;
        if (linePos == 0 or linePos == (self.width - 1)) {
            return false;
        }
        const upLeft = self.data[index - self.width - 1];
        const upRight = self.data[index - self.width + 1];
        const downLeft = self.data[index + self.width - 1];
        const downRight = self.data[index + self.width + 1];
        if (upLeft == 'M' and downRight == 'S') {
            if (upRight == 'M' and downLeft == 'S') {
                return true;
            } else if (upRight == 'S' and downLeft == 'M') {
                return true;
            }
        } else if (upLeft == 'S' and downRight == 'M') {
            if (upRight == 'M' and downLeft == 'S') {
                return true;
            } else if (upRight == 'S' and downLeft == 'M') {
                return true;
            }
        }
        return false;
    }

    pub fn ceres_search(self: Grid, needle: []const u8) CeresResult {
        var ceres_count: u64 = 0;
        var x_count: u64 = 0;
        for (self.data, 0..) |character, index| {
            if (character == '\n') {
                continue;
            }
            ceres_count += self.ceres_check(index, 1, needle); // horizontal fwd & bwd
            ceres_count += self.ceres_check(index, self.width, needle); // vertical fwd & bwd
            ceres_count += self.ceres_check(index, self.width + 1, needle); // right diagonal fwd & bwd
            ceres_count += self.ceres_check(index, self.width - 1, needle); // left diagonal fwd & bwd
            if (self.x_check(index)) {
                x_count += 1;
            }
        }
        return .{ .ceres_count = ceres_count, .x_count = x_count };
    }

    pub fn deinit(self: Grid) void {
        allocator.free(self.data);
        std.debug.assert(gpa.deinit() == .ok);
    }
};

fn ceres_search(file_path: []const u8) !CeresResult {
    const grid = try Grid.init(file_path);
    defer grid.deinit();
    return grid.ceres_search("XMAS");
}

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();

    var gpa = std.heap.GeneralPurposeAllocator(.{ .thread_safe = true }){};
    const allocator = gpa.allocator();
    defer std.debug.assert(gpa.deinit() == .ok);

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    for (args[1..]) |file_path| {
        const res = ceres_search(file_path) catch |err| {
            std.log.err("{s}: {s}", .{ file_path, @errorName(err) });
            continue;
        };
        try stdout.print("{s}: {} | {}\n", .{ file_path, res.ceres_count, res.x_count });
    }
}

test "run against example file" {
    const res = try ceres_search("example_input.txt");
    try std.testing.expectEqual(18, res.ceres_count);
    try std.testing.expectEqual(9, res.x_count);
}

test "run against actual file" {
    // const res = try ceres_search("aoc_input.txt");
    // try std.testing.expectEqual(2646, res.ceres_count);
    // try std.testing.expectEqual(0, res.x_count);
}
