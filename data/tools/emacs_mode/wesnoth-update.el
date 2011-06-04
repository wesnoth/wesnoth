;;; wesnoth-update.el --- Update known WML data via existing valid WML.

;; Author: Chris Mann

;; This file is part of wesnoth-mode.

;;; Commentary:
;; Update WML information using WML built-in to Wesnoth.

;; The following should be added to your .emacs so that `wesnoth-update' can
;; correctly generate WML data:
;;   (setq wesnoth-root-directory "/path/to/wesnoth/"
;;         wesnoth-update-output-directory "/path/to/wesnoth-mode/"
;;         wesnoth-addition-file
;;         "/path/to/wesnoth-mode/wesnoth-wml-additions.cfg")
;; Specifying the appropriate path in each case.

;; Although WML data is provided along with wesnoth-mode, you can generate
;; update-to-date, version-specific WML reference data for `wesnoth-mode'
;; using `wesnoth-update'.  This requires Wesnoth to be install and its
;; pathname set for this to behave correctly.  for example:
;; (setq wesnoth-root-directory "/usr/share/wesnoth/")

;; Then set the output directory for `wesnoth-update's results:
;; (setq wesnoth-update-output-directory "/path/to/wesnoth-mode/")
;; This is recommended to be in the same directory as `wesnoth-mode' and
;; must be in `load-path'.

;; Once set, `wesnoth-update' will produce 'wesnoth-wml-data.el' in
;; `wesnoth-update-output-directory' and the information will automatically
;; be available in the future sessions.

;; Although much data is retreived, it is unlikely to be completely
;; comprehensive.  wesnoth-mode can be taught about additional tags,
;; attributes and macros using the current project, or a single file, using
;; `wesnoth-update-wml-additions'.

;; To teach wesnoth-mode about elements it may have missed, you can extend the
;; sample additions included with wesnoth-mode; namely
;; wesnoth-wml-additions.cfg (although any source of WML can be used).  To
;; enable this, do the following:
;; Set `wesnoth-addition-file' appropriately, for example:
;; (setq wesnoth-addition-file "/path/to/wesnoth-wml-additions.cfg")

;; Once set correctly, running M-x wesnoth-update will update the WML data
;; available to `wesnoth-mode'.

;;; History:
;; 0.1.6
;; * More verbose output
;; * Fix bug which could cause `wesnoth-update' to hang.
;; 0.1.5
;; * `wesnoth-update' now finds more built-in macros.
;; 0.1.4
;; * Fixed inaccuracies when updating project information.
;; * WML data from the addition file can now read when as it is required.
;; 0.1.3
;; * Any arguments are now stored for each macro.
;; 0.1.2
;; * Allow forced updating of the hash table.
;; * Allow clearing of local macro data via a prefix argument.
;; 0.1.1
;; * Provide means for increased performance when referencing attributes and
;;   tags.
;; * Gather project macro information for the local buffer only, instead of
;;   from files in the directory.
;; 0.1
;; * Initial version

;;; Code:
(require 'pp)

(defvar wesnoth-update-version "0.1.6"
  "Version of `wesnoth-update'.")

