#!/bin/bash

build()
{

	# Config

	add_full_dir /etc/boot-encryption

	# GPG

	add_binary gpg
	add_binary gpg-connect-agent
	add_binary gpg-agent
	add_binary /usr/lib/gnupg/scdaemon
	add_binary pinentry-tty

	pinentry_tmp="$(mktemp)"
	cat <<'EOF' > "$pinentry_tmp"
#!/bin/sh

set -eu

exec pinentry-tty --ttyname=/dev/console "$@"
EOF
	chmod +x "$pinentry_tmp"
	add_binary "$pinentry_tmp" /usr/bin/pinentry
	rm "$pinentry_tmp"

	# Main Script

	add_binary basename
	add_binary find
	add_binary boot-encryption

	add_runscript
}
