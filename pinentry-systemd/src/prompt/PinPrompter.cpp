#include "PinPrompter.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>
#include <cstring>
#include "FileDescriptor.hpp"


std::string PinPrompter::prompt()
{
	int fds[2];
	if (::pipe(fds) != 0)
		throw std::runtime_error{"Failed to open pipe."};

	FileDescriptor read_fd{fds[0]};
	FileDescriptor write_fd{fds[1]};


	pid_t pid = ::fork();

	if (pid == 0)
	{
		read_fd.close();
		::dup2(write_fd, STDOUT_FILENO);
		write_fd.close();

		// TODO: allow TTY input by providing a fd to the tty when possible
		// TODO: other options (timeout, ...)
		::execlp("systemd-ask-password", "systemd-ask-password", "--no-tty", "--", nullptr);
		throw std::runtime_error{std::string("Exec failed: ") + strerror(errno) + "."};
	}
	else
	{
		write_fd.close();

		char buff[4096];  // TODO
		ssize_t rd = ::read(read_fd, buff, sizeof(buff));
		if (rd < 0)
			throw std::runtime_error{"Read failed."};

		// TODO
		::waitpid(pid, nullptr, 0);

		while (rd > 0 && buff[rd - 1] == '\n')
			--rd;

		return std::string(buff, rd);
	}
}
