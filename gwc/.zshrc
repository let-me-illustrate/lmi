# Personal configuration for cross-building in a chroot.

export WINEPATH='Z:\\opt\\lmi\\local\\bin;Z:\\opt\\lmi\\local\\lib'
export LMI_HOST=i686-w64-mingw32
export PATH="/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH"
# At a regular user prompt, outside the chroot, do this:
#   $ echo $DISPLAY
# and replace :0.0 below with the string it returns:
export DISPLAY=":0.0"

export coefficiency='--jobs=32'

# export TZ=UCT
export LANG=en_US.UTF-8 LC_TIME=en_DK.UTF-8 LC_COLLATE=C.UTF-8
# It is generally preferable to do this:
# update-locale LANG=en_US.UTF-8 LC_TIME=en_DK.UTF-8 LC_COLLATE=C.UTF-8
# but neither the chroot's nor the host's /etc/default/locale is
# sourced by schroot, which strives to set as few environment
# variables as possible.

# This ought to be unnecessary because $LANG contains the string
# "UTF-8"--see:
#   https://lists.nongnu.org/archive/html/lmi/2018-03/msg00049.html
# but it seems to be necessary nevertheless.
export LESSCHARSET=utf-8

# bindkey "\e[3~" delete-char      # Del
# bindkey '\e[H' beginning-of-line # Home
# bindkey '\e[F' end-of-line       # End
bindkey "^[[1;5D" backward-word  # Ctrl-left
bindkey "^[[1;5C" forward-word   # Ctrl-right
bindkey '\e[1;3D' backward-word  # Alt-left
bindkey '\e[1;3C' forward-word   # Alt-right
# By default, zsh unfortunately binds ^S for this purpose;
# use ^T instead, leaving ^S for flow control.
bindkey '^T' history-incremental-search-forward

prompt='%d[%?]%(!.#.$)'

HISTSIZE=1000
SAVEHIST=1000
HISTFILE=~/.history
setopt HIST_IGNORE_DUPS
setopt HIST_IGNORE_SPACE

autoload -U compinit
compinit -u

# This alias is explained in 'tabs/konsole_tabs'.
alias asdf='. ./startup_script'
