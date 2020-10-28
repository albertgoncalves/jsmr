" $ cp jb.vim ~/.vim/syntax/jb.vim
" $ grep '.jb' ~/.vimrc
" autocmd BufNewFile,BufRead *.jb setlocal filetype=jb

if exists("b:current_syntax")
    finish
endif

syn match Comment ";.*$"
syn match Number "[0-9]\+"
syn match Number "0x[0-9A-Fa-f]\+"
syn keyword Keyword
    \ minor_version
    \ major_version
    \ this_class
    \ super_class
    \ access_flags
    \ constants
    \ method
    \ name_index
    \ descriptor_index
    \ code
