const std = @import("std");

const Options = @This();

pub const CppStandard = enum {
    @"c++11",
};

standard: CppStandard,

pub const defaults: Options = .{
    .standard = .@"c++11",
};
