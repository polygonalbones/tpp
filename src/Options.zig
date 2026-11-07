const std = @import("std");

const Options = @This();

pub const CppStandard = enum {
    @"c++11",
};

standard: CppStandard,
trigraphs: bool,

pub const defaults: Options = .{
    .standard = .@"c++11",
    .trigraphs = false,
};
