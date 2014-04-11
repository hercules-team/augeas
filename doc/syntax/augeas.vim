" Vim syntax file
" Language:	Augeas
" Version: 1.0
" $Id$
" Maintainer:  Bruno Cornec <bruno@project-builder.org>
" Contributors:
"  Raphaël Pinson <raphink@gmail.com>

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif


syn case ignore
syn sync lines=250

syn keyword augeasStatement	module let incl transform autoload in rec
syn keyword augeasTestStatement	test get after put insa insb set rm
syn keyword augeasTodo contained	TODO FIXME XXX DEBUG NOTE

if exists("augeas_symbol_operator")
  syn match   augeasSymbolOperator      "[+\-/*=]"
  syn match   augeasSymbolOperator      "[<>]=\="
  syn match   augeasSymbolOperator      "<>"
  syn match   augeasSymbolOperator      ":="
  syn match   augeasSymbolOperator      "[()]"
  syn match   augeasSymbolOperator      "\.\."
  syn match   augeasSymbolOperator      "[\^.]"
  syn match   augeasMatrixDelimiter	"[][]"
  "if you prefer you can highlight the range
  "syn match  augeasMatrixDelimiter	"[\d\+\.\.\d\+]"
endif

if exists("augeas_no_tabs")
  syn match augeasShowTab "\t"
endif

syn region augeasComment	start="(\*"  end="\*)" contains=augeasTodo,augeasSpaceError


if !exists("augeas_no_functions")
  " functions
  syn keyword augeasLabel	del key store label value square seq
  syn keyword augeasFunction	Util Build Rx Sep Quote
endif

syn region  augeasRegexp  start="/"  end="[^\\]/"
syn region  augeasString  start=+"+  end=+"+  skip=+\\"+


" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_augeas_syn_inits")
  if version < 508
    let did_augeas_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink augeasAcces		augeasStatement
  HiLink augeasBoolean		Boolean
  HiLink augeasComment		Comment
  HiLink augeasConditional	Conditional
  HiLink augeasConstant		Constant
  HiLink augeasDelimiter	Identifier
  HiLink augeasDirective	augeasStatement
  HiLink augeasException	Exception
  HiLink augeasFloat		Float
  HiLink augeasFunction		Function
  HiLink augeasLabel		Label
  HiLink augeasMatrixDelimiter	Identifier
  HiLink augeasModifier		Type
  HiLink augeasNumber		Number
  HiLink augeasOperator		Operator
  HiLink augeasPredefined	augeasStatement
  HiLink augeasPreProc		PreProc
  HiLink augeasRepeat		Repeat
  HiLink augeasSpaceError	Error
  HiLink augeasStatement	Statement
  HiLink augeasString		String
  HiLink augeasStringEscape	Special
  HiLink augeasStringEscapeGPC	Special
  HiLink augeasRegexp		Special
  HiLink augeasStringError	Error
  HiLink augeasStruct		augeasStatement
  HiLink augeasSymbolOperator	augeasOperator
  HiLink augeasTestStatement	augeasStatement
  HiLink augeasTodo		Todo
  HiLink augeasType		Type
  HiLink augeasUnclassified	augeasStatement
  "  HiLink augeasAsm		Assembler
  HiLink augeasError		Error
  HiLink augeasAsmKey		augeasStatement
  HiLink augeasShowTab		Error

  delcommand HiLink
endif


let b:current_syntax = "augeas"

" vim: ts=8 sw=2
