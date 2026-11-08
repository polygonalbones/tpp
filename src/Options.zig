const std = @import("std");

const Options = @This();

pub const CppStandard = enum {
    @"c++11",
};

pub const Dump = enum {
    nothing,
    tokens,
};

standard: CppStandard,
dump: Dump,

pub const defaults: Options = .{
    .standard = .@"c++11",
    .dump = .nothing,
};
