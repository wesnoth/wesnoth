" Vim syntax file
" Language:		WML (Wesnoth Markup Language)
" Maintainer:	Étienne Simon <etienne.jl.simon@gmail.com>
" URL:			http://wesnoth.org
" Last Change:	26 Feb 2012

" LICENSE (3-clause BSD) :
" Copyright (c) 2012, Étienne Simon
" All rights reserved.
"
" Redistribution and use in source and binary forms, with or without
" modification, are permitted provided that the following conditions are met:
"     * Redistributions of source code must retain the above copyright
"       notice, this list of conditions and the following disclaimer.
"     * Redistributions in binary form must reproduce the above copyright
"       notice, this list of conditions and the following disclaimer in the
"       documentation and/or other materials provided with the distribution.
"     * The name of Étienne Simon may not be used to endorse or promote
"       products derived from this software without specific prior written
"       permission.
"
" THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
" ARE DISCLAIMED. IN NO EVENT SHALL ÉTIENNE SIMON BE LIABLE FOR ANY DIRECT,
" INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
" (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
" LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
" ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
" (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
" THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

" Quit when a syntax file was already loaded
if exists("b:current_syntax")
	finish
endif

" Lua highlighting is not working with the last version of lua.vim.
" You can try your luck by replacing wmllua.vim by lua.vim on the next line.
syn include @luaTop syntax/wmllua.vim

syn case match


