/*
 * SYSCALL_DEFINE3(poll, struct pollfd __user *, ufds, unsigned int, nfds, long, timeout_msecs)
 */
{
	.name = "poll",
	.num_args = 3,
	.arg1name = "ufds",
	.arg1type = ARG_ADDRESS,
	.arg2name = "nfds",
	.arg2type = ARG_LEN,
	.arg3name = "timeout_msecs",
},