# # file opens
# bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("%s %s\n", comm, str(args.filename)); }'


# syscall counts by process
bpftrace -e 'tracepoint:raw_syscalls:sys_enter{ @[comm] = count(); }'