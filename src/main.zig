const std = @import("std");
const builtin = @import("builtin");
const Compilation = @import("Compilation.zig");
const Options = @import("Options.zig");

var debug_allocator: std.heap.DebugAllocator(.{}) = .init;
var threaded_io = std.Io.Threaded.init_single_threaded;

// compiler state
var source_files: std.ArrayList([]const u8) = .empty;
var options: Options = .defaults;

pub fn main(init: std.process.Init.Minimal) u8 {
    const gpa, const is_debug = switch(builtin.mode) {
        .Debug, .ReleaseSafe => .{ debug_allocator.allocator(), true },
        .ReleaseFast, .ReleaseSmall => .{ std.heap.smp_allocator, false },
    };
    defer if(is_debug) {
        _ = debug_allocator.deinit();
    };

    const io = threaded_io.io();
    
    var stdout_buffer: [1024]u8 = undefined;
    var stdout_writer = std.Io.File.stdout().writerStreaming(io, &stdout_buffer);
    const stdout = &stdout_writer.interface;
    
    var args_iter = try init.args.iterateAllocator(gpa);
    defer args_iter.deinit();

    defer source_files.deinit(gpa);

    _ = args_iter.next();
    while(args_iter.next()) |arg| {
        if(arg.len == 0) {
            _ = stdout.print("tpp: Empty source file paths are invalid\n", .{}) catch {};
            _ = stdout.flush() catch {};
            return 1;
        }
        
        if(arg[0] == '-') {
            if(std.mem.eql(u8, arg, "--help")) {
                _ = stdout.print("Usage: tpp <source_files> <arguments>\n", .{}) catch {};
                _ = stdout.flush() catch {};
                return 0;
            }
            else if(std.mem.eql(u8, arg, "-std")) {
                const arg2 = args_iter.next() orelse {
                    print_std_opt_error(stdout, null) catch {};
                    return 1;
                };
                
                options.standard = std.meta.stringToEnum(Options.CppStandard, arg2) orelse {
                    print_std_opt_error(stdout, arg2) catch {};
                    return 1;
                };
            }
            else if(std.mem.cutPrefix(u8, arg, "-std=")) |arg2| {
                if(std.meta.stringToEnum(Options.CppStandard, arg2)) |standard| {
                    options.standard = standard;
                }
                else {
                    _ = print_std_opt_error(stdout, arg2) catch {};
                    return 1;
                }
            }
            else if(std.mem.eql(u8, arg, "-dump-tokens")) {
                options.dump = .tokens;
            }
            else {
                _ = stdout.print("tpp: Unknown argument '{s}'\n", .{arg}) catch {};
                _ = stdout.flush() catch {};
                return 1;
            }
        }
        else {
            source_files.append(gpa, arg) catch {};
        }
    }

    if(source_files.items.len == 0) {
        _ = stdout.print("tpp: No source files provided\nRun with --help for usage.\n", .{}) catch {};
        _ = stdout.flush() catch {};
        return 1;
    }

    const comp: Compilation = .{
        .gpa = gpa,
        .io = io,
        .source_files = source_files.items,
        .options = options,
    };
    
    comp.compile(stdout) catch {
        return 1;
    };
    return 0;
}

fn print_std_opt_error(stdout: *std.Io.Writer, arg: ?[]const u8) !void {
    const std_values = comptime blk: {
        var str: []const u8 = "";
        for(std.meta.fieldNames(Options.CppStandard), 0..) |f, i| {
            if(i != 0) str = str ++ ", ";
            str = str ++ f;
        }
        break :blk str;
    };
    _ = stdout.print("tpp: Invalid -std argument '{s}'\nPossible values are: {s}\n", .{ arg orelse "", std_values }) catch {};
    _ = stdout.flush() catch {};
}
