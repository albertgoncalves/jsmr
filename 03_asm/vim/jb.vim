" $ cp jb.vim ~/.vim/syntax/jb.vim
" $ grep '.jb' ~/.vimrc
" autocmd BufNewFile,BufRead *.jb setlocal filetype=jb

if exists("b:current_syntax")
    finish
endif

syn match Comment ";.*$"
syn match Number "\d\+"
syn match Number "0x\d\+"
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
