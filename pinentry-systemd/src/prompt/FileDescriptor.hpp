#ifndef INC_8C89C7F19A49477CB72B839C71E58DE3_HPP
#define INC_8C89C7F19A49477CB72B839C71E58DE3_HPP

#include <stdexcept>

class FileDescriptor
{
public:
	explicit FileDescriptor(int fd);

	~FileDescriptor();

	void close() noexcept;

	[[nodiscard]] int get() const
	{
		if (m_fd < 0)
			throw std::runtime_error{"Invalid file descriptor."};
		return m_fd;
	}

	operator int() const
	{
		return get();
	}

private:
	int m_fd;
};

#endif // INC_8C89C7F19A49477CB72B839C71E58DE3_HPP
