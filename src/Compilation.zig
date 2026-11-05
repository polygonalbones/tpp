const std = @import("std");
const Options = @import("Options.zig");

const Compilation = @This();

gpa: std.mem.Allocator,
source_files: []const []const u8,
options: Options,

pub fn compile(self: *const Compilation) !void {
    _ = self;
}
