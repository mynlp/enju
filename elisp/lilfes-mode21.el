;;;; Copyright (c) 2001-2004, MIYAO Yusuke
;;;; You may distribute this file under the terms of the Artistic License.
;;;;
;;;; Name:    lilfes-mode21.el (version 0.2)
;;;; Author:  MIYAO Yusuke (yusuke@is.s.u-tokyo.ac.jp)
;;;; Date:    2004.07.02 (since 2001.12.27)
;;;; Purpose: A major-mode for editing LiLFeS code for Emacs 21
;;;; $Id: lilfes-mode21.el,v 1.2 2004-09-25 03:21:49 yusuke Exp $
;;;;
;;;; Usage:
;;;;    %%%  Block comments
;;;;    %%   Usual comments
;;;;    %    small commments
;;;;
;;;; History:
;;;;	2001.12.27 version 0.1 (Start)
;;;;    2004.07.02 version 0.2 (support for liblilfes)
;;;;   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar lilfes-mode-syntax-table nil)
(defvar lilfes-mode-abbrev-table nil)
(defvar lilfes-mode-map nil)

(defgroup lilfes nil
  "Major mode for editing LiLFeS"
  :group 'languages)

(defcustom lilfes-indent-width 4
  "Level of indentation in LiLFeS"
  :type 'integer
  :group 'prolog)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(if lilfes-mode-syntax-table
    nil
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?  " " table)
    (modify-syntax-entry ?_ "_" table)
    (modify-syntax-entry ?: "_" table)
    (modify-syntax-entry ?/ "." table)
    (modify-syntax-entry ?* "." table)
;    (modify-syntax-entry ?/ ". 1b4b" table)
;    (modify-syntax-entry ?* ". 2b3b" table)
    (modify-syntax-entry ?+ "." table)
    (modify-syntax-entry ?- "." table)
    (modify-syntax-entry ?# "." table)
    (modify-syntax-entry ?^ "." table)
    (modify-syntax-entry ?= "." table)
    (modify-syntax-entry ?| "." table)
    (modify-syntax-entry ?< "." table)
    (modify-syntax-entry ?> "." table)
    (modify-syntax-entry ?! "." table)
    (modify-syntax-entry ?\\ "." table)
    (modify-syntax-entry ?. "." table)
    (modify-syntax-entry ?\; "." table)
    (modify-syntax-entry ?% "<\n" table)
    (modify-syntax-entry ?\n ">%" table)
    (modify-syntax-entry ?\' "$'" table)
    (modify-syntax-entry ?\[ "(]" table)
    (modify-syntax-entry ?\] ")[" table)
    (modify-syntax-entry ?\( "()" table)
    (modify-syntax-entry ?\) ")(" table)
    (modify-syntax-entry ?{ "(}" table)
    (modify-syntax-entry ?} "){" table)
    (setq lilfes-mode-syntax-table table)))

(define-abbrev-table 'lilfes-mode-abbrev-table ())

(if lilfes-mode-map
    nil
  (setq lilfes-mode-map (make-sparse-keymap))
  (define-key lilfes-mode-map "\t" 'lilfes-indent-line))

(defun lilfes-mode-variables ()
  (set-syntax-table lilfes-mode-syntax-table)
  (setq local-abbrev-table lilfes-mode-abbrev-table)
  (use-local-map lilfes-mode-map)
  (make-local-variable 'comment-start)
  (setq comment-start "%% ")
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "%+ *")
  (make-local-variable 'comment-column)
  (setq comment-column 48)
  (make-local-variable 'comment-indent-function)
  (setq comment-indent-function 'lilfes-comment-indent)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'lilfes-indent-line)
  (make-local-variable 'paragraph-start)
  (setq paragraph-start "%%\\|$")
  (make-local-variable 'paragraph-separate)
  (setq paragraph-separate paragraph-start)
  (make-local-variable 'paragraph-ignore-fill-prefix)
  (setq paragraph-ignore-fill-prefix t)
  (make-local-variable 'imenu-generic-expression)
  (setq imenu-generic-expression "^[a-z][a-zA-Z0-9_]+")
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults
	'((lilfes-font-lock-keywords
	   lilfes-font-lock-keywords-1
	   lilfes-font-lock-keywords-2
	   lilfes-font-lock-keywords-3)
	  t nil ((?\_ . "w"))))
  (make-local-variable 'font-lock-multiline)
  (setq font-lock-multiline t))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;###autoload
(defun lilfes-mode ()
  "Major mode for editing LiLFeS code."
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'lilfes-mode)
  (setq mode-name "LiLFeS")
  (lilfes-mode-variables)
  (run-hooks 'lilfes-mode-hook))

(defun lilfes-indent-line (&optional whole-exp)
  "Indent current line as LiLFeS code."
  (interactive "p")
  (let ((indent (lilfes-indent-level))
	(pos (- (point-max) (point))) beg)
    (if indent
	(progn
	  (beginning-of-line)
	  (setq beg (point))
	  (skip-chars-forward " \t")
	  (if (zerop (- indent (current-column)))
	      nil
	      (delete-region beg (point))
	      (indent-to indent))
	  (if (> (- (point-max) pos) (point))
	      (goto-char (- (point-max) pos))))
	t)))

(defun lilfes-indent-level ()
  "Compute LiLFeS indentation level."
  (save-excursion
    (beginning-of-line)
    (setq beg (point))
    (skip-chars-forward " \t")
    (cond
     ((looking-at "%%%") 0)		;Large comment starts
;;     ((looking-at "%%[^%]") (current-indentation))
     ((looking-at "%/\\*") 0)		;Block comment starts
     ((let ((comment-beg (save-excursion (search-backward "%/*" nil t 1)))
	    (comment-end (save-excursion (search-backward "*/" nil t 1))))
	(and comment-beg
	     (or (not comment-end)
		 (and comment-end
		      (> comment-beg comment-end)))))
      nil)
     ((looking-at "%[^%]") comment-column) ;Small comment starts
     ((bobp) 0)			;Beginning of buffer
     (t (lilfes-prev-indent-level)))))

(defun lilfes-prev-indent-level ()
  "Search a previous indent level."
  (let ((indent nil) (bol nil) (num-paren 0) end-char)
    (while (not indent)
      ;; (print (list (point) bol))
      (if bol
	  ;; searching a parenthesis in a line
	  (if (re-search-backward ")\\|(\\|\\]\\|\\[" bol t)
	      ;; paren found
	      (let ((paren (match-string 0)))
		(when (string-match "^\\([^\"]*\"[^\"]*\"\\)*[^\"]*$"
				    (buffer-substring-no-properties bol (point)))
		  (cond
		   ((or (string= paren ")") (string= paren "]"))
		    (setq num-paren (1+ num-paren)))
		   ((or (string= paren "(") (string= paren "["))
		    (if (> num-paren 0)
			(setq num-paren (1- num-paren))
		      (setq indent (1+ (current-column))))))))
	    ;; paren not found
	    (goto-char bol)
	    (setq bol nil)  ; search for a new line
	    ;; (print (list "current" (current-column) (point)))
	    (if (zerop num-paren)
		(setq indent (current-indentation))))
	;; search for a new line (set bol)
	(forward-comment (- 0 (point)))
	(when (bobp) (setq indent 0))
	(setq bol (save-excursion (beginning-of-line) (point)))
	(setq end-char (preceding-char))
	;; (print (list "new eol" (point)))
	(cond
	 ((char-equal end-char ?.) (setq indent 0))
	 ((char-equal end-char ?-) (setq indent lilfes-indent-width)))))
    ;; (print (list "indent" indent))
    indent))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Font lock support

(defvar lilfes-font-lock-builtin-face
  (if (boundp 'xemacs-logo)
      font-lock-preprocessor-face
    font-lock-builtin-face))
(defvar lilfes-font-lock-constant-face
  (if (boundp 'xemacs-logo)
      font-lock-reference-face
    font-lock-constant-face))

(defvar lilfes-function-keywords
  '("fail" "true" "is" "mod" "pred" "bot" "list" "nil" "cons"
    "integer" "string" "float"))
(defvar lilfes-builtin-operators
  '("+" "-" "*" "/" "//" "#" "^" "&" "->" "-->" ";" "==" "\\=" "=<" ">=" "<" ">" "=" "=:=" "\\+"))
(defvar lilfes-builtin-predicates
  '(
    ;; exec
    "dynamic" "is_dynamic" "multifile" "is_multifile" "disp_result"
    "eval" "call" "findall" "dfindall" "assert" "asserta" "halt"
    ;; fsmanip
    "copy" "normalize" "canonical_copy" "unifiable" "isshared" "isnotshared"
    "recopy" "b_equiv" "identical" "not_identical" 
    "is_integer" "is_string" "is_float" "have_child" "have_no_child"
    "compound" "simple" "follow" "follow_if_exists" "restriction" "overwrite" "generalize"
    "subnodelist" "subnodelist_count" "sharednodelist" "contains"
    "equivalent" "not_equivalent" "subsume"
    "type_equal" "type_subsume" "type_toptype" "type_copy"
    "type_extendable" "type_unify" "type_common" "type_nfeature"
    "type_featurelist" "type_havefeature"
    "type_subtypes" "type_nsubtypes" "type_supertypes" "type_nsupertypes" 
    "type_unifiables" "type_unifiables" "type_directsubtypes" "type_directsupertypes"
    "fs_compare"
    ;; display
    "print" "printtostr" "printAVM" "printAVMtostr"
    ;; arith
    "floattoint" "floor" "log" "exp"
    ;; database
    "db_open_env" "db_open" "db_sync" "db_close"
    "db_find" "db_insert" "db_delete" "db_clear" "db_findall"
    "db_first" "db_last" "db_next" "db_prev" "db_save" "db_load"
    "declare_array" "delete_array" "get_array" "set_array" "unset_array"
    "clear_array" "save_array" "load_array"
    ;; regex
    "regex_match" "regex_subst"
    ;; module
    ;; prof
    "start_prof" "stop_prof" "total_prof"
    ;; stream
    "open_null_stream" "open_socket_stream" "open_pty_stream"
    "open_process_stream" "open_file_stream" "open_server_stream"
    "accept_connection" "close_connection"
    "write_string" "writeln_string" "write_string_list" "writeln_string_list"
    "read_string" "readln_string" "write_stream" "read_stream"
    "close" "flush_output" "eof_stream" "stdin_stream" "stdout_stream" "stderr_stream"
    ;; string
    "strtolist" "strtodlist" "listtostr" "strcat" "strlen"
    "strtotype" "typetostr" "feattostr"
    "inttostr" "strtoint" "floattostr" "strtofloat"
    ;; system
    "getenv" "environ" "getdate" "now" "system"
    ;; trace
    "trace" "notrace"
    ;; xml
    "tfstoxml"
    ;; misc
    "lilfes_version"
    ))

(defconst lilfes-font-lock-keywords-1
  (eval-when-compile
    '(
      ("%/\\*\\([^*]\\|\\*[^/]\\)*\\*/" . font-lock-comment-face)        ;; block comments
      ("%.*$" . font-lock-comment-face)                                  ;; comments
      ("^:-[ \t]*\\(module\\)(\\(\"[^\"]*\"\\))"
       (1 font-lock-warning-face) (2 font-lock-string-face nil t))       ;; module
      ("^:-[ \t]*\\(module_interface\\)" 1 font-lock-warning-face)       ;; interface
      ("^:-[ \t]*\\(module_implementation\\)" 1 font-lock-warning-face)  ;; implementation
      ("^:-[ \t]*\\(module_extendable\\)" 1 font-lock-warning-face)  ;; implementation
      ("^:-[ \t]*\\(ensure_loaded\\)(\\(\"[^\"]*\"\\))"
       (1 font-lock-keyword-face) (2 font-lock-string-face nil t))       ;; ensure_loaded
      ("\"\\([^\"]\\)*\\(\"\\|\n\\)" . font-lock-string-face)            ;; strings
      ))
  "Simple highlighting for LiLFeS mode.")

(defconst lilfes-font-lock-keywords-2
  (eval-when-compile
    (append
     lilfes-font-lock-keywords-1
     (list
      '("\\(!\\)" (1 (cons lilfes-font-lock-builtin-face '(underline))))                  ;; cut
      '("\\(['A-Za-z0-9_]+\\\\\\)" (1 (cons lilfes-font-lock-constant-face '(bold))))     ;; features
      '("\\(^[A-Za-z0-9_]+\\)[ \t]*<-.*\\<pred\\>" 1 font-lock-function-name-face) ;; predicates
      '("\\([a-z0-9_]+\\)[ \t]*<-" 1 font-lock-type-face)                          ;; types
      '("\\('[^']+'\\)" 1 font-lock-type-face)                                     ;; types
      '("\\$[A-Za-z0-9_]+" . font-lock-variable-name-face)                         ;; variables
      '("\\<[A-Z][A-Za-z0-9_]*\\>" . font-lock-variable-name-face)                 ;; variables
      (cons
       (concat "\\<\\(" (regexp-opt lilfes-function-keywords) "\\)\\>")
       font-lock-keyword-face)                                                     ;; keywords
      '("\\(<-\\|:-\\)" (1 (cons lilfes-font-lock-builtin-face '(bold))))                 ;; clause def.
      '("\\(\\[\\||\\|\\]\\)" (1 (cons lilfes-font-lock-builtin-face '(bold))))           ;; list
      (cons (regexp-opt lilfes-builtin-operators) lilfes-font-lock-builtin-face)          ;; operators
      (cons "\\<[0-9\.]+\\>" lilfes-font-lock-constant-face)                                ;; numbers
      )))
  "Moderate expressions to highlight in LiLFeS mode.")

(defconst lilfes-font-lock-keywords-3
  (eval-when-compile
    (append
     lilfes-font-lock-keywords-2
     (list
      (cons
       (concat "\\<\\(" (regexp-opt lilfes-builtin-predicates) "\\)\\>")
       font-lock-keyword-face)                                                   ;; builtin
      '("\\(^[A-Za-z0-9_]+\\)\\((\\|[ \t]*:-\\|[ \t]*\\.\\)"                     ;; predicates
	1 font-lock-function-name-face)
      )))
  "Maximum expressions to highlight in LiLFeS mode.")

(defvar lilfes-font-lock-keywords lilfes-font-lock-keywords-2
  "Default expressions to highlight in LiLFeS mode.")

