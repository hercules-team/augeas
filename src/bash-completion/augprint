# bash completion for augtool                               -*- shell-script -*-

_augprint()
{
    local cur prev words cword
    _init_completion || return

    case $prev in
        --help | --version | -!(-*)[hV])
            return
            ;;
        --target | -!(-*)[t])
            _filedir
            return
            ;;
        --lens | -!(-*)[l])
            local lenses
            lenses="$(augtool --noload 'match /augeas/load/*/lens' | sed -e 's/.*@//')"
					  COMPREPLY=($(compgen -W "${lenses,,}" -- "${cur,,}"))
            return
						;;
    esac

    if [[ "$cur" == -* ]]; then
        local opts="$(_parse_help "$1")"
        COMPREPLY=($(compgen -W '${opts:-$(_parse_help "$1")}' -- "$cur"))
		else
        _filedir
		fi
} &&
    complete -F _augprint augprint

# ex: filetype=sh
