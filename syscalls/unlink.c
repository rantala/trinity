/*
 * SYSCALL_DEFINE1(unlink, const char __user *, pathname)
 */
#include "trinity.h"
#include "sanitise.h"

struct syscall syscall_unlink = {
	.name = "unlink",
	.num_args = 1,
	.arg1name = "pathname",
	.arg1type = ARG_ADDRESS,
};