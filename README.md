ebpf-tools

port of bcc libbpf-tools and ply

how to use:

1.clone ebpf-tool code
git clone --recurse-submodules git@github.com:qualcomm-yuanjiey/ebpf-tools.git

2.compile ebpf-tool
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- CC=aarch64-linux-gnu-gcc  -j256

3.clone linux kernel
git clone "kernel.git"

4.config your kernel defconfig/menuconfig
+CONFIG_DEBUG_INFO_BTF=y
+CONFIG_FTRACE=y
+CONFIG_DEBUG_INFO_REDUCED=n
+CONFIG_STACK_TRACER=y
+CONFIG_KPROBES=y

5.boot your kernel boot.img to your board, adb and shell is ok 
C:\Users\yuanjiey>adb shell
/ #
/ #
/ # ls
bin           init          mnt           sbin          yyj_test
boot          init.d        proc          sys
dev           kernel-tests  root          tmp
etc           lib           rootfs        usr
home          media         run           var
/ #

5.push ebpf exe(example: offcputime) to your board
adb push ebpf-tools\libbpf-tools\offcputime /

6.run

/ # ./offcputime
Tracing off-CPU time (us) of all threads... Hit Ctrl-C to end.
^C    bpf_prog_4c49d2a608ac53c7_handle_sched_switch
    bpf_prog_4c49d2a608ac53c7_handle_sched_switch
    bpf_prog_003e03a6eea60dfa_sched_switch
    bpf_trace_run4
    __bpf_trace_sched_switch
    __schedule
    schedule
    schedule_timeout
    wait_for_completion_interruptible
    ffs_epfile_io
    ffs_epfile_write_iter
    vfs_write
    ksys_write
    __arm64_sys_write
    invoke_syscall
    el0_svc_common.constprop.0
    do_el0_svc
    el0_svc
    el0t_64_sync_handler
    el0t_64_sync
    write
    [unknown]
    [unknown]
    [unknown]
    [unknown]
    [unknown]
    [unknown]
    [unknown]
    -                adbd (509)
        394


7.how to write a ebpf program
follow this commit
https://github.com/qualcomm-yuanjiey/ebpf-tools/commit/8b895b3d220036d62ad99f068f170a6fbbe4c6f1
