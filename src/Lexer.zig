const std = @import("std");

const Lexer = @This();

source: []const u8,

const Token = struct {
    kind: enum {
        unknown,
        eof,
    },
    value: []const u8,
};

pub fn fromFile(gpa: std.mem.Allocator, io: std.Io, path: []const u8) !Lexer {
    var file = try std.Io.Dir.cwd().openFile(io, path, .{ .mode = .read_only });
    defer file.close(io);
    
    const stat = try file.stat(io);
    
    const buffer = try gpa.alloc(u8, @intCast(stat.size));
    errdefer gpa.free(buffer);

    _ = try file.readPositionalAll(io, buffer, 0);
    return .{ .source = buffer };
}

pub fn deinit(self: *Lexer, gpa: std.mem.Allocator) void {
    gpa.free(self.source);
}
