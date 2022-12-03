# bash completion for augtool                               -*- shell-script -*-

_augmatch()
{
    local cur prev words cword
    _init_completion || return

    case $prev in
        --help | -!(-*)[h])
            return
            ;;
        --lens | -!(-*)[l])
            local lenses
            lenses="$(augtool --noload 'match /augeas/load/*/lens' | sed -e 's/.*@//')"
					  COMPREPLY=($(compgen -W "${lenses,,}" -- "${cur,,}"))
            return
						;;
        --root | --include | -!(-*)[rI])
            _filedir -d
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
    complete -F _augmatch augmatch

# ex: filetype=sh
