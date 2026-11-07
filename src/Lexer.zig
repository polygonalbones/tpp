const std = @import("std");

const Lexer = @This();

source: []const u8,
pos: usize,

pub const Token = struct {
    kind: enum {
        unknown,
        identifier,
        keyword,
        literal,
        operator,
        punctuator,
        eof,
    },
    span: struct {
        start: usize,
        end: usize,
    },
};

pub fn fromFile(gpa: std.mem.Allocator, io: std.Io, path: []const u8) !Lexer {
    var file = try std.Io.Dir.cwd().openFile(io, path, .{ .mode = .read_only });
    defer file.close(io);
    
    const stat = try file.stat(io);
    
    const buffer = try gpa.alloc(u8, @intCast(stat.size));
    errdefer gpa.free(buffer);

    _ = try file.readPositionalAll(io, buffer, 0);
    return .{ .source = buffer, .pos = 0 };
}

inline fn isNonDigit(char: u8) bool {
    return std.ascii.isAlphabetic(char) or char == '_';
}

pub fn next(self: *Lexer) Token {
    while(self.pos < self.source.len) {
        switch(self.source[self.pos]) {
            ' ', '\t', '\x0b', '\n', '\r', '\x0c' => self.pos += 1,
            else => break,
        }
    }

    var token: Token = .{
        .kind = .unknown,
        .span = .{ .start = self.pos, .end = self.pos }
    };

    if(self.pos == self.source.len) {
        token.kind = .eof;
        return token;
    }

    // identifier:
    //      identifier-nondigit
    //      identifier identifier-nondigit
    //      identifier digit
    if(isNonDigit(self.source[self.pos])) {
        token.kind = .identifier;
        self.pos += 1;
        while(self.pos < self.source.len and
            (isNonDigit(self.source[self.pos]) or std.ascii.isDigit(self.source[self.pos])))
            {
                self.pos += 1;
            }
        token.span.end = self.pos;
    }

    return token;
}

pub fn deinit(self: *Lexer, gpa: std.mem.Allocator) void {
    gpa.free(self.source);
}
