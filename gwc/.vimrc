" This shouldn't be necessary, but it is: $LANG is 'en_US.UTF-8',
" yet vim uses Latin-1 nevertheless without this line:
set encoding=utf-8

set shortmess=Il

noremap <S-Up> <C-Y>
noremap <S-Down> <C-E>

inoremap <S-Up>   <C-X><C-Y>
inoremap <S-Down> <C-X><C-E>

" Counterpart to J--on second thought, not such a good idea
" nnoremap K i<Enter><Esc>

" Tab key means ASCII HT (in insert mode)
inoremap <Tab> <C-V><Tab>

" Make 'n' search forward even if the last search was backward;
" similarly, make 'N' always search backward.
nnoremap <expr> n 'Nn'[v:searchforward]
nnoremap <expr> N 'nN'[v:searchforward]

set shiftwidth=4
set expandtab

" Not very helpful--use 'scrolloff' instead
" set cursorline

" Always show at least one line above or below the cursor
set scrolloff=1

set cindent
set cinoptions=t0\ g0\ {s\ (0\ U1 comments^=:///

" By default, vim detects '*.mak' as type make, but not '*.make'
au BufNewFile,BufRead *.make setf make

" Restore last position
au BufReadPost * normal! g`"zv

au BufReadPost,BufNewFile *.tpp set ft=cpp
au BufReadPost,BufNewFile *.xpp set ft=cpp
" au BufReadPost,BufNewFile *.?pp set ft=cpp

" mustache--"ignored" is preferable to default:
"   https://lists.gnu.org/archive/html/lmi/2017-10/msg00016.html
" for a plugin, see:
"   https://lists.gnu.org/archive/html/lmi/2017-10/msg00017.html
au BufReadPost,BufNewFile *.mst set ft=ignored

syntax on
set hlsearch

set spell spelllang=en_us

set wildmode=longest,longest,list:longest,full

set history=1000

set virtualedit=all

set background=dark

" Make comments less prominent
highlight Comment                                ctermfg=39

highlight clear SpellBad
highlight SpellBad   cterm=underline ctermbg=240 ctermfg=226
highlight clear SpellCap
highlight SpellCap   cterm=underline ctermbg=240 ctermfg=252
highlight clear SpellLocal
highlight SpellLocal cterm=underline ctermbg=240 ctermfg=154
highlight clear SpellRare
highlight SpellRare  cterm=underline ctermbg=240 ctermfg=208

highlight TabLineFill                            ctermfg=16
highlight TabLine                    ctermbg=237 ctermfg=124
highlight TabLineSel                 ctermbg=239 ctermfg=202

" From tpope's 'sensible.vim':
set tabpagemax=50

" Always show tabline, even if there's only one tab:
set showtabline=2

" Show invisibles: 'eol' and 'tab' are meant to be unobtrusive,
" while 'nbsp' and 'trail' are meant to stand out; 'extends' and
" 'precedes' normally wouldn't be seen because of 'set wrap'.
set list
" set listchars=nbsp:◙,eol:☜,tab:☞☞,extends:»,precedes:«,trail:▒
" On second thought, 'eol' should probably be left alone, and
" 'shouldered open box' is conventional for nbsp.
set listchars=nbsp:⍽,tab:☞☞,extends:»,precedes:«,trail:▒
highlight clear NonText
highlight NonText    ctermfg=gray " eol, extends, precedes
highlight clear SpecialKey
highlight SpecialKey ctermfg=red  " nbsp, tab, trail

let g:netrw_liststyle=3
" What was the reason for this?
" map <Tab> <C-W>w

" Prevent gvim cursor from blinking
:set guicursor+=a:blinkon0
