const std = @import("std");

const Lexer = @This();

source: []const u8,
pos: usize,

pub const Token = struct {
    kind: enum {
        unknown,
        identifier,
        keyword,
        integer_literal,
        character_literal,
        floating_literal,
        string_literal,
        boolean_literal,
        pointer_literal,
        user_defined_literal,
        operator,
        punctuator,
        eof,
    },
    span: struct {
        start: usize,
        end: usize,
    },
};

const keywords: std.StaticStringMap(void) = .initComptime(.{
    .{"alignas"},    .{"continue"},     .{"friend"},    .{"register"},         .{"true"},
    .{"alignof"},    .{"decltype"},     .{"goto"},      .{"reinterpret_cast"}, .{"try"},
    .{"asm"},        .{"default"},      .{"if"},        .{"return"},           .{"typedef"},
    .{"auto"},       .{"delete"},       .{"inline"},    .{"short"},            .{"typeid"},
    .{"bool"},       .{"do"},           .{"int"},       .{"signed"},           .{"typename"},
    .{"break"},      .{"double"},       .{"long"},      .{"sizeof"},           .{"union"},
    .{"case"},       .{"dynamic_cast"}, .{"mutable"},   .{"static"},           .{"unsigned"},
    .{"catch"},      .{"else"},         .{"namespace"}, .{"static_assert"},    .{"using"},
    .{"char"},       .{"enum"},         .{"new"},       .{"static_cast"},      .{"virtual"},
    .{"char16_t"},   .{"explicit"},     .{"noexcept"},  .{"struct"},           .{"void"},
    .{"char32_t"},   .{"export"},       .{"nullptr"},   .{"switch"},           .{"volatile"},
    .{"class"},      .{"extern"},       .{"operator"},  .{"template"},         .{"wchar_t"},
    .{"const"},      .{"false"},        .{"private"},   .{"this"},             .{"while"},
    .{"constexpr"},  .{"float"},        .{"protected"}, .{"thread_local"},
    .{"const_cast"}, .{"for"},          .{"public"},    .{"throw"},
});

pub fn fromFile(gpa: std.mem.Allocator, io: std.Io, path: []const u8) !Lexer {
    var file = try std.Io.Dir.cwd().openFile(io, path, .{ .mode = .read_only });
    defer file.close(io);
    
    const stat = try file.stat(io);
    
    const buffer = try gpa.alloc(u8, @intCast(stat.size));
    errdefer gpa.free(buffer);

    _ = try file.readPositionalAll(io, buffer, 0);
    return .{ .source = buffer, .pos = 0 };
}

fn isNonDigit(char: u8) bool {
    return std.ascii.isAlphabetic(char) or char == '_';
}

fn isNonZeroDigit(char: u8) bool {
    return switch(char) {
        '1'...'9' => true,
        else => false,
    };
}

fn isOctalDigit(char: u8) bool {
    return switch(char) {
        '0'...'7' => true,
        else => false,
    };
}

fn isHexadecimalDigit(char: u8) bool {
    return switch(char) {
        '0'...'9' => true,
        'a'...'f' => true,
        'A'...'F' => true,
        else => false,
    };
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

    //  identifier:
    //             identifier-nondigit
    //             | identifier identifier-nondigit
    //             | identifier digit
    if(isNonDigit(self.source[self.pos])) {
        token.kind = .identifier;
        self.pos += 1;
        while(self.pos < self.source.len and (isNonDigit(self.source[self.pos]) or std.ascii.isDigit(self.source[self.pos]))) {
            self.pos += 1;
        }
        token.span.end = self.pos;
        if(keywords.get(self.source[token.span.start..token.span.end]) != null) {
            token.kind = .keyword;
        }
    }
    //  integer-literal:
    //                  decimal-literal integer-suffix?
    //                  | octal-literal integer-suffix?
    //                  | hexadecimal-literal integer-suffix?
    else if(std.ascii.isDigit(self.source[self.pos])) {
        //  TODO: handle integer suffixes
        
        //  decimal-literal:
        //                  nonzero-digit
        //                  | decimal-literal digit
        if(isNonZeroDigit(self.source[self.pos])) {
            token.kind = .integer_literal;
            self.pos += 1;
            while(self.pos < self.source.len and (isNonZeroDigit(self.source[self.pos]) or std.ascii.isDigit(self.source[self.pos]))) {
                self.pos += 1;
            }
            token.span.end = self.pos;
        }
        else if(self.source[self.pos] == '0') {
            token.kind = .integer_literal;
            self.pos += 1;
            //  hexadecimal-literal:
            //                      0x hexadecimal-digit
            //                      | 0X hexadecimal-digit
            //                      | hexadecimal-literal hexadecimal-digit       
            if(self.pos < self.source.len and (self.source[self.pos] == 'x' or self.source[self.pos] == 'X')) {
                self.pos += 1;
                while(self.pos < self.source.len and isHexadecimalDigit(self.source[self.pos])) {
                    self.pos += 1;
                }
            }
            //  octal-literal:
            //                0
            //                | octal-literal octal-digit
            else {
                while(self.pos < self.source.len and isOctalDigit(self.source[self.pos])) {
                    self.pos += 1;
                }
            }
            token.span.end = self.pos;
        }
    }

    return token;
}

pub fn deinit(self: *Lexer, gpa: std.mem.Allocator) void {
    gpa.free(self.source);
}
