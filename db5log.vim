" :so $VIMRUNTIME/syntax/db5log.vim

" augroup filetypedetect
"	 au! BufRead,BufNewFile db5fuse.log set filetype=db5log
" augroup END 

if version < 600
	syntax clear
elseif exists("b:current_syntax")
	finish
endif

syn match db5log_string		/'[^']\+'/

syn keyword db5log_lev_debug	debug	contained
syn keyword db5log_lev_info	info	contained
syn keyword db5log_lev_notice	notice	contained
syn keyword db5log_lev_warn	war	contained
syn keyword db5log_lev_error	error	contained
syn keyword db5log_lev_critic	critic	contained

syn match db5log_content	/.*$/		contains=db5log_string
syn match db5log_col		/:/		nextgroup=db5log_content
syn match db5log_level		/[^:]\+/	nextgroup=db5log_col contains=db5log_lev_debug,db5log_lev_info,db5log_lev_notice,db5log_lev_warn,db5log_lev_error,db5log_lev_critic
syn match db5log_dot		/\./		nextgroup=db5log_level
syn match db5log_context	/[^.\]]\+/		nextgroup=db5log_dot
syn match db5log_app		/^\s*\[[^\]]\+\]/		nextgroup=db5log_context



if !exists("did_db5log_syntax_inits")
	let did_db5log_syntax_inits = 1
	hi link db5log_app	title
	hi link db5log_context	type
	"hi link db5log_dot	statement
	hi link db5log_level	statement
	"hi link db5log_col	statement
	hi link db5log_content 	comment

	hi link db5log_string	string

	hi db5log_lev_debug	ctermfg=0
	hi db5log_lev_info	ctermfg=5
	hi db5log_lev_notice	ctermfg=3
	hi db5log_lev_warn	ctermfg=2
	hi db5log_lev_error	ctermfg=1
	hi link db5log_lev_critic	error
endif

let b:current_syntax="db5log"

