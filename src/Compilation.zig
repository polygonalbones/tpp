const std = @import("std");
const Lexer = @import("Lexer.zig");
const Options = @import("Options.zig");

const Compilation = @This();

gpa: std.mem.Allocator,
io: std.Io,
source_files: []const []const u8,
options: Options,

pub fn compile(self: *const Compilation) !void {
    for(self.source_files) |source_file| {
        var lexer = try Lexer.fromFile(self.gpa, self.io, source_file);
        defer lexer.deinit(self.gpa);
    }
}
