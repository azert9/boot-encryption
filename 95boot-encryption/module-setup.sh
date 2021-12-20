#!/bin/bash

check() {
	# always include this module by default
	return 0
}

install() {

	# Config

	find /etc/boot-encryption -type f -print0 | while IFS= read -r -d $'\0' file
	do
		inst "$file"
	done

	# Main Script

	inst basename
	inst find
	inst "$moddir/boot-encryption" /usr/bin/boot-encryption

	# Main Service

	inst "$moddir/boot-encryption.service" /etc/systemd/system/boot-encryption.service

	find "$initdir/etc/boot-encryption/cryptsetup" -type f -name "*.key.gpg" -print0 | while IFS= read -r -d $'\0' file
	do
		volume="$(basename --suffix=.key.gpg "$file")"
		sed -i "s/# insert point: Unit/Before=systemd-cryptsetup@$volume.service\n\0/g" "$initdir"/etc/systemd/system/boot-encryption.service
		sed -i "s/# insert point: Install/WantedBy=systemd-cryptsetup@$volume.service\n\0/g" "$initdir"/etc/systemd/system/boot-encryption.service
	done

	systemctl --quiet --root="$initdir" enable boot-encryption.service

	# Pinentry

	inst pinentry-systemd /usr/bin/pinentry

	# GnuPG

	inst gpg
	inst gpg-connect-agent
	inst gpg-agent
	inst /usr/lib/gnupg/scdaemon
}