(defcustom wesnoth-root-directory nil
  "Root directory of wesnoth."
  :type 'directory
  :group 'wesnoth-mode)

(defcustom wesnoth-addition-file nil
  "Filename to the file containing additional WML information."
  :type 'file
  :group 'wesnoth-mode)

(defcustom wesnoth-update-output-directory nil
  "Directory to write discovered WML syntax information.
Ensure this directory is in your `load-path'."
  :type 'directory
  :group 'wesnoth-mode)

(defvar wesnoth-found-cfgs '()
  "Temporary list of all .cfg files found.")

(defvar wesnoth-tmp-tag-data '()
  "Temporary list of tag data.")

(defvar wesnoth-tmp-macro-data '()
  "Temporary list of macro data.")

(defvar wesnoth-tag-data '()
  "All information regarding the relation of tags and attributes.")

(defvar wesnoth-macro-data '()
  "Information regarding built-in macros.")

(defvar wesnoth-local-macro-data '()
  "All macro definitions available in the current project.")

(defvar wesnoth-tag-hash-table (make-hash-table :test 'equal
						:size 350)
  "Hash table of known WML tag data.")

(defun wesnoth-create-wml-hash-table (tag-data &optional force)
  "Handle generation of `wesnoth-tag-hash-table'.
TAG-DATA is the data to add to the hash-table.  If FORCE is
non-nil, update the hash-table regardless of whether it replacing
any existing data."
  (when (or (= (hash-table-count wesnoth-tag-hash-table) 0)
	    force)
    (clrhash wesnoth-tag-hash-table)
    (dolist (tag tag-data)
      (puthash (car tag) (cdr tag) wesnoth-tag-hash-table))))

(defun wesnoth-file-cfg-p (file)
  "Return non-nil if FILE has a '.cfg' extension."
  (and (not (file-directory-p file)) (string-match "\\.cfg$" file)))

(defun wesnoth-fetch-all-dirs (dir)
  "Retrieve a list of subdirectories to scan.
DIR is the directory to check."
  (let ((dirs-to-scan (wesnoth-files-in-dir dir)))
    (while dirs-to-scan
	(setq dirs-to-scan (append (wesnoth-files-in-dir (pop dirs-to-scan))
				   dirs-to-scan)))))

(defun wesnoth-files-in-dir (dir)
  "Add cfgs to `wesnoth-files-in-dir'.
Returns a list of sub-directories in DIR."
  (let ((cfgs (wesnoth-cfg-files-in-dir dir)))
    (when cfgs
      (setq wesnoth-found-cfgs (append cfgs wesnoth-found-cfgs))))
  (let ((dirs '()))
    (dolist (file (directory-files dir t))
      (when (and (not (string-match "^\\..*" (file-name-nondirectory file)))
                 (not (file-symlink-p file)))
	(cond ((file-directory-p file)
	       (add-to-list 'dirs file))
	      ((wesnoth-file-cfg-p file)
	       (add-to-list 'wesnoth-found-cfgs file)))))
    dirs))

(defun wesnoth-cfg-files-in-dir (dir)
  "Return all cfg files in DIR."
  (let ((result '()))
    (dolist (file (directory-files dir t))
      (and (wesnoth-file-cfg-p file)
	   (add-to-list 'result file)))
    result))

(defun wesnoth-determine-details (dir-or-file function)
  "Process .cfg files in DIR-OR-FILE using FUNCTION.
DIR-OR-FILE can be a file, a directory, or a list of files."
  (cond ((listp dir-or-file)
	 (dolist (file dir-or-file)
	   (wesnoth-handle-file function file)))
	((and (file-exists-p dir-or-file)
	      (not (file-directory-p dir-or-file)))
	 (wesnoth-handle-file function dir-or-file))
	(t
	 (wesnoth-fetch-all-dirs dir-or-file)
	 (while wesnoth-found-cfgs
           (message "Updating WML semantics: %s..."
                    (substring (car wesnoth-found-cfgs)
                               (length (expand-file-name
                                        wesnoth-root-directory))))
	   (unless (string-match "^\\..+" (file-name-nondirectory
					   (car wesnoth-found-cfgs)))
	     (wesnoth-handle-file function (car wesnoth-found-cfgs))
	     (setq wesnoth-found-cfgs (cdr wesnoth-found-cfgs)))))))

(defun wesnoth-handle-file (function file)
  "Perform FUNCTION on FILE."
  (with-temp-buffer
    (when (file-exists-p file)
      (insert-file-contents file)
      (funcall function))))

(defun wesnoth-extract-tag-information ()
  "Retrieve relevant tag and attribute information."
  (let ((unmatched-tag-list '()))
    (goto-char (point-min))
    (wesnoth-determine-macro-information)
    (while (search-forward-regexp
	    "^[\t ]*\\(\\[[+/]?\\(\\(\\w\\|_\\)+\\)\\]\\|\\(\\w\\|_\\)+=\\)"
	    (point-max) t)
      (beginning-of-line)
      (cond
       ((and (save-excursion
	       (search-backward-regexp
		"^[\t ]*\\(\\[[^/]]?\\|#define \\|#enddef \\)"
		(point-min) t))
	     (string-match "#define " (match-string 1))
	     (looking-at "^[\t ]*\\[\\+?\\(\\(\\w\\|_\\)+\\)\\]"))
	(wesnoth-append-tag-information (match-string-no-properties 1)
                                        nil nil)
	(setq unmatched-tag-list (cons (match-string-no-properties 1)
				       unmatched-tag-list)))
       ((looking-at "^[\t ]*\\[\\+?\\(\\(\\w\\|_\\)+\\)\\]")
	(wesnoth-append-tag-information (car unmatched-tag-list)
					(match-string-no-properties 1)
					nil)
	(wesnoth-append-tag-information (match-string-no-properties 1)
                                        nil nil)
	(setq unmatched-tag-list (cons (match-string-no-properties 1)
				       unmatched-tag-list)))
       ((looking-at "^[\t ]*\\(\\(\\w\\|_\\)+\\)=")
	(wesnoth-append-tag-information (car unmatched-tag-list)
					nil (match-string-no-properties 1)))
       ((looking-at "^[\t ]*\\[/\\(\\(\\w\\|_\\)+\\)\\]\\|")
	(when (string= (match-string-no-properties 1)
		       (car unmatched-tag-list))
	  (setq unmatched-tag-list (cdr unmatched-tag-list)))))
      (end-of-line))))

(defun wesnoth-append-tag-information (tag subtag attribute)
  "Add the information regarding TAG to the list.
SUBTAG and ATTRIBUTE are a children of TAG to be added."
  (let ((match (assoc tag wesnoth-tmp-tag-data)))
    (if (not match)
	(add-to-list 'wesnoth-tmp-tag-data
                     (list tag (and subtag (list subtag))
                           (and attribute (list attribute))))
      (if subtag
	  (let ((tmp (nth 1 match)))
	    (when (not (member subtag tmp))
	      (add-to-list 'tmp subtag)
	      (setq match (list tag tmp (car (last match))))))
	(when attribute (let ((tmp (nth 2 match)))
			  (when (not (member attribute tmp))
			    (add-to-list 'tmp attribute)
			    (setq match (list tag (nth 1 match) tmp))))))
      (setq wesnoth-tmp-tag-data
	    (remove (assoc tag wesnoth-tmp-tag-data)
		    wesnoth-tmp-tag-data))
      (add-to-list 'wesnoth-tmp-tag-data match))))

(defun wesnoth-determine-macro-information ()
  "Process the buffer, retrieving macro definition information."
  (save-excursion
    (goto-char (point-min))
    (while (search-forward-regexp
	    "#define \\(\\(?:\\w\\|_\\)+\\)\\(\\([\t ]+\\(\\w\\|_\\)+\\)*\\)"
	    (point-max) t)
      (beginning-of-line)
      (add-to-list 'wesnoth-tmp-macro-data
		   (list (match-string-no-properties 1)
			 (and (match-string 2)
			      (split-string
			       (match-string-no-properties 2)))))
      (end-of-line))
    wesnoth-tmp-macro-data))

(defun wesnoth-output-path ()
  "Determine the path to output wml information via `wesnoth-update'."
  (or wesnoth-update-output-directory
      (if (boundp 'user-emacs-directory)
	  (symbol-value 'user-emacs-directory)
	"~/.emacs.d/")))

(defun wesnoth-read-tmp-tag-data ()
  "Read `wesnoth-tmp-tag-data' and reset its value."
  (let ((results wesnoth-tmp-tag-data))
    (setq wesnoth-tmp-tag-data nil)
    results))

(defun wesnoth-tag-additions ()
  "Update WML tag information contained in `wesnoth-addition-file'."
  (setq wesnoth-tmp-tag-data nil)
  (wesnoth-determine-details wesnoth-addition-file
			     'wesnoth-extract-tag-information)
  (wesnoth-read-tmp-tag-data))

(defun wesnoth-macro-additions ()
  "Update WML macro information contained in `wesnoth-addition-file'."
  (setq wesnoth-tmp-macro-data nil)
  (wesnoth-determine-details
   wesnoth-addition-file
   (lambda ()
     (wesnoth-determine-macro-information)))
  (let ((results wesnoth-tmp-macro-data))
    (setq wesnoth-tmp-macro-data nil)
    results))

(defun wesnoth-update ()
  "Update WML information.
Path to WML information included in wesnoth is set by
`wesnoth-root-directory.'."
  (interactive)
  (setq wesnoth-tag-data nil
	wesnoth-macro-data nil
	wesnoth-found-cfgs nil
	wesnoth-tmp-macro-data nil
	wesnoth-tmp-tag-data nil)
  (unless (and (stringp wesnoth-root-directory)
	       (file-exists-p wesnoth-root-directory))
    ;; Update failed; restore data.
    (load "wesnoth-wml-data")
    (error "%s: directory does not exist"
	   wesnoth-root-directory))
  (message "Updating WML semantics: Searching for .cfg's...")
  (wesnoth-determine-details wesnoth-root-directory
			     'wesnoth-extract-tag-information)
  (setq wesnoth-tag-data wesnoth-tmp-tag-data
	wesnoth-tmp-tag-data nil
	wesnoth-macro-data wesnoth-tmp-macro-data
	wesnoth-tmp-macro-data nil)
  (with-temp-buffer
    (insert "(setq wesnoth-tag-data '"
            (pp wesnoth-tag-data) ")\n\n"
            "(setq wesnoth-macro-data '"
            (pp wesnoth-macro-data) ")\n\n"
            "(provide 'wesnoth-wml-data)\n")
    (write-file (expand-file-name "wesnoth-wml-data.el"
				  (wesnoth-output-path)))
    (load "wesnoth-wml-data"))
  (message "Updating WML semantics: done"))

(defun wesnoth-merge-macro-data (&rest macro-data)
  "Merge WML macro information and return the result.
MACRO-DATA is the macro-data to merge."
  (let ((set-data '())
	(macro-base-data (car macro-data)))
    (while (setq macro-data (cdr macro-data))
      (setq set-data (car macro-data))
      (while set-data
	(setq macro-base-data
	      (append (list (car set-data))
		      (remove (assoc (car (car set-data)) macro-base-data)
			      macro-base-data))
	      set-data (cdr set-data))))
    macro-base-data))

(defun wesnoth-merge-tag-data (&rest tag-data)
  "Merge WML tag information and return the result.
TAG-DATA is the tag-data to merge."
  (setq wesnoth-tmp-tag-data (car tag-data))
  (let ((set-data '()))
    (while (setq tag-data (cdr tag-data))
      (setq set-data (car tag-data))
      (while set-data
	(let ((subtags (nth 1 (car set-data))))
	  (while subtags
	    (wesnoth-append-tag-information (caar set-data) (car subtags)
					    nil)
	    (setq subtags (cdr subtags))))
	(let ((attributes (nth 2 (car set-data))))
	  (while attributes
	    (wesnoth-append-tag-information (caar set-data) nil
					    (car attributes))
	    (setq attributes (cdr attributes))))
	(setq set-data (cdr set-data))))
    (wesnoth-read-tmp-tag-data)))

(defun wesnoth-update-project-information (&optional clear)
  "Update WML macro information for the current project.
If CLEAR is non-nil, reset `wesnoth-local-macro-data'."
  (interactive "P")
  (setq wesnoth-tmp-macro-data nil)
  (if clear
      (setq wesnoth-local-macro-data nil)
    (setq wesnoth-local-macro-data
	  (wesnoth-merge-macro-data wesnoth-local-macro-data
				    (wesnoth-determine-macro-information)))
    (setq wesnoth-tmp-macro-data nil)))

(defun wesnoth-refresh-wml-data ()
  "Return merged WML tag data and WML data from the addition file."
  (save-match-data
    (let ((result (wesnoth-merge-tag-data
		   wesnoth-tag-data (wesnoth-tag-additions))))
      (wesnoth-create-wml-hash-table result t)
      result)))

(provide 'wesnoth-update)

;;; wesnoth-update.el ends here
