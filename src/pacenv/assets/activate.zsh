if typeset -f deactivate_%%NAME%% >/dev/null; then
	echo "The '%%NAME%%' is already sourced, or it may be a name conflict."
	return
fi

deactivate_%%NAME%%() {
	PATH=${PATH:s,%%ROOT%%usr/bin:,}
	PS1=${PS1:s,(%%NAME%%) ,}
	unset -f ${funcstack[1]}
}

PATH="%%ROOT%%usr/bin:$PATH"
PS1="(%%NAME%%) $PS1"

# vim: set filetype=zsh ts=2 sw=2 noexpandtab:
