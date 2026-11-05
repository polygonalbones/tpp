const std = @import("std");

const Options = @This();

const CppStandard = enum {
    cpp98,
};

standard: CppStandard,

pub const defaults: Options = .{
    .standard = .cpp98,
};