"""""""""""""
" Litterals "
"""""""""""""
" A number may be decimal and negative (no space allowed between -, . and \d)
" A string may start and end with a " or with << and >>
" If a string is between double quote, the special sequences "" and $| may be inserted in it
" The boolean constants are yes, no, true and false
syn match wesmlBoolean display /\<\(yes\|no\|true\|false\)\>/ contained
syn match wesmlNumber "-\?\d\+\(\.\d\+\)\?" contained
syn match wesmlQStringEsc display +""+ contained
syn region wesmlQString display start=/"\ze[^"]/ skip=/""/ end=/"/ contains=wesmlQStringEsc,@Spell,wesmlLuaQ,wesmlStringSpecial,wesmlStringVariable,wesmlFormula contained
syn region wesmlCString display start=/<<\@<=/ end=/>>/ contains=@Spell,wesmlLuaC,wesmlStringSpecial,wesmlStringVariable,wesmlFormula contained
syn match wesmlUnderscore display /\<_\>\(\s*\("\|<<\)\)\@=/ contained
syn match wesmlEString display +""+ contained
syn match wesmlStringSpecial display +$|+ contained


"""""""""""
" Comment "
"""""""""""
syn match wesmlComment display /#.*$/ contains=wesmlTodo,wesmlWmlindent
syn keyword wesmlTodo TODO XXX FIXME contained
syn match wesmlWmlindent display /#\@<=wmlindent\>.*/ contained


""""""""""""""""
" Preprocessor "
""""""""""""""""
" Preprocessor directives start with a # except the include directive {}
syn match wesmlPPDArgument display /\s\+.\+$/ contained contains=wesmlComment
syn match wesmlPPDIfverArgument display /\s\+.\+$/ contained contains=wesmlPPDOperator,wesmlVerNumber,wesmlComment
syn match wesmlPPDOperator display "\(==\|!=\|<=\?\|>=\?\)" contained
syn match wesmlVerNumber display "\d\+\(\.\d\+\)*\(-svn\)\?" contained
syn match wesmlPPDNullary display /^\s*\zs#\s*enddef\>/
syn match wesmlPPDNullaryTest display /^\s*\zs#\s*\(else\|endif\)\>/
syn match wesmlPPDNary display /^\s*\zs#\s*\(define\|undef\|textdomain\|line\)\>\s\+\S/ contains=wesmlPPDArgument
syn match wesmlPPDNaryTest display /^\s*\zs#\s*\(ifdef\|ifndef\|ifhave\|ifnhave\)\>\s\+\S/ contains=wesmlPPDArgument
syn match wesmlPPDIfver display /^\s*\zs#\s*\(ifver\|ifnver\)\>\s\+\S/ contains=wesmlPPDIfverArgument
syn match wesmlIncludeMark display +{\@<=\(\~\|\./\|/\)\?+ contained
syn region wesmlInclude display start=/{/ end=/}/ contains=wesmlMacroName,wesmlNumber,wesmlVariable,wesmlIdentifier,wesmlCString,wesmlQString,wesmlEString,wesmlBoolean,wesmlFormula
syn match wesmlMacroName display /{\@<=[^ }]\+/ contained contains=wesmlIncludeMark


"""""""
" Tag "
"""""""
" A tag is of the form : \[\s*[+/]\?\s*\w\+\s*\]
syn match wesmlTag display /\[[^]]\+\]/ contains=wesmlTagMark,wesmlStdTag,wesmlTagError,wesmlGUITag
syn match wesmlTagMark display /\[\s*[+/]\?\|\]/ contained
syn match wesmlTagError display "\[\s*\([^a-zA-Z_\]/+ ]\|\(\w\+\([^a-zA-Z_\] ]\|\s\+[^\] ]\+\)\|[/+]\(\s*[^\] ]\+\s\+[^\] ]\+\|\s*\w*[^a-zA-Z_ \]]\+\)\)\)[^\]]*\]" contained


"""""""""""""
" Attribute "
"""""""""""""
" Match a line: key,...=value,...
syn match wesmlAttribute display /^\s*[^{#]\+=[^#]*/ contains=wesmlKey,wesmlEqual,wesmlValue
syn match wesmlKey display /\s*\zs\([^= ]*\s*\)*[^= ]*\ze\s*=/ contained contains=wesmlComma,wesmlInclude
syn match wesmlEqual display "=" contained
syn match wesmlPlus display "+" contained
syn match wesmlComma display "," contained
syn match wesmlValue display /=\@<=[^#]*/ contained contains=wesmlVariable,wesmlNumber,wesmlQString,wesmlCString,wesmlEString,wesmlComma,wesmlUnderscore,wesmlBoolean,wesmlIdentifier,wesmlPlus,wesmlInclude,wesmlFormula,wesmlValueError
syn match wesmlValueError display "=" contained


""""""""""""
" Variable "
""""""""""""
" An identifier must start with \h and continue with \w.
" It can be indexed with []
" A member can be selected with .
" If the variable is in a string, it must end with a char other than \w[]$. FIXME not sure how wesnoth parser is working on this one, this set is certainly not convering all situations.
syn match wesmlIdentifier display /\(\h\w*\(\[[^\]]\+\]\)\?\)\(\.\h\w*\(\[[^\]]\+\]\)\?\)*/ contained contains=wesmlBracket,wesmlDot,wesmlMacroName,wesmlBoolean,wesmlUnderscore
syn match wesmlVariable display /\([\]a-zA-Z0-9_]\@<=\.\|\$\)\(\h\w*\(\[[^\]]\+\]\)\?\)\(\.\h\w*\(\[[^\]]\+\]\)\?\)*/ contained contains=wesmlBracket,wesmlDot
syn region wesmlBracket display start=/\[/ end=/\]/ contained contains=wesmlNumber,wesmlVariable,wesmlInclude
syn match wesmlDot display /\./ contained
syn match wesmlStringVariable display /$\h[a-zA-Z0-9_\[\]$\.]*|\?/ contains=wesmlVariable,wesmlStringVariableEnd contained
syn match wesmlStringVariableEnd display "|" contained


"""""""""""
" Formula "
"""""""""""
" TODO Basic support, must be improved a lot.
syn region wesmlFormula display start="$(" end=")" contains=wesmlFormulaExpression,wesmlNumber,wesmlVariable,wesmlFormulaOperator,wesmlIdentifier,wesmlQString,wesmlFormulaString contained
syn region wesmlFormulaExpression display start="$\@<!(" end=")" contains=wesmlFormulaExpression,wesmlNumber,wesmlVariable,wesmlFormulaOperator,wesmlIdentifier,wesmlQString,wesmlFormulaString contained
syn match wesmlFormulaOperator "+\|->\?\|\*\|/\|\[\|\]\|,\|\.\|'" contained
syn region wesmlFormulaString start=+'+ end=+'+ contained


