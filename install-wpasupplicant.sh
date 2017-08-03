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
    DEPS=("gcc" "git" "dh_autoreconf" "pkg-config" "bison" "flex")
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
                "gcc")
                    _info "On Debian/Ubuntu: apt-get install build-essential"
                    prompt_installer build-essential
                    ;;
                "git")
                    _info "On Debian/Ubuntu: apt-get install git"
                    prompt_installer git
                    ;;
                "dh_autoreconf")
                    _info "On Debian/Ubuntu: apt-get install dh-autoreconf"
                    prompt_installer dh-autoreconf
                    ;;
                "pkg-config")
                    _info "On Debian/Ubuntu: apt-get install pkg-config"
                    prompt_installer pkg-config
                    ;;
                "bison")
                    _info "On Debian/Ubuntu: apt-get install bison"
                    prompt_installer bison
                    ;;
                "flex")
                    _info "On Debian/Ubuntu: apt-get install flex"
                    prompt_installer flex
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
function check_package {
    found=$(dpkg -l | grep -c $1)

    if [ $found -eq 0 ]; then
        _fail "Missing library: $1"
        case $1 in
            "libssl-dev")
                _info "On Debian/Ubuntu: apt-get install libssl-dev"
                prompt_installer libssl-dev
            ;;
            "openvswitch-switch")
                _info "On Debian/Ubuntu: apt-get install openvswitch-switch"
                prompt_installer openvswitch-switch
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

#if [ "$0" == "$BASH_SOURCE" ]; then
#    _fail "Please source the script -> source install-hostapd.sh"
#    exit 1
#fi

KERNEL=$(uname -r | cut -d "." -f 1,2,3 | cut -d "-" -f 1)
KERNEL_MAJOR=$(uname -r | cut -d "." -f 1)
KERNEL_MINOR=$(uname -r | cut -d "." -f 2)

if [[ $KERNEL_MAJOR -lt 4 || $KERNEL_MAJOR -eq 4 && $KERNEL_MINOR -lt 8 ]]; then
    _fail "You are running kernel version $KERNEL. Please update to version >= 4.8.0"
else
    ROOTDIR=$(pwd)
    _info "Check dependencies"
    check_dependencies
    check_package libssl-dev

    if [ ! -d "/opt/libnl" ]; then
        _info "Install libnl to /opt/libnl"

        cd $HOME
        git clone https://github.com/thom311/libnl/

        cd libnl
        ./autogen.sh
        ./configure --prefix=/opt/libnl --disable-static
        make
        sudo make install

        echo "/opt/libnl/lib" | sudo tee /etc/ld.so.conf.d/libnl.conf
        sudo ldconfig

        cd $ROOTDIR
    else
        _warn "Directory /opt/libnl already exists"
    fi

    _info "Install wpa_supplicant to /usr/local/bin/wpa_supplicant"

    sudo cp /usr/src/linux-headers-$(uname -r)/include/uapi/linux/if_macsec.h /usr/include/linux/if_macsec.h
    export PKG_CONFIG_PATH=/opt/libnl/lib/pkgconfig

    cd wpa_supplicant
    make

    if [ $? -eq 0 ]; then
        sudo cp wpa_supplicant /usr/local/bin/wpa_supplicant
        make clean
        _info "Setup completed"
    else
        make clean
        _fail "Build failed"
    fi

    cd $ROOTDIR
fi