;;; wesnoth-mode.el --- A major mode for editing WML.
;; Copyright (C) 2006, 2007, 2008 Chris Mann

;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation; either version 2 of the
;; License, or (at your option) any later version.

;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to the
;; Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
;; MA 02139, USA.

;;; Description:
;; wesnoth-mode is a major mode for Emacs which assists in the editing
;; of Wesnoth Markup Language (WML) files.  Currently, this mode
;; features syntax highlighting support, automatic indentation,
;; tag-completion and preliminary support for syntax checking.

;;; Commentary:
;; Add the following to your .emacs:
;;   (add-to-list 'load-path "path/to/wesnoth-mode")
;;   (autoload 'wesnoth-mode "wesnoth-mode" "Major mode for editing WML." t)
;; Optionally adding:
;;   (add-to-list 'auto-mode-alist '("\\.cfg\\'" . wesnoth-mode))
;; to automatically load wesnoth-mode for all files ending in '.cfg'.

;;; History:
;; 1.2.5
;; * Fixed support for GNU Emacs 21.
;; * Added several new tags to `wesnoth-tags-list'.
;; * Added M-TAB binding for `wesnoth-insert-tag'.
;; * `wesnoth-insert-tag' now takes an optional numeric argument indicating
;;   how many blocks to wrap across instead of a region.
;; * Support for `wesnoth-indent-preprocessor-bol' removed.
;; * Fixed a bug in `wesnoth-insert-tag' and `wesnoth-insert-missing-closing'
;;   causing tags not to be inserted in the correct position.
;; * Fixed highlighting of array indexes as tags.
;; 1.2.4
;; * Improved syntax-highlighting for macro calls.
;; * Underscore is now treated as whitespace.
;; * Fixed incorrect indentation when preprocessor preceeded by whitespace.
;; * Point is now placed at the first non-whitespace character of the line,
;;   instead of the last.
;; * Corrected minor indentation bugs.
;; * Indenting across large regions is now much more efficient.
;; * Fix hooks modifying wesnoth-mode-map causing default bindings not being
;;   applied.
;; 1.2.3
;; * Now compatible with GNU Emacs 21.4.
;; * Added support for several new tags.
;; * Added menu entry for wesnoth-mode.
;; * Significant speed increase to indentation.
;; * Indentation can now be customised using `wesnoth-indent-preprocessor-bol'
;;   and `wesnoth-indent-savefile'; support for `wesnoth-indentation-function'
;;   has been removed.
;; * Trailing whitespace is no longer created when creating a second
;;   consecutive newline.
;; * Spurious newlines are no longer created when inserting a tag elements
;;   around a region.
;; 1.2.2
;; * Added functions: `wesnoth-indent', `wesnoth-element-closing',
;;   `wesnoth-element', `wesnoth-element-opening',
;;   `wesnoth-insert-and-indent', `wesnoth-insert-missing-closing'.
;; * Renamed `wesnoth-indent-line-default', `wesnoth-indent-line-savefile' and
;;   `wesnoth-jump-backward', `wesnoth-jump-forward' to
;;   `wesnoth-indent-withtags-inline', `wesnoth-indent-default-inline' and
;;   `wesnoth-backward-tag', `wesnoth-forward-tag', respectively.
;; * Fixed a bug in indentation where content was needed between elements pairs
;;   for indentation to work.
;; * Fixed `wesnoth-newline-and-indent' ignoring the state of
;;   `wesnoth-auto-indent-flag'.
;; * Fixed `{...}' and `#endif' not font-locking correctly.
;; * Added indentation styles: `wesnoth-indent-default',
;;   `wesnoth-indent-withtags' which implement a a similar indentation
;;   style to the existing styles, however all preprocessor statements are
;;   indented to the first column.
;; * Added support for several new tags.
;; * Modified `wesnoth-newline' to behave more consistently.
;; * `wesnoth-jump-to-matching', `wesnoth-forward-tag', `wesnoth-backward-tag'
;;   now leaves point at the beginning (when moving backward) or end (when
;;   moving forward) of the match.
;; * `wesnoth-jump-to-matching' now attempts to find a target if necessary and
;;   will now work on preprocessor statements.  Will now warn if jump
;;   destination may not be correct (due to errors in WML structure).
;; * Indentation style is now determined by `wesnoth-indentation-function'.
;; * `wesnoth-check-structure' can now be applied over an active region and
;;   now checks preprocessor statements for correct nesting.
;; * `wesnoth-newline' and `wesnoth-newline-and-indent' can now be forced to
;;   perform indentation by providing a prefix argument.
;; * Indentation styles now leave point at the first non-whitespace character
;;   of the line.
;; * `wesnoth-check-tag-names' now reports on success.
;; * `wesnoth-insert-tag' is now able to insert tags around a region.
;; * `outline-minor-mode' now works on macro definitions.
;; 1.2.1
;; * Base indent now defaults to 4.
;; * Added support for #ifndef.

;;; Code:
(defconst wesnoth-mode-version "1.2.5"
  "The current version of `wesnoth-mode'.")

(defgroup wesnoth-mode nil "Wesnoth-mode access"
  :group 'languages
  :prefix "wesnoth-")

(defcustom wesnoth-auto-indent-flag t
  "Non-nil means indent the current line upon creating a newline."
  :type 'boolean
  :group 'wesnoth-mode)

(defcustom wesnoth-indent-default-style t
  "Non-nil means to use the current indentation conventions.
If nil, use the old convention for indentation.
The current convention is all attributes are indented a level deeper
than their parent; in the past attributes were indented to the same
level as their parent.")

(defvaralias 'wesnoth-indent-savefile 'wesnoth-indent-default-style)

(defcustom wesnoth-base-indent 4
  "The number of columns to indent WML."
  :type 'integer
  :group 'wesnoth-mode)

(defconst wesnoth-preprocessor-regexp
  "[\t ]*#\\(enddef\\|define \\|e\\(lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\|\\(ifn?\\|un\\)def\\)"
  "Regular expression to match all preprocessor statements.")

(defconst wesnoth-preprocessor-opening-regexp
  "[\t ]*#\\(define \\|else\\|ifdef \\|ifndef \\)"
  "Regular expression to match \"opening\" preprocessor statements.")

(defconst wesnoth-preprocessor-closing-regexp
  "[\t ]*#e\\(lse\\|nd\\(\\(de\\|i\\)f\\)\\)"
  "Regular expression to match \"closing\" preprocessor statements.")

(defvar wesnoth-define-blocks '()
  "Cache of all toplevel #define and #enddef pairs.")

(defvar wesnoth-mode-hook nil)

(defvar wesnoth-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "C-M-a") 'wesnoth-backward-element)
    (define-key map (kbd "C-M-e") 'wesnoth-forward-element)
    (define-key map (kbd "C-m") 'wesnoth-newline)
    (define-key map (kbd "C-j") 'wesnoth-newline-and-indent)
    (define-key map (kbd "C-c c") 'wesnoth-check-structure)
    (define-key map (kbd "C-c C-c") 'wesnoth-check-structure)
    (define-key map (kbd "C-c e") 'wesnoth-insert-tag)
    (define-key map (kbd "C-c C-e") 'wesnoth-insert-tag)
    (define-key map (kbd "M-TAB") 'wesnoth-insert-tag)
    (define-key map (kbd "C-c m") 'wesnoth-jump-to-matching)
    (define-key map (kbd "C-c C-m") 'wesnoth-jump-to-matching)
    (define-key map (kbd "C-c n") 'wesnoth-check-tag-names)
    (define-key map (kbd "C-c C-n") 'wesnoth-check-tag-names)
    (define-key map (kbd "C-c /") 'wesnoth-insert-missing-closing)
    (define-key map (kbd "C-c C-/") 'wesnoth-insert-missing-closing)
    (define-key map [menu-bar wesnoth]
      (cons "WML" (make-sparse-keymap "WML")))
    (define-key map [menu-bar wesnoth insert-tag]
      '("Insert Tag" . wesnoth-insert-tag))
    (define-key map [menu-bar wesnoth check-names]
      '("Check Tag Names" . wesnoth-check-tag-names))
    (define-key map [menu-bar wesnoth check-structure]
      '("Check Structure" . wesnoth-check-structure))
    (define-key map [menu-bar wesnoth jump-to-matching]
      '("Jump to Matching" . wesnoth-jump-to-matching))
    (define-key map [menu-bar wesnoth insert-missing-closing]
      '("Insert Missing Tag" . wesnoth-insert-missing-closing))
    map)
  "Keymap used in wesnoth mode.")

(defvar wesnoth-syntax-table
  (let ((wesnoth-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?=  "." wesnoth-syntax-table)
    (modify-syntax-entry ?_ "_" wesnoth-syntax-table)
    (modify-syntax-entry ?-  "_" wesnoth-syntax-table)
    (modify-syntax-entry ?.  "_" wesnoth-syntax-table)
    (modify-syntax-entry ?\n ">" wesnoth-syntax-table)
    (modify-syntax-entry ?\r ">" wesnoth-syntax-table)
    wesnoth-syntax-table)
  "Syntax table for `wesnoth-mode'.")

;; Prevents automatic syntax-highlighting of elements which might be
;; pre-processor statements.
(defvar wesnoth-syntactic-keywords
  (list
   '("\\(^[\t ]*\\(#\\(?:define \\|e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\|\\(?:ifn?\\|un\\)def \\)\\)\\|#enddef\\)" 1 "w")
   '("\\(#[\t ]*.*$\\)" 1 "<"))
  "Highlighting syntactic keywords within `wesnoth-mode'.")

(defun wesnoth-preprocessor-best-face ()
  "Use `font-lock-preprocessor-face' when available."
  (if (boundp 'font-lock-preprocessor-face)
      (copy-face 'font-lock-preprocessor-face 'wesnoth-preprocessor-face)
    (copy-face 'font-lock-keyword-face 'wesnoth-preprocessor-face)))

(defvar wesnoth-font-lock-keywords
  (list
   '("#\\(?:define\\|\\(?:ifn?\\|un\\)def\\)" . 'wesnoth-preprocessor-face)
   '("\\(#\\(?:define\\|\\(?:ifn?\\|un\\)def\\)\\)[\t ]+\\(\\(\\w\\|_\\)+\\)"
     2 font-lock-function-name-face)
   '("\\(#e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\)" .
     'wesnoth-preprocessor-face)
   '("\\({[@~]?\\(\\w\\|\\.\\|/\\|-\\)+}\\)"
     (1 font-lock-function-name-face))
   '("\\({\\(\\w\\|:\\|_\\)+\\|{[~@]?\\)"
     (1 font-lock-function-name-face))
   '("}" .  font-lock-function-name-face)
   '("^[\t ]*\\(\\[[^]]+\\]\\)" 1 font-lock-type-face)
   '("\\$\\(\\w\\|_\\)+" . font-lock-variable-name-face)
   '("\\(\\(\\w\\|_\\)+\\(\\,[\t ]*\\(\\w\\|_\\)+\\)*\\)="
     1 font-lock-variable-name-face))
  "Syntax highlighting for `wesnoth-mode'.")

(defconst wesnoth-element-closing "^[\t ]*\\(\\[/\\|#enddef\\)"
  "String to use for a closing element.")

(defconst wesnoth-element-opening "^[\t ]*\\(\\[[^/]\\|#define\\)"
  "String to use for an opening element.")

(defconst wesnoth-element "^[\t ]*\\(\\[[^]]?\\|#define\\|#enddef\\)"
  "String to use for an opening or closing element.")

;;; Insertion
(defvar wesnoth-tags-list
  (list
   "abilities" "about" "advances" "advancefrom" "ai" "allow_recruit" "and"
   "animation" "array" "attack" "attack_anim" "attacks" "avoid" "binary_path"
   "bold" "campaign" "capture_village" "choose""clear_variable"
   "colour_adjust" "command" "deaths" "debug_message" "defend" "defends"
   "defense" "delay" "destination" "disallow_recruit" "do" "effect" "else"
   "end_turn" "endlevel" "entry" "era" "event" "expenses" "filter"
   "filter_attack" "filter_adjacent_location" "filter_location"
   "filter_radius" "filter_second" "filter_vision" "format" "frame"
   "game_config" "generator" "gold" "have_unit" "header" "hide_unit" "if"
   "illuminated_time" "image" "img" "income" "italic" "item" "jump" "kill"
   "killed" "label" "language" "leader_goal" "main_map" "menu" "message"
   "mini_map" "missile_frame" "modifications" "modify_side" "modify_turns"
   "move" "move_unit_fake" "movement_costs" "movetype" "multiplayer"
   "multiplayer_side" "music" "not" "num_units" "object" "objectives"
   "objective" "observers" "option" "or" "panel" "part" "place_shroud"
   "position" "print" "protect_location" "protect_unit" "race" "random"
   "recall" "recalls" "recruit" "recruits" "redraw" "ref" "remove_shroud"
   "remove_unit_overlay" "removeitem" "replay" "replay_start" "resistance"
   "resolution" "results" "role" "save" "scenario" "scroll" "scroll_to"
   "scroll_to_unit" "section" "set_menu_item" "set_recruit" "set_specials"
   "set_variable" "show_if" "side" "side_playing" "snapshot" "sound" "source"
   "specials" "statistics" "status" "stone" "store_gold" "store_locations"
   "store_starting_location" "store_side" "store_unit" "story" "target" "team"
   "teleport" "teleport_anim" "terrain" "terrain_graphics" "terrain_mask"
   "test" "text_input" "textdomain" "theme" "then" "tile" "time" "time_area"
   "time_of_day" "topic" "toplevel" "trait" "turn" "tutorial" "unhide_unit"
   "unit" "unit_abilities" "unit_alignment" "unit_description" "unit_hp"
   "unit_image" "unit_level" "unit_moves" "unit_overlay" "unit_profile"
   "unit_status" "unit_traits" "unit_type" "unit_weapons" "unit_xp" "units"
   "unstone" "unstore_unit" "upkeep" "variable" "variables" "village"
   "villages" "while" "wml_filter")
  "A list containing all tags which are available in WML.")

(defvar wesnoth-completion-cache '()
  "List of tags which have been generated by `wesnoth-build-completion'.")

(defun wesnoth-build-completion (&optional rebuild)
  "Create a new list for tag completion if necessary.
Rebuilding list is required for versions of GNU Emacs earlier
than 22.  If REBUILD is non-nil, regenerate `wesnoth-completion-cache'."
  (interactive "P")
  (if (> emacs-major-version 21)
      wesnoth-tags-list
    (if (and wesnoth-completion-cache (not rebuild))
	wesnoth-completion-cache
      (let ((tags '())
	    (iter 0))
	(dolist (tag wesnoth-tags-list)
	  (setq iter (1+ iter))
	  (setq tags (append tags (list (cons tag iter)))))
	(setq wesnoth-completion-cache tags)))))

(defun wesnoth-insert-tag (&optional elements tagname)
  "Insert the specified opening tag and it's matching closing tag.
Both the opening and closing tags will be placed on their own
lines with point positioned between them.  Completion of tags at
the prompt uses `wesnoth-tags-list'.

ELEMENTS is specifies the number of following blocks which the
tag should wrap around.

TAGNAME is the name of the tag to be inserted."
  (interactive "Ps")
  (unless tagname
    (setq tagname (completing-read "Tag: " (wesnoth-build-completion))))
  (or elements (setq elements 0))
  (let ((depth 0)
	(start (save-excursion (forward-line -1) (point)))
	(end (unless (= elements 0)
	       (wesnoth-nth-pair-position elements))))
    (wesnoth-insert-element-separately "[" tagname "]")
    (save-excursion
      (if end
	  (goto-char (marker-position end))
	(newline 2))
      (wesnoth-insert-element-separately "[/" tagname "]")
      (indent-region start (point) nil))
    (unless end
      (forward-line 1)))
  (wesnoth-indent))

(defun wesnoth-nth-pair-position (count)
  "Return `point' after COUNT number of matching element pairs.
COUNT is a positive number representing the number of balanced
pairs to move across.
`point' is returned as a marker object."
  (save-excursion
    (while (> count 0)
      ;; Currently looking-at target tag.  Stop here to avoid
      ;; incorrect nesting.
      (unless (wesnoth-search-for-matching-tag
	       'search-forward-regexp wesnoth-element-closing 'point-max)
	(setq count 0)
	(search-backward-regexp wesnoth-element-closing (point-min) t))
      (and (> (decf count) 0) (forward-line 1)))
    (end-of-line)
    (point-marker)))

(defun wesnoth-insert-element-separately (&rest strings)
  "Concatenate STRINGS and insert them on a line of their own."
  (let ((create-newline	 (save-excursion
			   (beginning-of-line)
			   (if (looking-at "^[\t ]*$") nil t))))
    (when create-newline
      (if (> (point) (save-excursion (back-to-indentation) (point)))
	  (progn
	    (end-of-line)
	    (newline))
	(beginning-of-line)
	(open-line 1)))
    (insert (apply 'concat strings))))

(defun wesnoth-insert-missing-closing (&optional start end)
  "Insert the next expected closing element at point.

START and END define the region to check for missing closing
elements.  If function `transient-mark-mode' is enabled, the region
specified will be used as START and END.  Otherwise, START and
END will be the minimum and maximum positions of the buffer,
respectively."
  (interactive)
  (if (and transient-mark-mode mark-active)
      (setq start (region-beginning)
	    end (copy-marker (region-end)))
    (setq start (point-min)
	  end (point-max)))
  (let ((element (wesnoth-check-structure start end)))
    (if (not element)
	(error "%s" "Unable to find element to insert")
      (when (string= element "Unexpected end of file")
	(error "%s" element))
      (wesnoth-insert-element-separately element)))
  (wesnoth-indent))

(defun wesnoth-insert-and-indent (&rest args)
  "Concatenate and insert the given string(s) before indenting.

ARGS is a list of strings to be inserted."
  (insert (apply 'concat args))
  (wesnoth-indent))

(defun wesnoth-newline (&optional indent)
  "Indent both the current line and the newline created.
If `wesnoth-auto-indent-flag' is nil, indentation will not be
performed.  Indentation can be forced by setting INDENT to
non-nil."
  (interactive)
  (save-excursion
    (when (and (or wesnoth-auto-indent-flag indent)
	       (not (looking-at "^[\t ]*$")))
      (wesnoth-indent)))
  (newline))

;;; Movement
(defmacro wesnoth-navigate-element (repeat search-function bound)
  "Move point to the tag in the given direction REPEAT times.

SEARCH-FUNCTION is the symbol of the function for searching in
the required direction, with BOUND marking the furthest point to
search."
  `(progn
     (or ,repeat (setq ,repeat 1))
     (while (> ,repeat 0)
       (and (eq ,search-function 'search-forward-regexp) (end-of-line))
       (funcall ,search-function wesnoth-element-opening ,bound t)
       (back-to-indentation)
       (decf ,repeat))))

(defun wesnoth-forward-element (repeat)
  "Move point to the end of the next tag.
REPEAT is an optional numeric argument.  If REPEAT is non-nil,
jump forward the specified number of tags."
  (interactive "p")
  (if (< repeat 0)
      (wesnoth-backward-element (abs repeat))
    (wesnoth-navigate-element repeat 'search-forward-regexp (point-max))))

(defun wesnoth-backward-element (repeat)
  "Move point to the beginning of the previous tag.
REPEAT is an optional numeric argument.  If REPEAT is non-nil,
jump backward the specified number of tags."
  (interactive "p")
  (if (< repeat 0)
      (wesnoth-forward-element (abs repeat))
    (wesnoth-navigate-element repeat 'search-backward-regexp (point-min))))

(defmacro wesnoth-search-for-matching-tag (search-function search-string bound)
  "Search for the matching tag for the current line.

SEARCH-FUNCTION is the name of the function used to perform the search.
SEARCH-STRING is a string representing the matching tag type.
BOUND is the bound to be passed to the search function."
  `(let ((depth 1))
     (unless (looking-at ,search-string)
       (unless (> (point) (funcall ,bound)) (end-of-line))
       (while (and (> depth 0)
		   (funcall ,search-function wesnoth-element
			    (funcall ,bound) t))
	 (if (string-match ,search-string (match-string 0))
	     (decf depth)
	   (incf depth)))
       t)))

(defun wesnoth-jump-to-matching ()
  "Jump point to the matching opening/closing tag."
  (interactive)
  (beginning-of-line)
  (if (looking-at wesnoth-element-opening)
      (wesnoth-search-for-matching-tag
       'search-forward-regexp wesnoth-element-closing 'point-max)
    (wesnoth-search-for-matching-tag
     'search-backward-regexp wesnoth-element-opening 'wesnoth-wml-start-pos))
  (back-to-indentation))

(defun wesnoth-wml-start-pos ()
  "Determine the position of `point' relative to where the actual WML begins.
Return the likely starting position of the WML if it is found.
Otherwise return nil."
  (save-excursion
    (goto-char (point-min))
    (when (search-forward-regexp wesnoth-element (point-max) t)
      (beginning-of-line)
      (point))))

(defun first-column-indent-p (point)
  "Return non-nil if the current line should not be indented.

POINT is the position in the buffer to check.
CONTEXT represents the type of element which precedes the current element."
  (or (not (wesnoth-wml-start-pos))
      (<= (point) (wesnoth-wml-start-pos))
      (nth 3 (parse-partial-sexp (point-min) point))
      (looking-at wesnoth-preprocessor-regexp)))

(defun wesnoth-indent ()
  "Indent the current line as WML."
  (beginning-of-line)
  (let ((cur-indent 0))
    (unless (first-column-indent-p (point))
      (multiple-value-bind (context ref-indent)
	  (wesnoth-determine-context (point))
	(cond
	 ((eq context 'opening)
	  (if (or (and wesnoth-indent-default-style
		       (not (looking-at wesnoth-element-closing)))
		  (looking-at wesnoth-element-opening))
	      (setq cur-indent (+ ref-indent wesnoth-base-indent))
	    (setq cur-indent ref-indent)))
	 ((eq context 'closing)
	  (if (or (looking-at "^[\t ]*\\[/")
		  (and (not wesnoth-indent-default-style)
		       (not (looking-at wesnoth-element-opening))))
	      (setq cur-indent (- ref-indent wesnoth-base-indent))
	    (setq cur-indent ref-indent))))))
    (indent-line-to (max cur-indent 0))))

(defun wesnoth-within-define (position)
  "Determine whether point is currently inside a #define block.
POSITION is the initial cursor position."
  (let ((depth 0))
    (dolist (element (or wesnoth-define-blocks
                         (wesnoth-find-macro-definitions)))
      (when (= (cadr (sort (append (mapcar 'marker-position (cadr element))
                                   (list position)) '>)) position)
	(setq depth (max (car element) depth))))
    depth))

(defun wesnoth-find-macro-definitions ()
  "Return information regarding positioning of macro definitions."
  (save-excursion
    (goto-char (point-min))
    (let ((depth 0)
	  openings cache)
      (while (search-forward-regexp "^[\t ]*\\(#define\\|#enddef\\)" (point-max) t)
	(and (string= (match-string 1) "#define") (beginning-of-line))
	(setq depth
	      (if (string= (match-string 1) "#define")
		  (progn
		    (add-to-list 'openings (point-marker))
		    (1+ depth))
		(add-to-list 'cache
			     (list depth (list (car openings) (point-marker))))
		(setq openings (cdr openings))
		(1- depth)))
	(end-of-line))
      cache)))

(defun wesnoth-indent-region (start end)
  "Indent the region from START to END.

Creates and destroys a cache of macro definition details as necessary."
  (interactive "r")
  (unwind-protect
      (save-excursion
	(goto-char end)
	(setq end (point-marker))
	(goto-char start)
	(setq wesnoth-define-blocks (wesnoth-find-macro-definitions))
	(or (bolp) (forward-line 1))
	(while (< (point) end)
	  (if (looking-at "^[\t ]*$")
	      (indent-line-to 0)
	    (funcall indent-line-function))
	  (forward-line 1)))
    (setq wesnoth-define-blocks nil)))

(defun wesnoth-determine-context (position)
  "Determine the type of the last relevant element.

POSITION is the buffer position of the element for which to
determine the context."
  (save-excursion
    (search-backward-regexp wesnoth-element (wesnoth-wml-start-pos) t)
    (let ((match (or (match-string 1) ""))
	  (depth (wesnoth-within-define position)))
      (while (and (> (wesnoth-within-define (point)) depth)
		  (not (= (point) (wesnoth-wml-start-pos))))
	(search-backward-regexp wesnoth-element
				(wesnoth-wml-start-pos) t)
	(setq match (match-string 1)))
      (when (and (= (point) (wesnoth-wml-start-pos)) (= depth 0)
		 (string-match "#define" match))
	;; Found nothing of use; reset match and assume top-level tag.
	(setq match ""))
      (cond
       ((string-match "\\[/\\|#enddef" match)
	(values 'closing (current-indentation)))
       ((string-match "\\[[^/]?\\|#define" match)
	(values 'opening (current-indentation)))))))

(defun wesnoth-newline-and-indent (&optional indent)
  "Indent both the current line and the newline created.
If `wesnoth-auto-indent-flag' is nil, indentation will not be
performed.

If the optional argument, INDENT is non-nil, force indentation to
be performed."
  (interactive)
  (wesnoth-newline)
  (when (or wesnoth-auto-indent-flag indent)
    (wesnoth-indent)))

;;; WML checks
(defun wesnoth-check-tag-names ()
  "Check the names of all tags in the buffer for correctness.
If a tag is found which is not present in the list an error will
be signalled and point will be moved to the corresponding
position."
  (interactive)
  (let ((tag-position nil)
	(missing-tag-name nil))
    (save-excursion
      (goto-char (point-min))
      (while (and (search-forward-regexp "^[\t ]*\\[" (point-max) t)
		  (not tag-position))
	(beginning-of-line)
	(when (looking-at "^[\t ]*\\[/?\\(\\(\\w\\|_\\)+\\|_\\)\\]")
	  (unless (member (match-string-no-properties 1) wesnoth-tags-list)
	    (setq tag-position (point))
	    (setq missing-tag-name (match-string-no-properties 1))))
	(end-of-line)))
    (if (not tag-position)
	(message "%s" "No unknown tag names found.")
      (goto-char tag-position)
      (back-to-indentation)
      (message "'%s' is not known to exist"
	       missing-tag-name))))

(defmacro wesnoth-element-requires (element requirement &optional pop)
  "Process requirements for corresponding preprocessor elements.
ELEMENT is the current element being tested.
REQUIREMENT is the element required to exist for correct nesting.
POP is an optional argument indicating the element should be
removed from the unmatched-tag-list."
  `(when (string= ,element (match-string-no-properties 1))

     (if (string-match ,requirement (car unmatched-tag-list))
	 (progn
	   (and ,pop (setq unmatched-tag-list (cdr unmatched-tag-list)))
	   t)
       (setq error-position (point)))))

(defmacro wesnoth-structure-result (position element)
  "Process results of the structure check.
POSITION is the error position or nil, if no error was found.
ELEMENT is the last unmatched element, or nil if all opening
elements have been matched."
  `(let ((expected nil))
     (when ,element
       (cond ((string= ,element "define ") (setq expected "#enddef"))
	     ((string-match "ifn?def " ,element) (setq expected "#endif"))))
     (if (interactive-p)
	 (if (or ,element ,position)
	     (progn
	       (and ,position (goto-char ,position))
	       (message "Error: Expecting %s" (or expected
						  (concat "[/" ,element "]"))))
	   (message "%s" "Structure appears consistent."))
       (when (or expected ,element)
	 (or expected (concat "[/" ,element "]"))))))

(defun wesnoth-check-structure (&optional start end)
  "Check the buffer for correct nesting of elements.
If a problem is found in the structure, point will be placed at
the location which an element was expected and the expected
element will be displayed in the mini-buffer.

START and END define the region to be checked.  If
function `transient-mark-mode' is enabled, the region specified will be
checked.  Otherwise START and END will be the minimum and maximum
positions of the buffer, respectively."
  (interactive)
  (unless (or start end)
    (if (and transient-mark-mode mark-active)
	(setq start (region-beginning)
	      end (copy-marker (region-end)))
      (setq start (point-min)
	    end (point-max))))
  (let ((unmatched-tag-list '())
	(error-position nil))
    (save-excursion
      (and start (goto-char start))
      (while (and (search-forward-regexp
		   (concat "^\\([\t ]*\\[\\(/?\\(\\w\\|_\\)+\\)\\]\\|"
			   wesnoth-preprocessor-regexp "\\)") end t)
		  (not error-position))
	(beginning-of-line)
	(if (or (looking-at "^[\t ]*\\[\\(\\(\\w\\|_\\)+\\)\\]")
		(looking-at "[\t ]*#\\(define \\|ifdef \\|ifndef \\)"))
	    (setq unmatched-tag-list (cons (match-string-no-properties 1)
					   unmatched-tag-list))
	  (cond
	   ((wesnoth-element-requires "#else" "ifn?def "))
	   ((wesnoth-element-requires "#endif" "ifn?def " t))
	   ((wesnoth-element-requires "#enddef" "define " t))
	   ((looking-at (concat "^[\t ]*\\[/\\(\\(\\w\\|_\\)+\\)\\]\\|"
				wesnoth-preprocessor-closing-regexp))
	    (if (string= (match-string-no-properties 1)
			 (car unmatched-tag-list))
		(setq unmatched-tag-list (cdr unmatched-tag-list))
	      (setq error-position (point))))))
	(end-of-line)))
    (wesnoth-structure-result error-position (car unmatched-tag-list))))

;;; wesnoth-mode
(define-derived-mode wesnoth-mode fundamental-mode "wesnoth-mode"
  "Major mode for editing WML."
  (wesnoth-preprocessor-best-face)
  (set-syntax-table wesnoth-syntax-table)
  (set (make-local-variable 'outline-regexp) "[\t ]*#define")
  (set (make-local-variable 'comment-start) "#")
  (set (make-local-variable 'indent-line-function) 'wesnoth-indent)
  (set (make-local-variable 'indent-region-function) 'wesnoth-indent-region)
  (set (make-local-variable 'font-lock-defaults)
       '(wesnoth-font-lock-keywords
	 nil t nil nil
	 (font-lock-syntactic-keywords . wesnoth-syntactic-keywords)))
  (setq indent-tabs-mode nil)
  (setq mode-name "WML")
  (run-hooks 'wesnoth-mode-hook))

(provide 'wesnoth-mode)

;;; wesnoth-mode.el ends here
