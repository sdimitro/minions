const std = @import("std");

const data = @embedFile("aoc_input.txt");
const width = std.mem.indexOf(u8, data, "\n").? + 1; // +1 to include the newline

const CeresResult = struct {
    ceres_count: u64,
    x_count: u64,
};

fn ceres_check(starting_index: usize, step: usize, needle: []const u8) u8 {
    @setEvalBranchQuota(1000000); // Let the compiler spin for a bit :)

    var score: u8 = 0;
    // normal
    var matched: usize = 0;
    var index = starting_index;
    for (needle) |character| {
        if (data[index] != character) {
            break;
        }
        matched += 1;
        if (matched == needle.len) {
            score += 1;
            break;
        }
        const res = @addWithOverflow(index, step);
        if (res[1] != 0 or res[0] >= data.len) {
            break;
        }
        index = res[0];
    }

    // reverse
    matched = 0;
    index = starting_index;
    for (needle) |character| {
        if (data[index] != character) {
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

fn x_check(index: usize) bool {
    if (data[index] != 'A') {
        return false;
    }
    if (index < width or index > (data.len - width)) {
        return false;
    }
    const linePos = index % width;
    if (linePos == 0 or linePos == (width - 1)) {
        return false;
    }
    const upLeft = data[index - width - 1];
    const upRight = data[index - width + 1];
    const downLeft = data[index + width - 1];
    const downRight = data[index + width + 1];
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

fn ceres_search() CeresResult {
    const needle = "XMAS";
    var ceres_count: u64 = 0;
    var x_count: u64 = 0;
    for (data, 0..) |character, index| {
        if (character == '\n') {
            continue;
        }
        ceres_count += ceres_check(index, 1, needle); // horizontal fwd & bwd
        ceres_count += ceres_check(index, width, needle); // vertical fwd & bwd
        ceres_count += ceres_check(index, width + 1, needle); // right diagonal fwd & bwd
        ceres_count += ceres_check(index, width - 1, needle); // left diagonal fwd & bwd
        if (x_check(index)) {
            x_count += 1;
        }
    }
    return .{ .ceres_count = ceres_count, .x_count = x_count };
}

pub fn main() !void {
    const stdout = std.io.getStdOut().writer();
    const res = comptime ceres_search();
    try stdout.print("aoc_input.txt: {} | {}\n", .{ res.ceres_count, res.x_count });
}
