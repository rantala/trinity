/*
 * SYSCALL_DEFINE2(removexattr, const char __user *, pathname, const char __user *, name)
 */
{
	.name = "removexattr",
	.num_args = 2,
	.arg1name = "pathname",
	.arg1type = ARG_ADDRESS,
	.arg2name = "name",
	.arg2type = ARG_ADDRESS,
},