#!/bin/bash

# -----------------------------------------------------------------------------
# logging helpers
# -----------------------------------------------------------------------------

function _log {
    level=$1
    msg=$2

    case "$level" in
        info)
            tag="\e[1;36minfo\e[0m"
            ;;
        err)
            tag="\e[1;31merr \e[0m"
            ;;
        warn)
            tag="\e[1;33mwarn\e[0m"
            ;;
        ok)
            tag="\e[1;32m ok \e[0m"
            ;;
        fail)
            tag="\e[1;31mfail\e[0m"
            ;;
        *)
            tag="    "
            ;;
    esac
    echo -e "`date +%Y-%m-%dT%H:%M:%S` [$tag] $msg"
}

function _err {
    msg=$1
    _log "err" "$msg"
}

function _warn {
    msg=$1
    _log "warn" "$msg"
}

function _info {
    msg=$1
    _log "info" "$msg"
}

function _success {
    msg=$1
    _log "ok" "$msg"
}

function _fail {
    msg=$1
    _log "fail" "$msg"
}

# -----------------------------------------------------------------------------
# Checking dependencies
# -----------------------------------------------------------------------------
function check_dependencies {
    IFS=':' read -a PATHS <<< "$PATH"
    DEPS=("tar" "wget" "gcc" "git" )
    for d in ${DEPS[@]}; do
        _info "Looking for '$d'"
        found=0
        for p in ${PATHS[@]}; do
            if [ -f "$p/$d" ]; then
                _success "Found '$d' in $p/$d"
                found=1
            fi
        done
        if [ $found -eq 0 ]; then
            _fail "Missing dependency: $d"
            case $d in
                "tar")
                    _info "On Debian/Ubuntu: apt-get install tar"
                    prompt_installer tar
                    ;;
                "wget")
                    _info "On Debian/Ubuntu: apt-get install wget"
                    prompt_installer wget
                    ;;
                "gcc")
                    _info "On Debian/Ubuntu: apt-get install build-essential"
                    prompt_installer build-essential
                    ;;
                "git")
                    _info "On Debian/Ubuntu: apt-get install git"
                    prompt_installer git
                    ;;
                *)
                    _info "No suggestions available. Good luck... :-)"
                    ;;
            esac
        fi
    done
}

# -----------------------------------------------------------------------------
# Checking library
# -----------------------------------------------------------------------------
function check_library {
    found=$(dpkg -l | grep -c $1)

    if [ $found -eq 0 ]; then
        _fail "Missing library: $1"
        case $1 in
            "libtalloc-dev")
                _info "On Debian/Ubuntu: apt-get install libtalloc-dev"
                prompt_installer libtalloc-dev
            ;;
            "libkqueue-dev")
                _info "On Debian/Ubuntu: apt-get install libkqueue-dev"
                prompt_installer libkqueue-dev
            ;;
            "libldap2-dev")
                _info "On Debian/Ubuntu: apt-get install libldap2-dev"
                prompt_installer libldap2-dev
            ;;

        esac
    fi
}

# -----------------------------------------------------------------------------
# Prompt installer
# -----------------------------------------------------------------------------
function prompt_installer {
    read -p "Do you want to install it? [Y/n]: " -n 10 CHOICE

    case $CHOICE in
        y|Y|yes|YES|"")
            sudo apt-get install $1
            ;;
        *)
            _info "Ok, install it yourself :)"
            exit 1
            ;;
    esac
}

# -----------------------------------------------------------------------------
# Main
# -----------------------------------------------------------------------------

_info "Check dependencies"
check_dependencies
check_library libtalloc-dev
check_library libkqueue-dev
check_library libldap2-dev


# OpenSSL
_info "Install OpenSSL (1.1.0f) in /opt/openssl-1.1.0f"

wget https://www.openssl.org/source/openssl-1.1.0f.tar.gz

tar -xzvf openssl-1.1.0f.tar.gz

sudo mkdir /opt/openssl-1.1.0f
cd openssl-1.1.0f
./config --prefix=/opt/openssl-1.1.0f --openssldir=/usr/local/ssl
make
make test
sudo make install
make clean

echo "/opt/openssl-1.1.0f/lib" | sudo tee /etc/ld.so.conf.d/libssl.conf
sudo ldconfig

cd ..

# FreeRADIUS
_info "Install FreeRADIUS server"

git clone https://github.com/alessandrotrisolini/freeradius-server

cd freeradius-server

./configure \
    --with-openssl \
    --with-openssl-lib-dir=/opt/openssl-1.1.0f/lib/ \
    --with-openssl-include-dir=/opt/openssl-1.1.0f/include/ \
    --with-dhcp

echo "\$INCLUDE dictionary.dhcp" >> share/dictionary
make
sudo make install
make clean

cd ..

_info "Setup finished"