# bash completion for augtool                               -*- shell-script -*-

_augtool()
{
    local cur prev words cword
    _init_completion || return

    case $prev in
        --help | --version | -!(-*)[hV])
            return
            ;;
        --load-file | --file | -!(-*)[lf])
            _filedir
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
		fi
} &&
    complete -F _augtool augtool

# ex: filetype=sh
