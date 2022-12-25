" Vim syntax file
" Language: masml (My ASM Language)
" Maintainer: Richard Si
" Latest Revision: December 24, 2022

" Referenced resources:
" - https://superuser.com/questions/844004/creating-a-simple-vim-syntax-highlighting
" - https://vim.fandom.com/wiki/Creating_your_own_syntax_files
" - https://devhints.io/vimscript
" - https://vi.stackexchange.com/questions/9416/how-do-you-find-where-a-variable-was-last-assigned
" - https://stackoverflow.com/questions/19079755/how-to-set-an-option-using-a-string-in-a-vim-script
" - https://vimdoc.sourceforge.net/htmldoc/options.html#'isfname'
" - https://vimdoc.sourceforge.net/htmldoc/options.html#'iskeyword'
" - https://vimdoc.sourceforge.net/htmldoc/pattern.html#pattern
" - https://vimdoc.sourceforge.net/htmldoc/syntax.html (most useful!)

if exists("b:current_syntax") && get(b:, "current_syntax") !=# "conf"
  finish
endif

" Since instruction names can contain dashes, Vim should treat - as a keyword
" character
set iskeyword+=-

syn keyword CommentTodo contained TODO FIXME XXX
syn keyword CmdType LOAD STORE
syn keyword CmdType SET-REGISTER SWAP
syn keyword CmdType ADD SUBTRACT MULTIPLY DIVIDE MODULO
syn keyword CmdType EQUAL NOT
syn keyword CmdType GOTO GOTO-IF GOTO-IF-NOT EXIT
syn keyword CmdType PRINT

syn match   Comment      "^#.*" contains=CommentTodo
syn match   Register     "\$1\|\$2"
syn match   Number       "[-+]\?\d\+"
syn match   Number       "[-+]\?\d*\.\d*"
syn match   Variable     "&\S\+"

hi def link Comment      Comment
hi def link CommentTodo  Todo
hi def link CmdType      Statement
hi def link Register     Special
hi def link Number       Comment
hi def link Variable     Identifier

let b:current_syntax = "masml"
