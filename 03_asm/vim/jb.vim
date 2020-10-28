" $ cp jb.vim ~/.vim/syntax/jb.vim
" $ grep '.jb' ~/.vimrc
" autocmd BufNewFile,BufRead *.jb setlocal filetype=jb

if exists("b:current_syntax")
    finish
endif

syn match Comment ";.*$"
syn match Number "\-\?[0-9]\+"
syn match Number "\-\?0x[0-9A-Fa-f]\+"
syn keyword Keyword
    \ access_flags
    \ code
    \ constants
    \ descriptor_index
    \ major_version
    \ method
    \ minor_version
    \ name_index
    \ super_class
    \ this_class
