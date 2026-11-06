const std = @import("std");

const Options = @This();

const CppStandard = enum {
    cpp11,
};

standard: CppStandard,
enable_trigraphs: bool,

pub const defaults: Options = .{
    .standard = .cpp11,
    .enable_trigraphs = false,
};
