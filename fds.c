#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fanotify.h>

#include "shm.h"
#include "files.h"
#include "pids.h"
#include "net.h"
#include "log.h"
#include "params.h"

unsigned int nr_file_fds = 0;

static void open_pipes(void)
{
	int pipes[2];
	unsigned int i;

	for (i = 0; i < MAX_PIPE_FDS; i+=2) {
		if (pipe(pipes) < 0) {
			perror("pipe fail.\n");
			exit(EXIT_FAILURE);
		}
		shm->pipe_fds[i] = pipes[0];
		shm->pipe_fds[i+1] = pipes[1];

		output(2, "fd[%d] = pipe\n", shm->pipe_fds[i]);
		output(2, "fd[%d] = pipe\n", shm->pipe_fds[i+1]);
	}
}

static void open_fanotify_fds(void)
{
	unsigned int i;

	for (i = 0; i < NR_FANOTIFY_FDS; ++i) {
		shm->fanotify_fds[i] = fanotify_init(0, 0);
		if (shm->fanotify_fds[i] >= 0)
			output(2, "fd[%d] = fanotify\n", shm->fanotify_fds[i]);
	}
}

static int get_new_random_fd(void)
{
	unsigned int i;
	unsigned int fd_index;
	FILE *file;
	int fd = 0;
	int ret;

	i = rand() % 4;

	if (do_specific_proto == TRUE)
		i = 1;

	if (no_files == TRUE)
		i = 1;

	/* Ugly special case.
	 * Sometimes, we can get here without any fd's setup.
	 * If this happens, we divide by zero if we pick case 0 because
	 * nr_file_fds is zero
	 *
	 * When this circumstance occurs, we just force it to use another network socket.
	 *
	 * FIXME: A better solution would be to like, actually open an fd. duh.
	 */
	if (nr_file_fds == 0)
		i = 1;


	switch (i) {
	case 0:
retry_file:
		fd_index = rand() % nr_file_fds;
		fd = shm->file_fds[fd_index];

		/* avoid stdin/stdout/stderr */
		if (logging == FALSE)
			ret = fileno(stderr);

		/* if logging is enabled, we want to make sure we skip
		 * over the logfiles, so get highest logfile fd. */
		else {
			file = shm->logfiles[shm->max_children - 1];
			if (file == NULL) {
				printf("## WTF, logfile was null!\n");
				printf("## logfiles: ");
				for_each_pidslot(i)
					printf("%p ", shm->logfiles[i]);
				printf("\n");
				exit(EXIT_FAILURE);
			}
			ret = fileno(file);
			if (ret == -1) {
				BUG("fileno failed!");
				printf("%s", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}


		if (fd <= ret)
			goto retry_file;
		break;

	case 1:
		/* When using victim files, sockets can be 0.
		 * Use files as a fallback, or pipes if no files are open.
		 */
		if (nr_sockets == 0) {
			if (nr_file_fds > 0)
				goto retry_file;
			else
				goto do_pipe;
		}
		fd = shm->socket_fds[rand() % nr_sockets];
		break;

	case 2:
do_pipe:
		fd = shm->pipe_fds[rand() % MAX_PIPE_FDS];
		break;
	case 3:
		fd = shm->fanotify_fds[rand() % NR_FANOTIFY_FDS];
		break;
	default:
		break;
	}

	return fd;
}

int get_random_fd(void)
{
	/* 25% chance of returning something new. */
	if ((rand() % 4) == 0)
		return get_new_random_fd();

	/* the rest of the time, return the same fd as last time. */
regen:
	if (shm->fd_lifetime == 0) {
		shm->current_fd = get_new_random_fd();
		shm->fd_lifetime = (rand() % shm->max_children) + 5;
	} else
		shm->fd_lifetime--;

	if (shm->current_fd == 0) {
		shm->fd_lifetime = 0;
		goto regen;
	}

	return shm->current_fd;
}

void setup_fds(void)
{
	open_sockets();
	if (no_files == TRUE)
		return;

	open_pipes();
	open_fanotify_fds();

	generate_filelist();
	if (files_in_index == 0)
		return;

	open_files();
}

void regenerate_fds(void)
{
	if (no_files == TRUE)
		return;

	close_files();
	open_files();
}
