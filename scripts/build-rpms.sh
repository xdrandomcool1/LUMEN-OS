#!/bin/bash
# Builds LUMEN OS RPMs from the current git HEAD into out/rpms and creates
# a dnf repository there (used by the ISO build).
set -euo pipefail
cd "$(dirname "$0")/.."
if [ ! -f /.dockerenv ] && [ ! -f /run/.containerenv ]; then
    exec scripts/dev.sh scripts/build-rpms.sh "$@"
fi
VER=$(rpmspec -q --qf '%{version}\n' packaging/lumen-os.spec 2>/dev/null | head -1)
TOP=/tmp/rpmbuild
rm -rf "$TOP"; mkdir -p "$TOP"/{SOURCES,SPECS,BUILD,RPMS,SRPMS}
git config --global --add safe.directory /src
# Include committed sources plus the vendored Cargo.lock.
git archive --format=tar.gz --prefix="lumen-os-$VER/" -o "$TOP/SOURCES/lumen-os-$VER.tar.gz" HEAD
cp packaging/lumen-os.spec "$TOP/SPECS/"
rpmbuild -bb --define "_topdir $TOP" "$TOP/SPECS/lumen-os.spec"
mkdir -p out/repo
find "$TOP/RPMS" -name '*.rpm' -exec cp {} out/repo/ \;
createrepo_c -q out/repo
ls -la out/repo/*.rpm