"""""""
" Lua "
"""""""
" Any string value with the key "code" is considered to be lua.
syn region wesmlLuaC start=+\(^[^#]*code\s*=\s*<<\)\@<=+ end=+\ze>>+ contains=@luaTop,wesmlStringSpecial,wesmlStringVariable,wesmlFormula contained
syn region wesmlLuaQ start=+\(^[^#]*code\s*=\s*"\)\@<=+ skip=+""+ end=+\ze"+ contains=@luaTop,wesmlStringSpecial,wesmlStringVariable,wesmlFormula contained


"""""""""""""""""
" Standard tags "
"""""""""""""""""
" Copied from the wiki: <http://wiki.wesnoth.org/AlphabeticalWML>
syn keyword wesmlStdTag contained abilities about advanced_preference advancefrom advancement
syn keyword wesmlStdTag contained advances ai allow_end_turn allow_extra_recruit allow_recruit 
syn keyword wesmlStdTag contained allow_undo and animate_unit animation array 
syn keyword wesmlStdTag contained attack attack_filter attacks avoid base_unit 
syn keyword wesmlStdTag contained binary_path bold brush campaign capture_village 
syn keyword wesmlStdTag contained case chat choose clear_global_variable clear_menu_item 
syn keyword wesmlStdTag contained clear_variable colour_adjust command damage deaths 
syn keyword wesmlStdTag contained defend defends defense delay deprecated_message 
syn keyword wesmlStdTag contained destination disallow_end_turn disallow_extra_recruit disallow_recruit do 
syn keyword wesmlStdTag contained editor_group editor_music editor_times effect 
syn keyword wesmlStdTag contained else endlevel end_turn era event 
syn keyword wesmlStdTag contained expenses filter filter filter_attack filter_attack 
syn keyword wesmlStdTag contained filter_condition filter_location filter_second filter_second filter_second_attack 
syn keyword wesmlStdTag contained filter_second_attack filter_side filter_vision filter_wml find_path 
syn keyword wesmlStdTag contained fire_event floating_text format frame game_config 
syn keyword wesmlStdTag contained generator get_global_variable gold harm_unit have_location 
syn keyword wesmlStdTag contained have_unit header heal_unit hide_help hide_unit 
syn keyword wesmlStdTag contained if image img income init_side 
syn keyword wesmlStdTag contained insert_tag inspect italic item jump 
syn keyword wesmlStdTag contained join kill killed label language 
syn keyword wesmlStdTag contained leader_goal locale lua main_map menu 
syn keyword wesmlStdTag contained message mini_map missile_frame modifications modify_ai 
syn keyword wesmlStdTag contained modify_side modify_turns modify_unit move move_unit 
syn keyword wesmlStdTag contained move_unit_fake move_units_fake movement costs movetype 
syn keyword wesmlStdTag contained multiplayer multiplayer_side music not num_units 
syn keyword wesmlStdTag contained object objectives objective observers open_help 
syn keyword wesmlStdTag contained option or panel part petrify 
syn keyword wesmlStdTag contained place_shroud position print protect_location protect_unit 
syn keyword wesmlStdTag contained race random recall recalls recruit 
syn keyword wesmlStdTag contained recruits redraw ref remove_shroud remove_unit_overlay 
syn keyword wesmlStdTag contained removeitem remove_sound_source replace_map replace_schedule replay 
syn keyword wesmlStdTag contained replay_start resistance resolution results role 
syn keyword wesmlStdTag contained save scenario scroll scroll_to scroll_to_unit 
syn keyword wesmlStdTag contained secondary_attack_filter secondary_unit_filter section select_unit set_global_variable 
syn keyword wesmlStdTag contained set_menu_item set_recruit set_extra_recruit set_variable set_variables 
syn keyword wesmlStdTag contained show_objectives side side_playing snapshot sound 
syn keyword wesmlStdTag contained sound_source source split statistics status 
syn keyword wesmlStdTag contained store_gold store_items store_locations store_map_dimensions store_reachable_locations 
syn keyword wesmlStdTag contained store_side store_starting_location store_time_of_day store_unit store_unit_type 
syn keyword wesmlStdTag contained store_unit_type_ids store_villagesstory switch target team 
syn keyword wesmlStdTag contained teleport teleport_anim terrain terrain_graphics terrain_mask 
syn keyword wesmlStdTag contained terrain_type test textdomain text_input theme 
syn keyword wesmlStdTag contained then tile time time_area time_of_day 
syn keyword wesmlStdTag contained topic toplevel trait transform_unit tunnel 
syn keyword wesmlStdTag contained turn tutorial unhide_unit unit unit_abilities 
syn keyword wesmlStdTag contained unit_alignment unit_description unit_filter unit_hp unit_image 
syn keyword wesmlStdTag contained unit_level unit_moves unit_overlay unit_profile unit_status 
syn keyword wesmlStdTag contained unit_traits unit_type unit_weapons unit_xp units 
syn keyword wesmlStdTag contained unpetrify unstore_unit upkeep variable variables 
syn keyword wesmlStdTag contained village villages volume while wml_message 

" GUI tags, listed via grep "\[[[:lower:]_-]\+\]" data/gui/default/w*/*.cfg -ho | sort -u
" image is already a standard tag
syn keyword wesmlGUITag contained background blur button button_definition circle
syn keyword wesmlGUITag contained column content definition draw drawing
syn keyword wesmlGUITag contained drawing_definition foreground grid header
syn keyword wesmlGUITag contained helptip horizontal_listbox_definition horizontal_scrollbar horizontal_scrollbar_definition
syn keyword wesmlGUITag contained image_definition instance item_definition
syn keyword wesmlGUITag contained label label_definition layer line linked_group
syn keyword wesmlGUITag contained list_data list_definition listbox listbox_definition main
syn keyword wesmlGUITag contained matrix matrix_definition minimap minimap_definition 
syn keyword wesmlGUITag contained multi_page multi_page_definition num_box node node_definition
syn keyword wesmlGUITag contained page_definition pane panel panel_definition password_box pre_commit
syn keyword wesmlGUITag contained progress_bar progress_bar_definition rectangle repeating_button repeating_button_definition
syn keyword wesmlGUITag contained resolution row scroll_label scroll_label_definition 
syn keyword wesmlGUITag contained scrollbar_panel scrollbar_panel_definition slider slider_definition
syn keyword wesmlGUITag contained spacer spacer_definition stack stacked_widget stacked_widget_definition
syn keyword wesmlGUITag contained state_disabled state_disabled_selected state_enabled state_enabled_selected
syn keyword wesmlGUITag contained state_focussed state_focussed_selected state_pressed text
syn keyword wesmlGUITag contained text_box text_box_definition toggle_button toggle_button_definition
syn keyword wesmlGUITag contained toggle_panel toggle_panel_definition tooltip tree_view tree_view_definition
syn keyword wesmlGUITag contained vertical_scrollbar vertical_scrollbar_definition viewport
syn keyword wesmlGUITag contained widget window window_definition




"""""""""""""
" Highlight "
"""""""""""""
hi def link wesmlBoolean					Constant
hi def link wesmlNumber						Number
hi def link wesmlQString					String
hi def link wesmlQStringEsc					SpecialChar
hi def link wesmlCString					String
hi def link wesmlEString					SpecialChar
hi def link wesmlStringSpecial				SpecialChar

hi def link wesmlComment					Comment
hi def link wesmlTodo						Todo
hi def link wesmlWmlindent					SpecialComment

hi def link wesmlPPDArgument				Macro
hi def link wesmlPPDIfverArgument			Macro
hi def link wesmlPPDOperator				Operator
hi def link wesmlVerNumber					Number
hi def link wesmlPPDNullary					Macro
hi def link wesmlPPDNullaryTest				PreCondit
hi def link wesmlPPDNary					Macro
hi def link wesmlPPDNaryTest				PreCondit
hi def link wesmlPPDIfver					PreCondit
hi def link wesmlInclude					Include
hi def link wesmlIncludeMark				Constant
hi def link wesmlMacroName					Include

hi def link wesmlTagMark					Function
hi def link wesmlTagError					Error

hi def link wesmlEqual						Operator
hi def link wesmlPlus						Operator
hi def link wesmlComma						Operator
hi def link wesmlUnderscore					Special
hi def link wesmlValueError					Error

hi def link wesmlVariable					Identifier
hi def link wesmlBracket					Operator
hi def link wesmlDot						Operator
hi def link wesmlStringVariableEnd			SpecialChar

hi def link wesmlFormula					Statement
hi def link wesmlFormulaOperator			Function
hi def link wesmlFormulaString				String

hi def link wesmlStdTag						Statement
hi def link wesmlGUITag						Statement
