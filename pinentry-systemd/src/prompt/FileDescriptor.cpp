#include "FileDescriptor.hpp"

#include <unistd.h>

FileDescriptor::FileDescriptor(int fd):
	m_fd(fd)
{}

FileDescriptor::~FileDescriptor()
{
	close();
}

void FileDescriptor::close() noexcept
{
	if (m_fd >= 0)
		::close(m_fd);
	m_fd = -1;
}
