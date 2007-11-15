;; wesnoth-mode.el - A major mode for editing WML.
;; Copyright (C) 2006, 2007  Chris Mann

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

;; Description:
;;   wesnoth-mode is a major mode for Emacs which assists in the editing
;;   of Wesnoth Markup Language (WML) files.  Currently, this mode
;;   features syntax highlighting support, automatic indentation,
;;   tag-completion and preliminary support for syntax checking.

;; Usage:
;;   Add the following to your .emacs:
;;     (add-to-list 'load-path "path/to/wesnoth-mode")
;;     (require 'wesnoth-mode)
;;   Optionally adding:
;;     (add-to-list 'auto-mode-alist '("\\.cfg\\'" . wesnoth-mode))
;;   to automatically load wesnoth-mode for all files ending in '.cfg'.
;;
;; 1.2.1 changes to the 4-space indent used in mainline and supports #ifndef.

(defconst wesnoth-mode-version "1.2.1"
  "The current version of wesnoth-mode.")

(defgroup wesnoth-mode nil "Wesnoth-mode access"
  :group 'languages
  :prefix "wesnoth-")

(defcustom wesnoth-indent-style-savefile nil
  "Whether to use savefile-style indentation for WML.
If nil, the standard-style for WML indentation will be used.
Otherwise, use savefile-style indentation."
  :type 'boolean
  :group 'wesnoth-mode)

(defcustom wesnoth-auto-indent t
  "Whether to attempt tag indentation when a newline is created.
If nil, no indentation will be attempted.  Otherwise, attempt to
indent the line."
  :type 'boolean
  :group 'wesnoth-mode)

(defcustom wesnoth-base-indent 4
  "Whether to attempt tag indentation when a newline is created.
If nil, no indentation will be attempted.  Otherwise, attempt to
indent the line."
  :type 'integer
  :group 'wesnoth-mode)

(defvar wesnoth-mode-hook nil)

(defvar wesnoth-mode-map ()
  "Keymap used in wesnoth mode.")
(unless wesnoth-mode-map
  (setq wesnoth-mode-map (make-sparse-keymap))
  (define-key wesnoth-mode-map "\C-\M-a" 'wesnoth-jump-backward-tag)
  (define-key wesnoth-mode-map "\C-\M-e" 'wesnoth-jump-forward-tag)
  (define-key wesnoth-mode-map "\C-c\C-m" 'wesnoth-jump-to-matching)
  (define-key wesnoth-mode-map "\C-cm" 'wesnoth-jump-to-matching)
  (define-key wesnoth-mode-map "\C-m" 'wesnoth-newline)
  (define-key wesnoth-mode-map "\C-j" 'wesnoth-newline-and-indent)
  (define-key wesnoth-mode-map "\C-c\C-c" 'wesnoth-check-tag-structure)
  (define-key wesnoth-mode-map "\C-cc" 'wesnoth-check-tag-structure)
  (define-key wesnoth-mode-map "\C-c\C-n" 'wesnoth-check-tag-names)
  (define-key wesnoth-mode-map "\C-cn" 'wesnoth-check-tag-names)
  (define-key wesnoth-mode-map "\C-c\C-e" 'wesnoth-insert-tag)
  (define-key wesnoth-mode-map "\C-ce" 'wesnoth-insert-tag))

(defvar wesnoth-syntax-table
  (let ((wesnoth-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?=  "." wesnoth-syntax-table)
    (modify-syntax-entry ?\_ "w" wesnoth-syntax-table)
    (modify-syntax-entry ?-  "_" wesnoth-syntax-table)
    (modify-syntax-entry ?.  "_" wesnoth-syntax-table)
    (modify-syntax-entry ?\' "\"" wesnoth-syntax-table)
    (modify-syntax-entry ?\n ">" wesnoth-syntax-table)
    (modify-syntax-entry ?\r ">" wesnoth-syntax-table)
    wesnoth-syntax-table)
  "Syntax table for wesnoth-mode")

;; Prevents automatic syntax-highlighting of elements which might be
;; pre-processor statements.
(defvar wesnoth-syntactic-keywords
  (list
   '("^[\t ]*\\(#\\(?:define\\|e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)fn?\\)\\)\\|\\(?:ifn?\\|un\\)def\\) \\)" 1 "w")
   '("\\(#[\t ]*.*$\\)" 1 "<"))
  "Highlighting syntactic keywords within wesnoth-mode")

(defvar wesnoth-font-lock-keywords
  (list
   '("\\(#\\(?:define\\|\\(?:ifn?\\|un\\)def\\)\\)"
     1 font-lock-preprocessor-face)
   '("\\(#\\(?:define\\|\\(?:if\\|un\\)def\\)\\)[\t ]+\\(\\w+\\)"
     2 font-lock-function-name-face)
   '("\\(#e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)fn?\\)\\)\\)" . font-lock-preprocessor-face)
   '("[\t ]*\\({[[:word:]/]+\\|{@[[:word:]/]+\\).?*\\(}\\)"
     (1 font-lock-function-name-face)
     (2 font-lock-function-name-face))
   '("\\[[^]]+\\]" . font-lock-type-face)
   '("\\$\\w+" . font-lock-variable-name-face)
   '("\\(\\w+\\(\\,[\t ]*\\w+\\)*\\)="
     1 font-lock-variable-name-face))
  "Syntax highlighting for wesnoth-mode")

(defvar wesnoth-tags-list
  (list
   "advances" "advancefrom" "ai" "allow_recruit" "and" "animation" "array"
   "attack" "attacks" "avoid" "binary_path" "bold" "campaign" "capture_village"
   "choose""clear_variable" "colour_adjust" "command" "deaths" "defend"
   "defends" "defense" "delay" "destination" "disallow_recruit" "do" "effect"
   "else" "end_turn" "endlevel" "era" "event" "expenses" "filter" "filter_second"
   "format" "frame" "game_config" "generator" "gold" "have_unit" "header"
   "hide_unit" "if" "illuminated_time" "image" "img" "income" "italic" "item"
   "jump" "kill" "killed" "label" "language" "leader_goal" "main_map" "menu"
   "message" "mini_map" "missile_frame" "modifications" "modify_side"
   "modify_turns" "move" "move_unit_fake" "movement_costs" "movetype"
   "multiplayer" "multiplayer_side" "music" "not" "num_units" "object"
   "objectives" "objective" "observers" "option" "or" "panel" "part"
   "place_shroud" "position" "print" "protect_location" "protect_unit" "race"
   "random" "recall" "recalls" "recruit" "recruits" "redraw" "ref"
   "remove_shroud" "remove_unit_overlay" "removeitem" "replay" "replay_start"
   "resistance" "resolution" "results" "role" "save" "scenario" "scroll"
   "scroll_to" "scroll_to_unit" "section" "set_recruit" "set_variable" "side"
   "side_playing" "snapshot" "sound" "source" "specials" "statistics" "status"
   "stone" "store_gold" "store_locations" "store_starting_location"
   "store_unit" "story" "target" "team" "teleport" "teleport_anim" "terrain"
   "terrain_graphics" "test" "theme" "then" "tile" "time" "time_area"
   "time_of_day" "topic" "toplevel" "trait" "turn" "tutorial" "unhide_unit"
   "unit" "unit" "unit_abilities" "unit_alignment" "unit_description"
   "unit_hp" "unit_image" "unit_level" "unit_moves" "unit_overlay"
   "unit_profile" "unit_status" "unit_traits" "unit_type" "unit_weapons"
   "unit_xp" "units" "unstone" "unstore_unit" "upkeep" "variable" "variables"
   "village" "villages" "while")
  "A list of all tags available in WML.  This list is used by
  `wesnoth-insert-tag' to provide completion for tag names and
  `wesnoth-check-tag-names' to check for possible errors.")

(defun wesnoth-insert-tag ()
  "Inserts the specified opening and closing tag, positioning
point on the line between them.  Completion of tags at the prompt
is available via the variable `wesnoth-tags-list', which provides
a list of all possible WML tags."
  (interactive)
  (let ((tagname
	 (completing-read
	  "Tag: "
	  wesnoth-tags-list
	  nil nil)))
    (progn
      (insert "[" tagname "]")
      (wesnoth-indent-line)
      (insert "\n")
      (wesnoth-indent-line)
      (save-excursion
	(insert "\n[/" tagname "]")
	(wesnoth-indent-line)
	(forward-line -1)))))

(defun wesnoth-jump-forward-tag (repeat)
  "Moves point to the end of the next tag.
REPEAT is an optional numeric argument.  If REPEAT is non-nil,
jump forward the specified number of tags."
  (interactive "p")
  (or repeat (setq repeat 1))
  (when (< repeat 0)
    (wesnoth-jump-backward-tag (abs repeat)))
  (let ((iterations 0))
    (while (< iterations repeat)
      (end-of-line)
      (search-forward-regexp "^[\t ]*\\[\\w+\\]" (buffer-size) t)
      (setq iterations (1+ iterations)))))

(defun wesnoth-jump-backward-tag (repeat)
  "Moves point to the beginning of the previous tag.
REPEAT is an optional numeric argument.  If REPEAT is non-nil,
jump backward the specified number of tags."
  (interactive "p")
  (or repeat (setq repeat 1))
  (when (< repeat 0)
    (wesnoth-jump-forward-tag (abs repeat)))
  (let ((iterations 0))
    (while (< iterations repeat)
      (beginning-of-line)
      (search-backward-regexp "^[\t ]*\\[\\w+\\]" 0 t)
      (unless (bobp)
	(search-forward-regexp "[^[:blank:]]")
	(backward-char))
      (setq iterations (1+ iterations)))))

(defun wesnoth-jump-to-matching ()
  "Jump point to the matching opening/closing tag.  If you did not
receive the expected results, you may need to run 
`wesnoth-check-tag-structure' and correct any errors detected.
A tag must be on the same line as point for jumping to occur."
  (interactive)
  (let ((open-tags 0)
	(search-started nil)
	(tag-position nil)
	(search-backward nil))
    (save-excursion
      (beginning-of-line)
      (if (looking-at "^[\t ]*\\[")
	  (when (looking-at "^[\t ]*\\[/")
	    (setq search-backward t))
	(error "Current line does not contain a tag for matching"))
      (if search-backward
	  (progn
	    (end-of-line)
	    (while (and
		    (or (< open-tags 0) (not search-started))
		    (search-backward-regexp "^[\t ]*\\[" (point-min) t))
	      (setq search-started t)
	      (if (looking-at "^[\t ]*\\[\\w+\\]")
		  (setq open-tags (1+ open-tags))
		(when (looking-at "^[\t ]*\\[/\\w+\\]")
		    (setq open-tags (1- open-tags))))))
	(while (and
		(or (> open-tags 0) (not search-started))
		(search-forward-regexp "^[\t ]*\\[" (point-max) t))
	  (beginning-of-line)
	  (setq search-started t)
	  (if (looking-at "^[\t ]*\\[\\w+\\]")
	      (setq open-tags (1+ open-tags))
	    (when (looking-at "^[\t ]*\\[/\\w+\\]")
		(setq open-tags (1- open-tags))))
	  (end-of-line)))
      (setq tag-position (point)))
    (goto-char tag-position))
  (end-of-line)
  (search-backward "["))

(defun wesnoth-indent-string ()
  "Indentation to the first column is performed on the current
line if it is part of a multi-line string.  Otherwise, no
indentation is performed."
  (when (nth 3 (syntax-ppss (point)))
    (indent-line-to 0)))

(defun wesnoth-wml-start-pos ()
  "wesnoth-wml-start-pos is used in determining the position of
`point' relative to where the actual WML begins within
wesnoth-mode indentation styles.  wesnoth-wml-start-pos returns
the position the likely starting position of WML code if it is
found, otherwise it returns a position which will always exceed
`point'."
  (save-excursion
    (goto-char (point-min))
    (if (search-forward-regexp
	 "^[\t ]*\\(\\[\\)\\|\\(#\\(?:define\\|e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\|\\(?:if\\|un\\)def\\) \\)"
	 (buffer-size) t)
	(progn
	  (beginning-of-line)
	  (point))
      (1+ (buffer-size)))))

(defun wesnoth-indent-line-default ()
  "Indent the current line as WML using normal-style indentation."
  (beginning-of-line)
  (if (<= (point) (wesnoth-wml-start-pos))
      (indent-line-to 0)
    (let ((not-indented t) cur-indent)
      (if (looking-at "^[ \t]*\\(\\[\\/[^]]*?\\|#enddef\\|#endif\\|#else\\)")
	  (progn
	    (save-excursion
	      (search-backward-regexp "^[ \t]*\\[")
	      (setq cur-indent (current-indentation))
	      (when (looking-at "^[ \t]*\\[/.+\\]")
		(setq cur-indent (- (current-indentation) wesnoth-base-indent))))
	    (if (< cur-indent 0)
		(setq cur-indent 0)))
	(if (not (looking-at "^[ \t]*\\(\\[[^/]*?\\]\\|#define\\|#ifn?\\|#else\\)"))
	    (save-excursion
	      (search-backward-regexp "^[\t ]*\\(\\[\\|#define\\|#ifn?\\|#else\\|#undef\\|#enddef\\|#endif\\)")
	      (progn
		(setq cur-indent (current-indentation))
		(setq not-indented nil)))
	  (save-excursion
	    (while not-indented
	      (search-backward-regexp "^[\t ]*\\([[#}]\\)")
	      (if (looking-at
		   "^[ \t]*\\(\\[[^/]*?\\]\\|#define\\|#if\\|#else\\)")
		  (progn
		    (setq cur-indent (+ (current-indentation) wesnoth-base-indent))
		    (setq not-indented nil))
		(if (bobp)
		    (setq not-indented nil)
		  (progn
		    (setq cur-indent (current-indentation))
		    (setq not-indented nil))))))))
      (if cur-indent
	  (indent-line-to cur-indent)
	(indent-line-to 0))))
  (wesnoth-indent-string)
  (end-of-line))

(defun wesnoth-indent-line-savefile (&optional goto-eol)
  "Indent the current line as WML code using savefile-style indentation."
  (beginning-of-line)
  (if (<= (point) (wesnoth-wml-start-pos))
      (indent-line-to 0)
    (let ((not-indented t) cur-indent)
      (if (looking-at "^[ \t]*\\(\\[\\/[^]]*?\\|#enddef\\|#endif\\|#else\\)")
	  (progn
	    (save-excursion
	      (search-backward-regexp "^[ \t]+.\\|^[{[#]")
	      (setq cur-indent (- (current-indentation) 2))
	      (when (looking-at "^[ \t]*\\(\\[[^/].+\\]\\|#define\\|#ifdef\\|#else\\)")
		(setq cur-indent (current-indentation))))
	    (if (< cur-indent 0)
		(setq cur-indent 0)))
	(save-excursion
	  (while not-indented
	    (search-backward-regexp "^[\t ]*\\([[#}]\\)")
	    (if (looking-at
		 "^[ \t]*\\(\\[[^/]+?\\]\\|#define\\|#if\\|#else\\)")
		(progn
		  (setq cur-indent (+ (current-indentation) 2))
		  (setq not-indented nil))
	      (if (bobp)
		  (setq not-indented nil)
		(progn
		  (setq cur-indent (current-indentation))
		  (setq not-indented nil)))))))
      (if cur-indent
	  (indent-line-to cur-indent)
	(indent-line-to 0))))
  (wesnoth-indent-string)
  (end-of-line))

(defun wesnoth-newline ()
  "Indents both the current line and the newline created if
wesnoth-auto-indent is non-nil.  Otherwise, just create a
newline."
  (interactive)
  (when wesnoth-auto-indent
    (save-excursion
      (beginning-of-line)
      (if (looking-at "^[\t ]*$")
	  (indent-line-to 0)
	(wesnoth-indent-line))))
  (newline))

(defun wesnoth-newline-and-indent (&optional goto-eol)
  "Indents both the current line and the newline created if
wesnoth-auto-indent is non-nil.  Otherwise, just create a newline
and indent it."
  (interactive)
  (wesnoth-newline)
  (wesnoth-indent-line))

(defun wesnoth-check-tag-names ()
  "Checks the names of all tags in the buffer for presence in `wesnoth-tags-list'.
If a tag is found which is not present in the list an error will
be signalled and point will be moved to the corresponding
position.  If the tag exists in WML, it should be added to
`wesnoth-tags-list'."
  (interactive)
  (let ((tag-position nil)
	(missing-tag-name nil))
    (save-excursion
      (goto-char (point-min))
      (while (and
	      (search-forward-regexp "^[\t ]*\\[" (point-max) t)
	      (not tag-position))
	(beginning-of-line)
	(when (looking-at "^[\t ]*\\[[/]?\\(\\w+\\)\\]")
	  (unless (member (match-string-no-properties 1) wesnoth-tags-list)
	    (setq tag-position (point))
	    (setq missing-tag-name (match-string-no-properties 1))))
	(end-of-line)))
    (if tag-position
	(progn
	  (goto-char tag-position)
	  (error "'%s' could not be found in wesnoth-tags-list" missing-tag-name))
      (message "All tag names appear to exist."))))

(defun wesnoth-check-tag-structure ()
  "Checks the buffer for correct nesting of tags.  If a problem
is found in the tag structure, point will be placed at the
location which the tag was expected and the expected tag will be
displayed in the minibuffer."
  (interactive)
  (let ((unmatched-tag-list '())
	(error-position nil))
    (save-excursion
      (goto-char (point-min))
      (while (and
	      (search-forward-regexp "^[\t ]*\\[" (point-max) t)
	      (not error-position))
	(beginning-of-line)
	(if (looking-at "^[\t ]*\\[\\(\\w+\\)\\]")
	    (setq unmatched-tag-list
		  (cons (match-string-no-properties 1) unmatched-tag-list))
	  (when (looking-at "^[\t ]*\\[/\\(\\w+\\)\\]")
	    (if (string= (match-string-no-properties 1) (car unmatched-tag-list))
		(setq unmatched-tag-list (cdr unmatched-tag-list))
	      (setq error-position (point)))))
	(end-of-line)))
    (if error-position
	(progn
	  (goto-char error-position)
	  (error "Expecting '[/%s]'" (car unmatched-tag-list)))
      (message "Tag structure check successful."))))

(defun wesnoth-indent-line ()
  "Determine which indentation function to use and use that style
of indentation on the current line."
  (interactive)
  (if wesnoth-indent-style-savefile
      (wesnoth-indent-line-savefile)
    (wesnoth-indent-line-default)))

(define-derived-mode wesnoth-mode fundamental-mode "wesnoth-mode"
  "Major mode for editing WML."
  (set-syntax-table wesnoth-syntax-table)
  (set (make-local-variable 'outline-regexp) "^[\t ]*\\[\\w+")
  (set (make-local-variable 'comment-start) "#")
  (set (make-local-variable 'indent-line-function) 'wesnoth-indent-line)
  (set (make-local-variable 'font-lock-defaults)
       '(wesnoth-font-lock-keywords
	 nil t nil nil
	 (font-lock-syntactic-keywords . wesnoth-syntactic-keywords)))
  (setq indent-tabs-mode nil)	;; Only use spaces for indents
  (setq mode-name "WML")
  (run-hooks 'wesnoth-mode-hook))

(provide 'wesnoth-mode)
