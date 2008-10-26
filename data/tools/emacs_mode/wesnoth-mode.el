;;; wesnoth-mode.el --- A major mode for editing WML.
;; Copyright (C) 2006, 2007, 2008 Chris Mann

;; This file is part of wesnoth-mode.

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
;; of Wesnoth Markup Language (WML) files.  Currently, this major-mode
;; features syntax highlighting support, automatic indentation,
;; context-sensitive completion and WML checking.

;;; Commentary:
;; Add the following to your .emacs:
;;   (add-to-list 'load-path "path/to/wesnoth-mode")
;;   (autoload 'wesnoth-mode "wesnoth-mode" "Major mode for editing WML." t)
;; Optionally adding:
;;   (add-to-list 'auto-mode-alist '("\\.cfg\\'" . wesnoth-mode))
;; to automatically load wesnoth-mode for all files ending in '.cfg'.

;;; History:
;; 1.3.1
;; * Completion history available is now specific to wesnoth-mode.
;; * Added binding to explicitly update macro information from the current
;;   buffer (C-c C-u).
;; * Significantly improved performance of completion and WML checking.
;; * Improved performance for inserting missing tags.
;; * Fixed a bug where #ifdef was never matched when checking WML.
;; * Added completion for preprocessor statements.
;; * Improved macro completion and checking.
;; 1.3.0
;; * Added support for Xemacs.
;; * WML checking is now context sensitive; checks attributes and macros.
;; * WML checks are now always performed on the entire buffer, with results
;;   displayed in a temporary buffer.
;; * Context-sensitive completion for attributes and tags.
;; * Completion for built-in and project-specific macros.
;; * Changed the following bindings:
;;   `wesnoth-insert-tag' - C-c e -> C-c t
;;   `wesnoth-jump-to-matching' - C-c m -> C-c o
;;   `wesnoth-check-structure' -> `wesnoth-check-wml' - C-c c
;; * Added the following bindings:
;;   `wesnoth-complete-attribute' - C-c a
;;   `wesnoth-complete-macro' - C-c m
;;   `wesnoth-complete-tag' - C-c t
;; * Removed the following bindings:
;;   `wesnoth-check-tag-names' - C-c n
;; * Removed `wesnoth-check-tag-names'.  Replaced by `wesnoth-check-wml'.
;; * Completion for an incomplete attribute, tag, or macro at point is
;;   attempted via TAB.
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
(eval-when-compile
  (require 'cl))
(require 'easymenu)
(require 'wesnoth-update)
(require 'wesnoth-wml-data)

(defconst wesnoth-mode-version "1.3.1"
  "The current version of `wesnoth-mode'.")

(defgroup wesnoth-mode nil "Wesnoth-mode access"
  :group 'languages
  :prefix "wesnoth-")

(defcustom wesnoth-auto-indent-flag t
  "Non-nil means indent the current line upon creating a newline."
  :type 'boolean
  :group 'wesnoth-mode)

(defcustom wesnoth-indent-savefile t
  "Non-nil means to use the current indentation conventions.
If nil, use the old convention for indentation.
The current convention is all attributes are indented a level deeper
than their parent; in the past attributes were indented to the same
level as their parent.")

(defcustom wesnoth-base-indent 4
  "The number of columns to indent WML."
  :type 'integer
  :group 'wesnoth-mode)

(defconst wesnoth-preprocessor-regexp
  "[\t ]*#\\(enddef\\|define\\|e\\(lse\\|nd\\(\\(de\\|i\\)f\\)\\)\\|\\(ifn?\\|un\\)def\\)"
  "Regular expression to match all preprocessor statements.")

(defconst wesnoth-preprocessor-opening-regexp
  "[\t ]*#\\(define \\|else\\|ifdef \\|ifndef \\)"
  "Regular expression to match \"opening\" preprocessor statements.")

(defconst wesnoth-preprocessor-closing-regexp
  "[\t ]*#\\(end\\(\\(de\\|i\\)f\\)\\)"
  "Regular expression to match \"closing\" preprocessor statements.")

(defvar wesnoth-define-blocks '()
  "Cache of all toplevel #define and #enddef pairs.")

(defvar wesnoth-history-list '()
  "History of inserted WML elements.")

(defvar wesnoth-mode-hook nil)

(defvar wesnoth-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "C-M-a") 'wesnoth-backward-element)
    (define-key map (kbd "C-M-e") 'wesnoth-forward-element)
    (define-key map (kbd "C-m") 'wesnoth-newline)
    (define-key map (kbd "C-j") 'wesnoth-newline-and-indent)
    (define-key map (kbd "C-c C-c") 'wesnoth-check-wml)
    (define-key map (kbd "C-c C-a") 'wesnoth-complete-attribute)
    (define-key map (kbd "C-c C-t") 'wesnoth-complete-tag)
    (define-key map (kbd "C-c C-p") 'wesnoth-complete-preprocessor)
    (define-key map (kbd "C-c C-u") 'wesnoth-update-project-information)
    (define-key map (kbd "M-TAB") 'wesnoth-complete-tag)
    (define-key map (kbd "C-c C-m") 'wesnoth-complete-macro)
    (define-key map (kbd "C-c C-o") 'wesnoth-jump-to-matching)
    (define-key map (kbd "C-c C-/") 'wesnoth-insert-missing-closing)
    (define-key map (kbd "TAB") 'wesnoth-indent-or-complete)
    map)
  "Keymap used in `wesnoth-mode'.")

(easy-menu-define wesnoth-menu wesnoth-mode-map "Menu for wesnoth-mode"
  '("WML"
    ["Check WML" wesnoth-check-wml t]
    ["Indent or Complete" wesnoth-indent-or-complete t]
    ["Indent buffer" (lambda ()
		       (interactive)
		       (wesnoth-indent-region (point-min) (point-max))) t]
    ["Insert Tag" wesnoth-complete-tag t]
    ["Insert Attribute" wesnoth-complete-attribute t]
    ["Insert Macro" wesnoth-complete-macro t]
    ["Insert Preprocessor" wesnoth-complete-preprocessor t]
    ["Insert Missing Tag" wesnoth-insert-missing-closing t]
    ["Jump to Matching" wesnoth-jump-to-matching t]
    ["Update Macros" wesnoth-update-project-information t]))

(defvar wesnoth-syntax-table
  (let ((wesnoth-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?= "." wesnoth-syntax-table)
    (modify-syntax-entry ?_ "_" wesnoth-syntax-table)
    (modify-syntax-entry ?- "_" wesnoth-syntax-table)
    (modify-syntax-entry ?. "_" wesnoth-syntax-table)
    (modify-syntax-entry ?\n ">" wesnoth-syntax-table)
    (modify-syntax-entry ?\r ">" wesnoth-syntax-table)
    wesnoth-syntax-table)
  "Syntax table for `wesnoth-mode'.")

;; Prevents automatic syntax-highlighting of elements which might be
;; pre-processor statements.
(defvar wesnoth-syntactic-keywords
  (list
   '("\\(^[\t ]*\\(#\\(?:define \\|e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\|\\(?:ifn?\\|un\\)def \\)\\)\\)" 1 "w")
   '("\\(#[\t ]*.*$\\)" 1 "<"))
  "Highlighting syntactic keywords within `wesnoth-mode'.")

(defvar wesnoth-font-lock-keywords
  (list
   '("#\\(?:define\\|\\(?:ifn?\\|un\\)def\\)" . font-lock-keyword-face)
   '("\\(#e\\(?:lse\\|nd\\(?:\\(?:de\\|i\\)f\\)\\)\\)" .
     font-lock-keyword-face)
   '("\\(#\\(?:define\\|\\(?:ifn?\\|un\\)def\\)\\)[\t ]+\\(\\(\\w\\|_\\)+\\)"
     2 font-lock-function-name-face)
   '("\\({[@~]?\\(\\w\\|\\.\\|/\\|-\\)+}\\)" (1 font-lock-function-name-face))
   '("\\({\\(\\w\\|:\\|_\\)+\\|{[~@]?\\)" (1 font-lock-function-name-face))
   '("}" .  font-lock-function-name-face)
   '("^[\t ]*\\(\\[[^]]+\\]\\)" 1 font-lock-type-face)
   '("\\$\\(\\w\\|_\\)+" . font-lock-variable-name-face)
   '("\\(\\(\\w\\|_\\)+\\(\\,[\t ]*\\(\\w\\|_\\)+\\)*\\)="
     1 font-lock-variable-name-face))
  "Syntax highlighting for `wesnoth-mode'.")

(defun wesnoth-element-closing (&optional limited)
  "Return the regexp to match a closing element.
If LIMITED is non-nil, return a regexp which matches only the
#enddef preprocessor."
  (concat "^[\t ]*\\(\\[/\\(\\w\\|_\\)+\\|"
	  (if limited
	      "#enddef"
	    "#end\\(?:def\\|if\\)")
	  "\\)"))

(defun wesnoth-element-opening (&optional limited)
  "Return the regexp to match a closing element.
If LIMITED is non-nil, return a regexp which matches only the
#define preprocessor."
  (concat "^[\t ]*\\(\\[\\+?\\(\\w\\|_\\)+\\]\\|#define "
	  (if limited
	      ""
	    "\\|#ifn?def ")
	  "\\)"))

(defun wesnoth-element (&optional limited)
  "Return the regexp to match a closing element.
If LIMITED is non-nil, return a regexp which matches only the
#define and #enddef preprocessors."
  (concat "^[\t ]*\\(\\[[/+]?\\(\\w\\|_\\)+\\]?\\|"
	  (if limited
	      "#define \\|#enddef"
	    (substring wesnoth-preprocessor-regexp 5))
	  "\\)"))


;;; Insertion and completion
(defmacro wesnoth-element-completion (completions prompt partial)
  "Process completion of COMPLETIONS, displaying PROMPT.
PARTIAL is the partial string on which to attempt completion."
  `(let* ((element (when ,partial (try-completion ,partial ,completions))))
     (cond ((eq element t)
	    (setq element nil))
	   ((null element)
	    (setq element
		  (completing-read ,prompt ,completions nil nil nil
				   'wesnoth-history-list)))
	   ((not (if (listp (car ,completions))
		     (assoc element ,completions)
		   (member element ,completions)))
	    (setq element
		  (completing-read ,prompt ,completions
				   nil nil ,partial
				   'wesnoth-history-list))))
     element))

(defun wesnoth-parent-tag ()
  "Return the name of the parent tag.
If the parent is a preprocessor statement, return non-nil.
If the element does not have a parent, return nil.
Otherwise, return a string containing the name of the parent tag."
  (save-excursion
    (let ((start-point (point))
	  (depth 1))
      (when (save-excursion (> (point) (progn (back-to-indentation)
					      (point))))
	(end-of-line))
      (while (and (> depth 0)
		  (search-backward-regexp (wesnoth-element)
					  (point-min) t))
	(if (string-match "[\t ]*\\[/" (match-string 0))
	    (incf depth)
	  (decf depth)))
      (beginning-of-line)
      (if (> depth 0)
	  nil
	(when (looking-at (wesnoth-element-opening))
	  (let ((parent (match-string-no-properties 1)))
	    (if (string-match wesnoth-preprocessor-opening-regexp parent)
		t
	      (substring parent 1 (1- (length parent))))))))))

(defun wesnoth-indent-or-complete (&optional elements)
  "Indent or complete the line at point, depending on context.
ELEMENTS is the number of elements to wrap around if inserting
matching tags."
  (interactive "P")
  (or elements (setq elements 0))
  (let ((target nil))
    (save-excursion
      (back-to-indentation)
      (cond ((looking-at "\\(\\(\\w\\|_\\)+\\)[\t ]*$")
	     (wesnoth-complete-attribute t))
	    ((looking-at "\\[\\(\\(\\w\\|_\\)*\\)[\t ]*$")
	     (wesnoth-complete-tag elements t))
	    ((looking-at "{\\(\\(\\w\\|_\\)*\\)[\t ]*$")
	     (wesnoth-complete-macro t))
	    ((looking-at "^#\\w+$")
	     (wesnoth-complete-preprocessor elements t))
	    ((looking-at "\\[/\\(\\(\\w\\|_\\)*\\)[\t ]*$")
	     (delete-region (point) (progn (end-of-line) (point)))
	     (wesnoth-insert-missing-closing)
	     (end-of-line))
	    (t
	     (wesnoth-indent)))
      (setq target (point)))
    (goto-char target)))

(defun wesnoth-preprocessor-closed-p (preprocessor)
  "Determine whether PREPROCESSOR has been closed.
PREPROCESSOR is a string matching the preprocessor statement to
be inserted."
  (save-excursion
    (back-to-indentation)
    (wesnoth-jump-to-matching preprocessor)
    (looking-at
     (if (string= preprocessor "#define ")
	 "#enddef"
       "#endif"))))

(defun wesnoth-complete-preprocessor (&optional elements completep)
  "Complete and insert the preprocessor at point.
ELEMENTS is the number of elements to wrap around.
If COMPLETEP is non-nil, attempt to complete preprocessor at point."
  (interactive "P")
  (or elements (setq elements 0))
  (let* ((completions (wesnoth-emacs-completion-formats
		       '("define" "else" "ifdef" "ifndef"
			 "enddef" "endif" "undef")))
	 (partial (when completep
		    (save-excursion
		      (back-to-indentation)
		      (when (looking-at "#\\(\\w+\\)$")
			(match-string-no-properties 1)))))
	 (preprocessor (or (wesnoth-element-completion
			    completions "Preprocessor: " partial)
			   partial))
	 (closedp
	  (save-excursion
	    (when preprocessor
	      (unless (string= "#" (substring preprocessor 0 1))
		(setq preprocessor (concat "#" preprocessor)))
	      (when (string-match "#\\(define\\|ifn?def\\|undef\\)" preprocessor)
		(setq preprocessor (concat preprocessor " ")))
	      (when partial
		(delete-region (progn (back-to-indentation) (point))
			       (progn (end-of-line) (point))))
	      (wesnoth-preprocessor-closed-p preprocessor)))))
    (when preprocessor
      (when partial
	(delete-region (progn (back-to-indentation) (point))
		       (progn (end-of-line) (point))))
      (if (and (string-match "#\\(define \\|ifn?def\\)" preprocessor)
	       (not closedp))
	  (progn
	    (wesnoth-insert-tag elements preprocessor)
	    (forward-line -1)
	    (end-of-line))
	(wesnoth-insert-element-separately preprocessor)))))

(defun wesnoth-macro-arguments ()
  "Find any current macro arguments."
  (let ((results '())
	(depth (wesnoth-within-define (point))))
    (save-excursion
      (while (> depth 0)
	(save-match-data
	  (search-backward-regexp
	   "[\t ]*#define \\(?:\\w+\\|_\\)*\\(\\([\t ]*\\(\\w\\|_\\)+\\)*\\)"
	   (point-min) t)
	  (when (<= (wesnoth-within-define (point)) depth)
	    (and (> depth 0)
		 (setq results
		       (append (mapcar (lambda (macro)
					 (list macro nil))
				       (split-string
					(match-string-no-properties 1)))
			       results)))
	    (decf depth)))))
    (message "%s" results)
    results))

(defun wesnoth-complete-macro (&optional completep)
  "Complete and insert the macro at point.
If COMPLETEP is non-nil, attempt to complete the macro at point."
  (interactive)
  (wesnoth-update-project-information)
  (let* ((macro-information (append (wesnoth-macro-arguments)
				    wesnoth-macro-data
				    wesnoth-local-macro-data))
	 (completions (wesnoth-emacs-completion-formats
		       (mapcar 'car macro-information)))
	 (partial (when completep
		    (save-excursion
		      (back-to-indentation)
		      (when (looking-at "{\\(\\(\\w\\|_\\)*\\)")
			(match-string-no-properties 1)))))
	 (macro (or (wesnoth-element-completion completions "Macro: " partial)
		    partial))
	 (args (cadr (assoc macro macro-information))))
    (when macro
      (if partial
	  (progn
	    (delete-region (progn (back-to-indentation) (point))
			   (progn (end-of-line) (point)))
	    (insert "{" macro (if args " }" "}")))
	(wesnoth-insert-element-separately "{" macro (if args " }" "}")))
      (save-excursion
	(wesnoth-indent))
      (when args
	(forward-char -1)))))

(defun wesnoth-complete-attribute (&optional completep)
  "Insert the attribute at point.
If COMPLETEP is non-nil, attempt to complete the attribute at point."
  (interactive)
  (let* ((completions (wesnoth-build-completion 2))
	 (partial (when completep
		    (save-excursion
		      (back-to-indentation)
		      (when (looking-at "\\(\\(\\w\\|_\\)+\\)")
			(match-string-no-properties 1)))))
	 (attribute (or (wesnoth-element-completion completions "Attribute: "
						    partial)
			partial)))
    (when attribute
      (if (and partial completep)
	  (progn
	    (delete-region (progn (back-to-indentation) (point))
			   (progn (end-of-line) (point)))
	    (insert attribute "="))
	(wesnoth-insert-element-separately attribute
					   (if (string-match "=" attribute)
					       ""
					     "=")))
      (save-excursion
	(wesnoth-indent)))))

(defun wesnoth-complete-tag (&optional elements completep)
  "Complete and insert the tag at point.
ELEMENTS is the number of elements to wrap around.
If COMPLETEP is non-nil, attempt to complete tag at point."
  (interactive "P")
  (or elements (setq elements 0))
  (let* ((completions (wesnoth-build-completion 1))
	 (partial (when completep
		    (save-excursion
		      (back-to-indentation)
		      (when (looking-at "\\[\\(\\(\\w\\|_\\)+\\)")
			(match-string-no-properties 1)))))
	 (tag (or (wesnoth-element-completion completions "Tag: " partial)
		  partial))
	 (closedp
	  (save-excursion
	    (wesnoth-jump-to-matching)
	    (back-to-indentation)
	    (and (looking-at "\\[/\\(\\(\\w\\|_\\)+\\)")
		 (string= tag (match-string 1))))))
    (when completep
      (delete-region (progn (back-to-indentation) (point))
		     (progn (end-of-line) (point))))
    (if (and closedp completep)
	(progn
	  (wesnoth-insert-and-indent "[" tag "]")
	  (end-of-line))
      (wesnoth-insert-tag elements tag))))

(defun wesnoth-build-completion (position)
  "Create a new list for tag completion if necessary.
Rebuilding list is required for versions of GNU Emacs earlier
than 22.  POSITION is the argument passed to `nth' for
`wesnoth-tag-data'."
  (interactive "P")
  (let* ((parent (wesnoth-parent-tag))
	 (candidates
	  (if (or (stringp parent) (null parent))
	      (nth (1- position) (gethash parent wesnoth-tag-hash-table))
	    (mapcar 'car wesnoth-tag-data))))
    (wesnoth-emacs-completion-formats candidates)))

(defun wesnoth-emacs-completion-formats (candidates)
  "Return the completions in the correct format for `emacs-major-version'.
CANDIDATES is a list of all possible completions."
  (if (> emacs-major-version 21)
      candidates
    (let ((tags '())
	  (iter 0))
      (dolist (tag candidates)
	(setq iter (1+ iter))
	(setq tags (append tags (list (cons tag iter)))))
      tags)))

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
    (setq tagname (completing-read "Tag: " (wesnoth-build-completion 1)
				   nil nil nil wesnoth-history-list)))
  (when (or (not elements)
	    (looking-at (concat "[\t ]*\\(:?\\[/\\|"
				wesnoth-preprocessor-regexp "\\)")))
    (setq elements 0))
  (let ((depth 0)
	(start (save-excursion (forward-line -1) (point)))
	(end (unless (= elements 0)
	       (wesnoth-nth-pair-position elements))))
    (if (string-match wesnoth-preprocessor-regexp tagname)
	(wesnoth-insert-element-separately tagname)
      (wesnoth-insert-element-separately "[" tagname "]"))
    (save-excursion
      (if end
	  (goto-char (marker-position end))
	(newline (if (string-match wesnoth-preprocessor-regexp tagname) 1 2)))
      (if (string-match wesnoth-preprocessor-opening-regexp tagname)
	  (wesnoth-insert-element-separately
	   (if (string= tagname "#define ")
	       "#enddef"
	     "#endif"))
	(wesnoth-insert-element-separately "[/" tagname "]"))
      (indent-region start (point) nil))
    (unless end
      (forward-line 1)))
  (wesnoth-indent))

(defun wesnoth-insert-element-separately (&rest strings)
  "Concatenate STRINGS and insert them on a line of their own."
  (if (save-excursion (and (> (point) (progn (back-to-indentation) (point)))))
      (if (save-excursion (forward-line 1) (looking-at "^[\t ]*$"))
	  (progn
	    (forward-line 1)
	    (end-of-line))
	(end-of-line)
	(newline))
    (beginning-of-line)
    (if (looking-at "^[\t ]*$")
	(end-of-line)
      (open-line 1)))
  (insert (apply 'concat strings)))

(defun wesnoth-insert-missing-closing (&optional start end)
  "Insert the next expected closing element at point.

START and END define the region to check for missing closing
elements.  If function `transient-mark-mode' is enabled, the region
specified will be used as START and END.  Otherwise, START and
END will be the minimum and current positions of the buffer,
respectively."
  (interactive)
  (if (and (boundp 'transient-mark-mode)
	   transient-mark-mode mark-active)
      (setq start (region-beginning)
	    end (copy-marker (region-end)))
    (setq start (point-min)
	  end (point)))
  (let ((element (wesnoth-check-structure start end)))
    (if (not element)
	(error "%s" "Unable to find element to insert")
      (when (string= element "Unexpected end of file")
	(error "%s" element))
      (wesnoth-insert-element-separately element)))
  (wesnoth-indent)
  (end-of-line))

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
       (funcall ,search-function (wesnoth-element-opening) ,bound t)
       (back-to-indentation)
       (decf ,repeat))))

(defun wesnoth-nth-pair-position (count)
  "Return `point' after COUNT number of matching element pairs.
COUNT is a positive number representing the number of balanced
pairs to move across.
`point' is returned as a marker object."
  (save-excursion
    (let ((start (point))
	  (failed nil))
      (if (> (point) (save-excursion (back-to-indentation) (point)))
	  (end-of-line)
	(beginning-of-line))
      (while (> count 0)
	;; Currently looking-at target tag.  Stop here to avoid
	;; incorrect nesting.
	(unless (wesnoth-search-for-matching-tag
		 'search-forward-regexp (wesnoth-element-closing) 'point-max)
	  (setq count 0)
	  (unless (or (= (point) (point-max))
		      (progn (beginning-of-line)
			     (search-backward-regexp (wesnoth-element-closing)
						     start t)))
	    (setq failed t)))
	(and (> (decf count) 0) (forward-line 1)))
      (if failed
	  (beginning-of-line)
	(end-of-line))
      (point-marker))))

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

(defmacro wesnoth-search-for-matching-tag (search-function
					   search-string bound &optional skip)
  "Search for the matching tag for the current line.

SEARCH-FUNCTION is the name of the function used to perform the search.
SEARCH-STRING is a string representing the matching tag type.
BOUND is the bound to be passed to the search function.
If SKIP is non-nil, skip the first element and continue from there."
  `(let ((depth 1))
     (when (or (and ,skip (forward-line 1))
	       (funcall ,search-function (wesnoth-element) (funcall ,bound) t))
       (when (or ,skip (not (string-match ,search-string (match-string 0))))
	 (while (and (> depth 0)
		     (funcall ,search-function (wesnoth-element)
			      (funcall ,bound) t))
	   (if (string-match ,search-string (match-string 0))
	       (decf depth)
	     (incf depth)))
	 (= depth 0)))))

(defun wesnoth-jump-to-matching (&optional opening)
  "Jump point to the matching opening/closing tag.
OPENING is an opening preprocessor statement to attempt to find a match for."
  (interactive)
  (beginning-of-line)
  (let ((target nil)
	(first-element nil))
    (save-excursion
      (if (or (and (stringp opening)
		   (string-match (wesnoth-element-opening) opening))
	      (looking-at (wesnoth-element-opening)))
	  (progn
	    (setq first-element (match-string-no-properties 0 opening))
	    (when (wesnoth-search-for-matching-tag
		   'search-forward-regexp (wesnoth-element-closing) 'point-max
		   (stringp opening))
	      (beginning-of-line)
	      (if (and (string-match wesnoth-preprocessor-opening-regexp
				     first-element)
		       (looking-at (wesnoth-element-closing)))
		  (when (string= (match-string-no-properties 0)
				 (cdr (assoc first-element
					     '(("#define " . "#enddef")
					       ("#ifndef " . "#endif")
					       ("#ifdef " . "#endif")))))
		    (setq target (point)))
		(setq target (point)))))
	(when (looking-at (wesnoth-element-closing))
	  (setq first-element (match-string-no-properties 0))
	  (end-of-line)
	  (when (wesnoth-search-for-matching-tag
		 'search-backward-regexp (wesnoth-element-opening)
		 'wesnoth-wml-start-pos)
	    (if (and (string-match wesnoth-preprocessor-closing-regexp
				   first-element)
		     (looking-at (wesnoth-element-opening)))
		(progn
		  (when (or (and (string= "#enddef" first-element)
				 (string= "#define "
					  (match-string-no-properties 0)))
			    (and (string= "#endif" first-element)
				 (string-match
				  "#ifn?def "
				  (match-string-no-properties 0))))
		    (setq target (point))))
	      (setq target (point)))))))
    (if target
	(goto-char target)
      (when (interactive-p)
	(error "Tag does not appear to be matched"))))
  (back-to-indentation))

;;; Indentation
(defun wesnoth-wml-start-pos ()
  "Determine the position of `point' relative to where the actual WML begins.
Return the likely starting position of the WML if it is found.
Otherwise return nil."
  (save-excursion
    (goto-char (point-min))
    (when (search-forward-regexp (wesnoth-element) (point-max) t)
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
	  (if (or (and wesnoth-indent-savefile
		       (not (looking-at (wesnoth-element-closing t))))
		  (looking-at (wesnoth-element-opening t)))
	      (setq cur-indent (+ ref-indent wesnoth-base-indent))
	    (setq cur-indent ref-indent)))
	 ((eq context 'closing)
	  (if (or (looking-at "^[\t ]*\\[/")
		  (and (not wesnoth-indent-savefile)
		       (not (looking-at (wesnoth-element-opening t)))))
	      (setq cur-indent (- ref-indent wesnoth-base-indent))
	    (setq cur-indent ref-indent))))))
    (indent-line-to (max cur-indent 0))))

(defun wesnoth-within-define (position)
  "Determine whether point is currently inside a #define block.
POSITION is the initial cursor position."
  (save-match-data
    (let ((depth 0))
      (dolist (element (or wesnoth-define-blocks
			   (wesnoth-find-macro-definitions)))
	(when (= (cadr (sort (append (mapcar 'marker-position (cadr element))
				     (list position)) '>)) position)
	  (setq depth (max (car element) depth))))
      depth)))

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
		(if openings
		    (progn
		      (add-to-list 'cache
				   (list depth (list (car openings)
						     (point-marker))))
		      (setq openings (cdr openings))
		      (1- depth))
		  depth)))
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
    (search-backward-regexp (wesnoth-element t)
			    (wesnoth-wml-start-pos) t)
    (let ((match (or (match-string 1) ""))
	  (depth (wesnoth-within-define position)))
      (while (and (> (wesnoth-within-define (point)) depth)
		  (not (= (point) (wesnoth-wml-start-pos))))
	(search-backward-regexp (wesnoth-element t)
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
(defun wesnoth-check-element-type (position last-tag)
  "Determine the context of the element.
POSITION is the position of the element in the list.
LAST-TAG is the parent element."
  (if (or (not last-tag)
	  (save-match-data
	    (string-match "#\\(?:define\\|ifn?def\\)" last-tag)))
      (member (match-string-no-properties 1)
	      (mapcar 'car wesnoth-tag-data))
    (member (match-string-no-properties 1)
	    (nth position (gethash last-tag
				   wesnoth-tag-hash-table)))))

;; Provide `line-number-at-pos' implementation (not available in Emacs 21).
(defun wesnoth-line-number-at-pos (&optional pos)
  "Return (narrowed) buffer line number at position POS.
If POS is nil, use current buffer location.
Counting starts at (point-min), so the value refers
to the contents of the accessible portion of the buffer."
  (let ((opoint (or pos (point))) start)
    (save-excursion
      (goto-char (point-min))
      (setq start (point))
      (goto-char opoint)
      (forward-line 0)
      (1+ (count-lines start (point))))))

(defun wesnoth-check-output (buffer format-string &rest args)
  "Output the string as passed to `format'.
BUFFER is the buffer to output the result.
FORMAT-STRING is the string as the first argument of `format'.
ARGS is any additional data required by `format' to handle FORMAT-STRING."
  (save-excursion
    (let ((lnap (wesnoth-line-number-at-pos)))
      (set-buffer buffer)
      (insert (apply 'format (concat "Line %d: " format-string "\n")
		     lnap args)))))

(defun wesnoth-extract-macro-details (macro-arguments)
  "Return a list of all macros in MACRO-ARGUMENTS."
  (when macro-arguments
    (let ((results '()))
      (save-match-data
	(dolist (macro (split-string macro-arguments "[{}][\t ]*" t))
	  (when (string-match "^\\(\\(?:\\w\\|_\\)+\\)"
			      macro)
	    (add-to-list 'results (match-string-no-properties 1 macro)))))
      results)))

(defun wesnoth-check-wml ()
  "Perform context-sensitive analysis of WML-code."
  (interactive)
  (wesnoth-update-project-information)
  (when (= 0 (hash-table-count wesnoth-tag-hash-table))
    (error "WML data not available; can not generate report"))
  (let ((unmatched '())
	(outbuf (get-buffer-create "*WML*")))
    (save-excursion
      (let ((buffer (buffer-name)))
	(set-buffer outbuf)
	(erase-buffer)
	(insert (format "Checking %s...\n" buffer))
	(message (format "Checking %s..." buffer))))
    (save-excursion
      (goto-char (or (wesnoth-wml-start-pos) (point-min)))
      (while (search-forward-regexp "[\t ]*{\\(\\(\\(\\w\\|_\\)+\\)[^=
]*\\)}" (point-max) t)
	(dolist (macro (wesnoth-extract-macro-details
			(match-string-no-properties 1)))
	  (unless (assoc macro
			 (append (wesnoth-macro-arguments)
				 wesnoth-local-macro-data
				 wesnoth-macro-data))
	    (wesnoth-check-output outbuf "Unknown macro definition: '{%s}'"
				  macro))))
      (goto-char (or (wesnoth-wml-start-pos) (point-min)))
      (while (search-forward-regexp
	      ;; Match tags, preprocessor statements and attributes.
	      (concat "^[\t ]*\\(\\[[+/]?\\([a-z]\\(\\w\\|_\\)+\\)\\]\\|"
		      "\\(\\w\\|_\\)+=\\|"
		      wesnoth-preprocessor-regexp "\\)")
	      (point-max) t)
	(save-excursion
	  (goto-char (match-beginning 1))
	  (cond ((nth 3 (parse-partial-sexp (point-min) (point)))
		 nil)
		((looking-at "[\t ]*\\[\\+?\\(\\(\\w\\|_\\)+\\)\\]")
		 (unless (wesnoth-check-element-type 0 (car unmatched))
		   (wesnoth-check-output outbuf
					 "Tag not available in this context: '%s'"
					 (match-string-no-properties 1)))
		 (setq unmatched (cons (match-string-no-properties 1)
				       unmatched)))
		((looking-at
		  (concat "[\t ]*\\(#define\\|#ifdef\\|#ifndef\\|#undef\\)"
			  "\\( \\(\\w\\|_\\)+\\)*"))
		 (unless (match-string-no-properties 2)
		   (wesnoth-check-output
		    outbuf (concat "Preprocessor statement has no argument: "
				   (match-string-no-properties 1))))
		 (unless (string= (match-string-no-properties 1) "#undef")
		   (setq unmatched (cons (match-string-no-properties 1)
					 unmatched))))
		((looking-at wesnoth-preprocessor-closing-regexp)
		 (when (and unmatched
			    (not (string-match
				  (cdr (assoc (match-string-no-properties 1)
					      '(("enddef" . "#define")
						("endif" . "#ifn?def"))))
				  (car unmatched))))
		   (wesnoth-check-output
		    outbuf
		    "Preprocessor statement does not nest correctly"))
		 (setq unmatched (cdr unmatched)))
		((looking-at "[\t ]*\\(\\(\\w\\|_\\)+\\)=\\(.+\\)?")
		 (unless (wesnoth-check-element-type 1 (car unmatched))
		   (wesnoth-check-output
		    outbuf "Attribute not available in this context: '%s'"
		    (match-string-no-properties 1)))
		 (unless (match-string 3)
		   (wesnoth-check-output
		    outbuf "Attribute has no value")))
		((looking-at "[\t ]*#else")
		 (unless (string-match "ifn?def" (car unmatched))
		   (if (string= (car unmatched) "#define")
		       (wesnoth-check-output outbuf "Expecting: '%s'"
					     (car unmatched))
		     (wesnoth-check-output outbuf "Expecting: '[/%s]'"
					   (car unmatched)))))
		((looking-at "[\t ]*\\[/\\(\\(\\w\\|_\\)+\\)\\]")
		 (when (and unmatched
			    (not (string= (match-string-no-properties 1)
					  (car unmatched))))
		   (wesnoth-check-output outbuf "Expecting: '[/%s]'"
					 (car unmatched)))
		 (setq unmatched (cdr unmatched))))))
      (if unmatched
	  (dolist (tag unmatched)
	    (wesnoth-check-output outbuf "Unmatched element: '%s'"
				  (car unmatched)))))
    (save-excursion
      (let ((buffer (buffer-name)))
	(set-buffer outbuf)
	(display-buffer outbuf t)
	(let ((warnings (- (wesnoth-line-number-at-pos
			    (save-excursion (goto-char (point-max)))) 2)))
	  (insert (format (concat "\nCheck complete.  %d warning"
				  (if (= warnings 1) "." "s.")) warnings)))
	(message (format "Checking %s...done" buffer))))))

(defmacro wesnoth-element-requires (element requirement &optional pop)
  "Process requirements for corresponding preprocessor elements.
ELEMENT is the current element being tested.
REQUIREMENT is the element required to exist for correct nesting.
POP is an optional argument indicating the element should be
removed from the list of unmatched elements."
  `(when (string= ,element (match-string-no-properties 1))

     (if (string-match ,requirement (car unmatched))
	 (progn
	   (and ,pop (setq unmatched (cdr unmatched)))
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
    (if (and (boundp 'transient-mark-mode)
	     transient-mark-mode mark-active)
	(setq start (region-beginning)
	      end (copy-marker (region-end)))
      (setq start (point-min)
	    end (point-max))))
  (let ((unmatched '())
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
	    (setq unmatched (cons (match-string-no-properties 1)
				  unmatched))
	  (cond
	   ((wesnoth-element-requires "#else" "ifn?def "))
	   ((wesnoth-element-requires "#endif" "ifn?def " t))
	   ((wesnoth-element-requires "#enddef" "define " t))
	   ((looking-at (concat "^[\t ]*\\[/\\(\\(\\w\\|_\\)+\\)\\]\\|"
				wesnoth-preprocessor-closing-regexp))
	    (if (string= (match-string-no-properties 1)
			 (car unmatched))
		(setq unmatched (cdr unmatched))
	      (setq error-position (point))))))
	(end-of-line)))
    (wesnoth-structure-result error-position (car unmatched))))

;;; wesnoth-mode
(define-derived-mode wesnoth-mode fundamental-mode "wesnoth-mode"
  "Major mode for editing WML."
  (kill-all-local-variables)
  (use-local-map wesnoth-mode-map)
  (setq major-mode 'wesnoth-mode)
  (setq mode-name "WML")
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
  (easy-menu-add wesnoth-menu wesnoth-mode-map)
  (wesnoth-create-wml-hash-table)
  (wesnoth-update-project-information)
  (run-hooks 'wesnoth-mode-hook))

(provide 'wesnoth-mode)

;;; wesnoth-mode.el ends here
