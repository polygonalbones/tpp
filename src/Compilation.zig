const std = @import("std");
const Lexer = @import("Lexer.zig");
const Options = @import("Options.zig");

const Compilation = @This();

gpa: std.mem.Allocator,
io: std.Io,
source_files: []const []const u8,
options: Options,

pub fn compile(self: *const Compilation, stdout: *std.Io.Writer) !void {
    for(self.source_files) |source_file| {
        var lexer = try Lexer.fromFile(self.gpa, self.io, source_file);
        defer lexer.deinit(self.gpa);

        blk: while(true) {
            const token = lexer.next();
            if(self.options.dump == .tokens) {
                _ = try stdout.print("{s} '{s}'\n", .{ @tagName(token.kind), lexer.source[token.span.start..token.span.end] });
            }
            switch(token.kind) {
                .eof => break :blk,
                .unknown => {
                    _ = stdout.print("tpp: Unknown token @ {d}..{d}", .{ token.span.start, token.span.end }) catch {};
                    _ = stdout.flush() catch {};
                    return error.UnknownToken;
                },
                else => {},
            }
        }

        _ = stdout.flush() catch {};
    }
}
