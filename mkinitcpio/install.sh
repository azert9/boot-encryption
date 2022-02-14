#!/usr/bin/ash

build {

	# Config

	add_full_dir /etc/boot-encryption

	# GPG

	add_binary gpg
	add_binary gpg-connect-agent
	add_binary gpg-agent
	add_binary /usr/lib/gnupg/scdaemon
	add_binary pinentry-tty

	cat <<'EOF' > "$BUILDROOT/usr/bin/pinentry"
#!/bin/sh

set -eu

exec pinentry-tty --ttyname=/dev/console "$@"
EOF
	chmod +x /usr/bin/pinentry

	# Main Script

	add_binary basename
	add_binary find
	add_binary boot-encyption

	add_runscript
}
