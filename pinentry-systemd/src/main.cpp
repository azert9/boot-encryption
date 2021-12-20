#include <string>
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <vector>

#include <assuan.h>
#include "prompt/PinPrompter.hpp"

// TODO: secure memory

static constexpr const char* FLAVOR = "custom-systemd";
static constexpr const char* VERSION = "1.0.0";


struct PromptInfo
{
	std::string description;
	std::string prompt;
};


class Fail
{
public:
	explicit Fail(std::string msg):
		m_msg(std::move(msg))
	{}

	void raise() const
	{
		throw std::runtime_error{m_msg};
	}

private:
	std::string m_msg;
};

void operator||(gpg_error_t error, const Fail& fail)
{
	if (error)
		fail.raise();
}


class AssuanContext
{
public:
	AssuanContext()
	{
		assuan_new(&m_ctx) || Fail{"Failed to create Assuan context."};
	}

	AssuanContext(const AssuanContext&) = delete;

	AssuanContext& operator=(const AssuanContext&) = delete;

	~AssuanContext()
	{
		assuan_release(m_ctx);
	}

	operator assuan_context_t&()
	{
		return m_ctx;
	}

private:
	assuan_context_t m_ctx;
};


gpg_error_t handle_cmd_getinfo(assuan_context_t ctx, char* line)
{
	// TODO

	std::string key{line};

	std::string value;

	if (key == "version")
		value = VERSION;
	else if (key == "pid")
		value = std::to_string(getpid());
	else if (key == "flavor")
		value = FLAVOR;
	else
		return gpg_error(GPG_ERR_ASS_PARAMETER);

	return assuan_send_data(ctx, value.c_str(), value.size());
}


gpg_error_t handle_cmd_setkeyinfo(assuan_context_t ctx, char* line)
{
	// Used for caching, but we don't do caching.
	return 0;
}


gpg_error_t handle_cmd_setdesc(assuan_context_t ctx, char* line)
{
	static_cast<PromptInfo*>(assuan_get_pointer(ctx))->description = line;
	return 0;
}


gpg_error_t handle_cmd_setprompt(assuan_context_t ctx, char* line)
{
	static_cast<PromptInfo*>(assuan_get_pointer(ctx))->prompt = line;
	return 0;
}


gpg_error_t handle_cmd_getpin(assuan_context_t ctx, char*)
{
	try
	{
		PinPrompter prompter;

		std::string pin_code = prompter.prompt();

		if (pin_code.empty())
			return gpg_error(GPG_ERR_CANCELED);

		return assuan_send_data(ctx, pin_code.c_str(), pin_code.size());
	}
	catch (std::runtime_error& e)
	{
		return gpg_error(GPG_ERR_LOCALE_PROBLEM);
	}
}


struct CommandHandler
{
	std::string command;

	gpg_error_t (* handler)(assuan_context_t, char*);
};


static std::vector<CommandHandler> s_command_handlers{
	{"GETINFO",    handle_cmd_getinfo},
	{"SETKEYINFO", handle_cmd_setkeyinfo},
	{"SETPROMPT",  handle_cmd_setprompt},
	{"SETDESC",    handle_cmd_setdesc},
	{"GETPIN",     handle_cmd_getpin},
};


void pinentry()
{
	// initializing gpg-error
	gpgrt_check_version(nullptr);

	AssuanContext ctx;

	assuan_fd_t io_files[2];
	io_files[0] = assuan_fdopen(STDIN_FILENO);
	io_files[1] = assuan_fdopen(STDOUT_FILENO);
	assuan_init_pipe_server(ctx, io_files) || Fail{"Failed to get pipes"};

	for (const CommandHandler& command_handler: s_command_handlers)
	{
		assuan_register_command(
			ctx,
			command_handler.command.c_str(),
			command_handler.handler,
			nullptr
		) || Fail{"Failed to register command."};
	}

	PromptInfo prompt_info{};
	assuan_set_pointer(ctx, &prompt_info);

	for (;;)
	{
		gpg_error_t error = assuan_accept(ctx);
		if (error == -1)
			break;
		error || Fail{"Error while receiving request."};

		assuan_process(ctx) || Fail{"Error while processing request."};
	}
}


int main()
{
	try
	{
		pinentry();
		return EXIT_SUCCESS;
	}
	catch (std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
