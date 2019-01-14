" FormulaAI syn file
" Language: Formula AI
" Maintainer: barbarianhero
" Last Change:	2008 Mar 26

" Quit if syntax file is already loaded
if version < 600
   syntax clear
elseif exists("b:current_syntax")
   finish
endif

syntax case ignore

syn region  formula_string			start=/'/ skip=/\\'/ end=/'/
syn region  formula_comment			start=/{/ end=/}/
syn keyword formula_keyword			def functions where
syn keyword formula_conditional 	if switch
syn keyword formula_function		abs choose dir filter find head
syn keyword formula_function 		map	max min set_var sort sum
syn match   formula_function 		"\bsize\b"
syn keyword formula_function_ai		attack chance_to_hit distance_between
syn keyword formula_function_ai 	distance_to_nearest_unowned_village
syn keyword formula_function_ai 	defense_on evaluate_for_position fallback
syn keyword formula_function_ai 	is_village loc max_possible_damage
syn keyword formula_function_ai 	move recruit set_var unit_at
syn keyword formula_function_ai 	unit_moves units_can_reach

hi def link formula_keyword 	Statement
hi def link formula_conditional Conditional
hi def link formula_function 	Statement
hi def link formula_function_ai Type
hi def link formula_comment 	Comment
hi def link formula_string 		String

let b:current_syntax = "FormulaAI"
