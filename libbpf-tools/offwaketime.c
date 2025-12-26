// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2016 Facebook
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <errno.h>
#include <stdbool.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "trace_helpers.h"
//#include "offcputime.h"
#include "offwaketime.skel.h"

#define PRINT_RAW_ADDR 0

/* counts, stackmap */
static int map_fd[2];
struct ksyms *ksyms = NULL;

static void print_ksym(__u64 addr)
{
    const struct ksym *sym;
	if (!addr)
		return;
	sym = ksyms__map_addr(ksyms, addr);
	if (!sym) {
		printf("ksym not found. Is kallsyms loaded?\n");
		return;
	}

	if (PRINT_RAW_ADDR)
		printf("%s/%llx;", sym->name, addr);
	else
		printf("%s;", sym->name);
}

#define TASK_COMM_LEN 16

struct key_t {
	char waker[TASK_COMM_LEN];
	char target[TASK_COMM_LEN];
	__u32 wret;
	__u32 tret;
};

static void print_stack(struct key_t *key, __u64 count)
{
	__u64 ip[PERF_MAX_STACK_DEPTH] = {};
	static bool warned;
	int i;

	printf("%s;", key->target);
	if (bpf_map_lookup_elem(map_fd[1], &key->tret, ip) != 0) {
		printf("---;");
	} else {
		for (i = PERF_MAX_STACK_DEPTH - 1; i >= 0; i--)
			print_ksym(ip[i]);
	}
	printf("-;");
	if (bpf_map_lookup_elem(map_fd[1], &key->wret, ip) != 0) {
		printf("---;");
	} else {
		for (i = 0; i < PERF_MAX_STACK_DEPTH; i++)
			print_ksym(ip[i]);
	}
	printf(";%s %lldus\n", key->waker, count);

	if ((key->tret == -EEXIST || key->wret == -EEXIST) && !warned) {
		printf("stackmap collisions seen. Consider increasing size\n");
		warned = true;
	} else if (((int)(key->tret) < 0 || (int)(key->wret) < 0)) {
		printf("err stackid %d %d\n", key->tret, key->wret);
	}
}

static void print_stacks(int fd)
{
	struct key_t key = {}, next_key;
	__u64 value;

	while (bpf_map_get_next_key(fd, &key, &next_key) == 0) {
		bpf_map_lookup_elem(fd, &next_key, &value);
		print_stack(&next_key, value);
		key = next_key;
	}
}

static void int_exit(int sig)
{
	print_stacks(map_fd[0]);
	exit(0);
}

int main(int argc, char **argv)
{
	struct offwaketime_bpf *obj;
	int delay = 1;
    int err;

    obj = offwaketime_bpf__open();
	if (!obj) {
		fprintf(stderr, "failed to open BPF object\n");
		return 1;
	}

	err = offwaketime_bpf__load(obj);
	if (err) {
		fprintf(stderr, "failed to load BPF programs\n");
		goto cleanup;
	}

	map_fd[0] = bpf_map__fd(obj->maps.counts);
	map_fd[1] = bpf_map__fd(obj->maps.stackmap);
	if (map_fd[0] < 0 || map_fd[1] < 0) {
		fprintf(stderr, "ERROR: finding a map in obj file failed\n");
		goto cleanup;
	}

	signal(SIGINT, int_exit);
	signal(SIGTERM, int_exit);

    ksyms = ksyms__load();
	if (!ksyms) {
		fprintf(stderr, "failed to load kallsyms\n");
		goto cleanup;
	}

	err = offwaketime_bpf__attach(obj);
	if (err) {
		fprintf(stderr, "failed to attach BPF programs\n");
		goto cleanup;
	}

	if (argc > 1)
		delay = atoi(argv[1]);
	sleep(delay);
	print_stacks(map_fd[0]);

cleanup:
    ksyms__free(ksyms);
	offwaketime_bpf__destroy(obj);
	return 0;
}
