const std = @import("std");
const builtin = @import("builtin");
const Compilation = @import("Compilation.zig");
const Options = @import("Options.zig");

var debug_allocator: std.heap.DebugAllocator(.{}) = .init;

// compiler state
var source_files: std.ArrayList([]const u8) = .empty;
var options: Options = .defaults;

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

    defer source_files.deinit(gpa);

    _ = args_iter.next();
    while(args_iter.next()) |arg| {
        if(arg[0] == '-') {
            // TODO: -Wall, -Wextra, -D, -o
        }
        else {
            try source_files.append(gpa, arg);
        }
    }

    const unit: Compilation = .{
        .gpa = gpa,
        .source_files = source_files.items,
        .options = options,
    };
    try unit.compile();
}
