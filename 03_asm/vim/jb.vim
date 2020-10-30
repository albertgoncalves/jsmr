" $ cp jb.vim ~/.vim/syntax/jb.vim
" $ grep '.jb' ~/.vimrc
" autocmd BufNewFile,BufRead *.jb setlocal filetype=jb

if exists("b:current_syntax")
    finish
endif

syn match Comment ";.*$"
syn match Number "\-\?[0-9]\+"
syn match Number "\-\?0x[0-9A-Fa-f]\+"
syn match String "\".*\""

syn keyword Flags
    \ abstract
    \ annotation
    \ enum
    \ final
    \ interface
    \ module
    \ public
    \ super
    \ synthetic

hi link Flags Statement

syn keyword Constants
    \ class
    \ name_and_type
    \ string
syn keyword Fields
    \ access_flags
    \ constants
    \ major_version
    \ method
    \ minor_version
    \ super_class
    \ this_class
syn keyword MethodFields
    \ code
    \ descriptor_index
    \ name_index

hi link Constants Keyword
hi link Fields Keyword
hi link MethodFields Keyword
