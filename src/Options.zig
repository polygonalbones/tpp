const std = @import("std");

const Options = @This();

pub const CppStandard = enum {
    @"c++11",
};

standard: CppStandard,
enable_trigraphs: bool,

pub const defaults: Options = .{
    .standard = .@"c++11",
    .enable_trigraphs = false,
};
