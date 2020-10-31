" $ cp jb.vim ~/.vim/syntax/jb.vim
" $ grep '.jb' ~/.vimrc
" autocmd BufNewFile,BufRead *.jb setlocal filetype=jb

if exists("b:current_syntax")
    finish
endif

syn match Comment ";.*$"
syn match Number "\-\?[0-9]\+"
syn match Number "\-\?0x[0-9A-Fa-f]\+"
syn match String "\"[^\"]*\"\?"
syn match Function "\.[^ ]\+"

syn keyword Flags
    \ ABSTRACT
    \ ANNOTATION
    \ ENUM
    \ FINAL
    \ INTERFACE
    \ MODULE
    \ PUBLIC
    \ STATIC
    \ SUPER
    \ SYNTHETIC

hi link Flags Statement

syn keyword Constants
    \ class
    \ field_ref
    \ method_ref
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
syn keyword CodeFields
    \ max_local
    \ max_stack
syn keyword MethodFields
    \ code
    \ name_index
    \ type_index

hi link CodeFields Keyword
hi link Constants Keyword
hi link Fields Keyword
hi link MethodFields Keyword
