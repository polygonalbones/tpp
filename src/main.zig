const std = @import("std");
const builtin = @import("builtin");

var debug_allocator: std.heap.DebugAllocator(.{}) = .init;

pub fn main(init: std.process.Init.Minimal) !void {
    const gpa, const is_debug = switch(builtin.mode) {
        .Debug, .ReleaseSafe => .{ debug_allocator.allocator(), true },
        .ReleaseFast, .ReleaseSmall => .{ std.heap.smp_allocator, false },
    };
    defer if(is_debug) {
        _ = debug_allocator.deinit();
    };
    
    var args_iter = try init.args.iterateAllocator(gpa);
    defer args_iter.deinit();

    _ = args_iter.next();
    while(args_iter.next()) |arg| {
        _ = arg;
    }
}
