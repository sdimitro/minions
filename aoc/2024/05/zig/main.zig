const std = @import("std");

const data = @embedFile("aoc_input.txt");
const MapEntry = struct { before: std.AutoHashMap(u32, bool) };

fn cmpByRule(context: std.AutoHashMap(u32, MapEntry), a: u32, b: u32) bool {
    if (context.get(b)) |e| {
        if (e.before.contains(a)) {
            return true;
        }
    }
    return false;
}

pub fn main() !void {
    var dataIter = std.mem.splitSequence(u8, data, "\n\n");
    const rules = dataIter.next().?;
    const updates = dataIter.next().?;

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer std.debug.assert(gpa.deinit() == .ok);

    var ruleMap = std.AutoHashMap(u32, MapEntry).init(
        allocator,
    );
    defer {
        var iter = ruleMap.iterator();
        while (iter.next()) |entry| {
            entry.value_ptr.before.deinit();
        }
        ruleMap.deinit();
    }

    var rulesIter = std.mem.splitAny(u8, rules, "\n");
    while (rulesIter.next()) |entry| {
        var entryIter = std.mem.splitAny(u8, entry, "|");
        const key = try std.fmt.parseInt(u32, entryIter.next().?, 0);
        const val = try std.fmt.parseInt(u32, entryIter.next().?, 0);

        if (ruleMap.getPtr(key)) |e| {
            try e.before.put(val, true);
        } else {
            var m = std.AutoHashMap(u32, bool).init(allocator);
            try m.put(val, true);
            try ruleMap.put(key, MapEntry{ .before = m });
        }
        if (!ruleMap.contains(val)) {
            try ruleMap.put(val, MapEntry{ .before = std.AutoHashMap(u32, bool).init(allocator) });
        }
    }

    var correct_score: u64 = 0;
    var incorrect_score: u64 = 0;
    var updatesIter = std.mem.splitAny(u8, updates, "\n");
    while (updatesIter.next()) |entry| {
        if (entry.len == 0) {
            break;
        }
        var list = std.ArrayList(u32).init(allocator);
        defer list.deinit();

        var entryIter = std.mem.splitAny(u8, entry, ",");
        while (entryIter.next()) |num| {
            try list.append(try std.fmt.parseInt(u32, num, 0));
        }
        std.mem.reverse(u32, list.items);

        var sortedList = try list.clone();
        defer sortedList.deinit();
        std.mem.sort(u32, sortedList.items, ruleMap, cmpByRule);

        if (std.mem.eql(u32, list.items, sortedList.items)) {
            correct_score += list.items[list.items.len / 2];
        } else {
            incorrect_score += sortedList.items[sortedList.items.len / 2];
        }
    }
    std.log.info("score: {} | {}", .{ correct_score, incorrect_score });
}
