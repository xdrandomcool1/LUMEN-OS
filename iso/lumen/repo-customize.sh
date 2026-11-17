#!/bin/sh
# kiwi repository customize hook: $1 is the generated .repo file.
# When the build environment uses extra trust anchors (TLS-intercepting
# proxy, see scripts/devcontainer/extra-ca), point dnf at the build host's
# CA bundle. Nothing is written into the image itself.
if ls /extra-ca/*.crt >/dev/null 2>&1; then
    # dnf resolves this path inside the image root; build-iso.sh seeds it
    # there and config.sh deletes it before the image is sealed.
    echo "sslcacert=/var/tmp/lumen-build-ca.crt" >> "$1"
fi
