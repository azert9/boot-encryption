# Unlocking a Root Partition Using a YubiKey and GPG

If you are using an encrypted system, a YubiKey can be a great way to conveniently unlock it at bootup.
This is often implemented with a HMAC challenge-response. You can also add a passphrase for two-factor security, at the
expense of convenience.

But what if you used the GPG application instead of HMAC? Using the method detailed in this guide, you will be
able to replace a long passphrase by the PIN code of your YubiKey.

The steps below assume this configuration:
* Operating System: Arch Linux
* Initramfs: mkinitcpio or Dracut
* Key Model: YubiKey 5

If your configuration differ, you will have to adapt accordingly. I may also expand this guide in the future.

Obligatory disclaimer: backup your files, make sure you understand your system enough so you can repair it if it is broken, and I am not responsible for any damage. 

## Prerequists

* You have basic knowledge of GPG, dm-crypt, and the Linux boot process.
* You are using mkinitcpio or Dracut.
* You have one or more encrypted paritions, which are decrypted early at boot.
* You have a YubiKey with a GPG private key (tested with the YubiKey 5).

## Mkinitcpio

This section is for mkinitcpio only.

### Installing the mkinitcpio hook

Copy the mkinitcpio hook files:

```sh
sudo cp ./mkinitcpio/hook.sh /etc/initcpio/hooks/gpg-encrypt
sudo cp ./mkinitcpio/install.sh /etc/initcpio/install/gpg-encrypt
```

Add the `gpg-encrypt` hook to the list of hooks in `/etc/mkinitcpio.conf`. It should
be placed just before `encrypt` :

```
HOOKS=(base ... gpg-encrypt encrypt ...)
```

### Configuring the encrypt hook

Each decrypted key file is saved as `/run/cryptsetup-keys.d/*.key`. 

In order for the encrypt hook to find the key file of your root partition, you must add a kernel parameter:

```
cryptkey=rootfs:/run/cryptsetup-keys.d/$NAME.key
```

Replace `$NAME` by the name of the mapped device, as in `/dev/mapper/$NAME`.

## Dracut

This section is for Dracut only.

### Enabling GPG Pinentry in Intramfs

When using a PIN code, GPG needs to be able to prompt for it. This repository contains a minimal
`pinentry` implementation based on `systemd-ask-password`, which is usually used for LUKS passwords.

First, build it:

```sh
cd pinentry-systemd
mkdir boot
cd boot
cmake ..
make
```

Then, install it:

```sh
sudo cp ./pinentry-systemd /usr/bin/
```

### Configuring Dracut

Simply copy `95boot-encryption/` from this repository to `/usr/lib/dracut/modules.d/`:

```sh
sudo cp -r ./95boot-encryption /usr/lib/dracut/modules.d/
```

## Configuring Your Public Key

Export your GPG public key to a file named `/etc/boot-encryption/public/*.key`:

```sh
sudo mkdir -p /etc/boot-encryption/public/
gpg --export --armor $GPG_KEY_ID | sudo tee /etc/boot-encryption/public/main.key
```

All the keys matching this pattern will be imported before performing decryption.

## Adding a Key File to the LUKS Device

Generate a key file and add it to the LUKS device:

```sh
sudo sh -c '(umask 077 && head -c 256 /dev/urandom > /tmp/keyfile)'
sudo cryptsetup luksAddKey $ENCRYPTED_DEVICE /tmp/keyfile
```

Encrypt the key file with your GPG public key, and save it as `/etc/boot-encryption/cryptsetup/$NAME.key.gpg`.
Replace `$NAME` by the name of the mapped device, as in `/dev/mapper/$NAME`.

```sh
sudo mkdir --mode=0700 /etc/boot-encryption/cryptsetup
sudo sh -c 'umask 077 && touch /etc/boot-encryption/cryptsetup/$NAME.key.gpg'
gpg --encrypt --recipient $GPG_KEY_ID --output - <(sudo cat /tmp/keyfile) \
	| sudo tee > /dev/null /etc/boot-encryption/cryptsetup/$NAME.key.gpg
```

You can then remove the unencrypted key file:

```sh
sudo rm /tmp/keyfile
```

Repeat for every encrypted partition that you want to unlock at boot.

## Regenerating the Initramfs

Your initramfs must be regenerated. Proceed as usual, and you are done!

## Final Words

If you have any suggestion or remark (typo, clarification, ...), feel free to open an issue. I hope this guide has proven useful to you!
