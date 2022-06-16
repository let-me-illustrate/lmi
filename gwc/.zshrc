# Personal configuration for cross-building in a chroot.

# To work with lmi, it is crucial to source 'set_toolchain.sh'. It's
# deliberately not sourced here, so that it can assume that some
# things have already been set up: for instance, it assumes that
#   /usr/share/misc/config.guess
# is present, which need not be the case in a brand-new chroot.

# Minimal system path.
export MINIMAL_PATH="/usr/bin:/bin:/usr/sbin:/sbin"
export PATH="$MINIMAL_PATH"

# At a regular user prompt, outside the chroot, do this:
#   $ echo $DISPLAY
# and replace :0.0 below with the string it returns:
export DISPLAY=":0.0"

umask g=rwx

# Make the X clipboard available to root, for vim only. See:
#   https://lists.nongnu.org/archive/html/lmi/2019-10/msg00000.html
if [ "$(id -u)" -eq 0 ]; then
  alias vim='XAUTHORITY=/home/greg/.Xauthority vim'
fi

# Something like
#  "--jobs=$(nproc || sysctl -n hw.ncpu || getconf _NPROCESSORS_ONLN)"
# could be used instead for other *nix systems:
export coefficiency="--jobs=$(nproc)"

# This should be unnecessary:
# export TZ=UCT

# redhat-based distributions may lack 'C.UTF-8'--see:
#   https://bugzilla.redhat.com/show_bug.cgi?id=902094
if locale -a | grep --quiet C.UTF-8; then
  pref_lc=C.UTF-8; else
  pref_lc=en_US.UTF-8;
fi
export LANG=en_US.UTF-8 LC_ALL=$pref_lc LC_TIME=en_DK.UTF-8 LC_COLLATE=$pref_lc
# Instead of assigning those variables, this is generally preferable:
#   update-locale LANG=en_US.UTF-8 LC_TIME=en_DK.UTF-8 LC_COLLATE=C.UTF-8
# but neither the chroot's nor the host's /etc/default/locale is
# sourced by schroot, which strives to set as few environment
# variables as possible.

# This ought to be unnecessary because $LANG contains the string
# "UTF-8"--see:
#   https://lists.nongnu.org/archive/html/lmi/2018-03/msg00049.html
# but it seems to be necessary nevertheless.
export LESSCHARSET=utf-8

# Avoid "it looks like wine32 is missing, you should install it": see
#   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00016.html
export WINEDEBUG=-all,err+all,fixme+all

# This UBSan option is generally useful.
export UBSAN_OPTIONS=print_stacktrace=1
# These ABSan options are generally useful.
export ASAN_OPTIONS=detect_leaks=0:detect_invalid_pointer_pairs=1:strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1

# Use vim keybindings.
bindkey -v

# Replace the default vim Backspace binding:
bindkey '^?' backward-delete-char # Backspace
# because it is astonishing that 'vi-backward-delete-char'
# doesn't delete past the point where insert mode was entered;
# and bind these explicitly--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-02/msg00008.html
bindkey '\e[3~' delete-char       # Del
bindkey '\e[H' beginning-of-line  # Home
bindkey '\e[F' end-of-line        # End
bindkey '\e[1;5D' backward-word   # Ctrl-left
bindkey '\e[1;5C' forward-word    # Ctrl-right
bindkey '\e[1;3D' backward-word   # Alt-left
bindkey '\e[1;3C' forward-word    # Alt-right

# Bind those all in the 'vicmd' keymap, too:
bindkey -M vicmd '^?' backward-delete-char # Backspace
bindkey -M vicmd '\e[3~' delete-char       # Del
bindkey -M vicmd '\e[H' beginning-of-line  # Home
bindkey -M vicmd '\e[F' end-of-line        # End
bindkey -M vicmd '\e[1;5D' backward-word   # Ctrl-left
bindkey -M vicmd '\e[1;5C' forward-word    # Ctrl-right
bindkey -M vicmd '\e[1;3D' backward-word   # Alt-left
bindkey -M vicmd '\e[1;3C' forward-word    # Alt-right

# Enable useful features that emacs mode binds by default.
# Binding '^R' here doesn't interfere with '^R' (undo) in vicmd mode.
# To reserve '^S' for flow control, prefer '^F' to emacs mode's '^S'.
bindkey '^R' history-incremental-search-backward # emacs Ctrl-R
bindkey '^F' history-incremental-search-forward  # emacs Ctrl-S
bindkey '^G' send-break                          # emacs Ctrl-G
bindkey '\eq' push-line                          # emacs Esc-Q

# Set this preemptively, even though it's reset almost immediately.
# Otherwise, a redhat server initially shows an unwanted default
# prompt when zsh is started.
PS1='%d[%?]%(!.#.$)'

function zle-line-init zle-keymap-select {
    local local_prompt='%d[%?]%(!.#.$)'
    if [[ ${KEYMAP} == vicmd ]]; then
        prompt="%F{yellow}${local_prompt}%f"
    else
        prompt="${local_prompt}"
    fi
    zle reset-prompt
}

zle -N zle-line-init
zle -N zle-keymap-select

HISTSIZE=1000000
SAVEHIST=$HISTSIZE
HISTFILE=~/.history
setopt HIST_IGNORE_DUPS
setopt HIST_IGNORE_SPACE

# Don't kill session if Ctrl-D hit by accident.
setopt IGNORE_EOF

# Get rid of silly redhat defaults like their 1400-char $LS_COLORS.
unset HISTCONTROL
unset KDEDIRS
unset LESSOPEN
unset LS_COLORS
unset MANPATH
unset PERL5LIB
unset PERL_LOCAL_LIB_ROOT
unset PERL_MB_OPT
unset PERL_MM_OPT
unset QTDIR
unset QT_GRAPHICSSYSTEM_CHECKED
unset QTINC
unset QTLIB
unset QT_PLUGIN_PATH
unset SYSTEMDIR

autoload -U compinit
compinit -u

# This alias is explained in 'tabs/konsole_tabs'.
alias asdf='. ./startup_script'
