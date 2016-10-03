;;;; Copyright (c) 1997-2000, MITSUISHI Yutaka
;;;; You may distribute this file under the terms of the Artistic License.
;;;;
;;;; Name:    lilfes-mode.el (version 0.30)
;;;; Author:  MITSUISHI Yutaka (mitsuisi@is.s.u-tokyo.ac.jp)
;;;; Date:    2000.08.30 (since 1997.05.27)
;;;; Purpose: lilfes-mode
;;;;
;;;; (based on prolog.el and lisp-mode.el)
;;;;
;;;; History:
;;;;    2000.10.25 version 0.31 (%/* と */ で囲まれたコメントの
;;;;                             インデントはしないようにした.
;;;;                             % がある時の括弧の認識がおかし
;;;;                             かったのを直した)
;;;;    2000.08.30 version 0.30 (lisp-mode に入っていたのを
;;;;                             入らないようにした.
;;;;                             modify-syntax-entry をいくつか
;;;;                             まともにした)
;;;;    2000.08.30 version 0.21 (コメントの最後のピリオドを
;;;;                             定義の終わりのピリオドと
;;;;                             見做さないようにした)
;;;;    2000.08.28 version 0.20 (公開した)
;;;;	1997.12.13 version 0.18 (文末の認識処理の変更)
;;;;	1997.09.29 version 0.17 (; と \ を空白文字として扱う)
;;;;	1997.08.20 version 0.16 ([] を括弧として扱う)
;;;;	1997.08.15 version 0.15 (| を空白文字として扱う)
;;;;	1997.08.14 version 0.14 (途中にコメントがあるとうまく
;;;;				 いかなかったのを直した.
;;;;				 が, むちゃくちゃ汚い)
;;;;	1997.06.03 version 0.13 (途中にコメントがあるとうまく
;;;;				 いかなかったのを直した)
;;;;	1997.06.01 version 0.12 (?- の扱い)
;;;;	1997.05.29 version 0.11 (major-mode をちゃんと設定) 
;;;;	1997.05.27 version 0.10
;;;;   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(require 'prolog "prolog")

(defvar lilfes-default-indent 4)

(defun lilfes-mode ()
  (interactive)
  (prolog-mode)
  (modify-syntax-entry ?  " ")
  (modify-syntax-entry ?_ "_")
  (modify-syntax-entry ?! "w")
  (modify-syntax-entry ?\\ " ")
  (setq major-mode 'lilfes-mode)
  (setq mode-name "LiLFeS")
  )

(defun prolog-indent-line (&optional whole-exp)
  "Indent current line as Prolog code.
With argument, indent any additional lines of the same clause
rigidly along with this one (not yet)."
  (interactive "p")
  (let ((indent (prolog-indent-level))
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

(defun prolog-indent-level ()
  "Compute prolog indentation level."
;  (print (point))
  (save-excursion
    (beginning-of-line)
    (setq beg (point))
    (skip-chars-forward " \t")
    (cond
    ((looking-at "%%%") 0)		;Large comment starts
     ((looking-at "%/*") 0)		;Block comment starts
     ((let* ((comment-beg (save-excursion (search-backward "%/*" nil t 1)))
	     (comment-end (save-excursion (search-backward "*/" nil t 1))))
	(and comment-beg
	     (or (not comment-end)
		 (and comment-end
		      (> comment-beg comment-end)))))
	nil)
     ((looking-at "%[^%]") comment-column) ;Small comment starts
     ((bobp) 0)				;Beginning of buffer
     (t
      (beginning-of-line)
;;;      (indent-to (calculate-lilfes-indent))
      (calculate-lilfes-indent)))))

(defun lilfes-beginning-of-defun ()
  "Move backward to the beginning of a definition of a predicate.
This function simply search a period backward.
If there is :- or <- or ?-, go forward after that.
Return the current column."
  (interactive "P")
  (let ((indent-point (point))
	(check t))
    (while check ; Added for a period at the end of a comment
      (re-search-backward "\\.[$ \t\n]+" nil 'move 1) ; search period
;;;   (re-search-backward "\\." nil 'move 1) ; search period
;;;	 (goto-char (1- (match-end 0)))
      (let ((period-point (point)))
	(beginning-of-line)
	(setq check (re-search-forward "\%" period-point 'move 1))))
    (if (not (bobp))
	(progn
	  (forward-line 1)
	  (beginning-of-line)))
    (while (or (looking-at "\%") (looking-at "\n")) ; skip comments
      (forward-line 1)
      (beginning-of-line))
;;;    (print (point))
    (let ((new-point 
	   (if (<= (point) indent-point)
	       (search-forward-regexp "\\(:\\|<\\|?\\)-" indent-point t)
	     nil)))
      (if new-point
	  (progn (goto-char new-point)
;;;		 (current-column)
;;;		 (print 'here)
		 lilfes-default-indent)
	(current-column)))))

(defun lilfes-parse-partial-sexp (from to)
;  (print 'a) (print from) (print to)
;;;  (setq l-hook lisp-mode-hook)
;;;  (setq p-hook prolog-mode-hook)
;;;  (setq lisp-mode-hook nil)
;;;  (setq prolog-mode-hook p-hook)	;;; !!!!!
;;;  (lisp-mode)
  (modify-syntax-entry ?% " ")
;;;  (modify-syntax-entry ?[ "(]")
;;;  (modify-syntax-entry ?] ")[")
;;;  (modify-syntax-entry ?| " ")
;;;  (modify-syntax-entry ?\; " ")
;;;  (modify-syntax-entry ?\\ " ")
  (let ((state (parse-partial-sexp from to 0 nil nil t))
	(first-state state)
	last-sexp
	containing-sexp)
;    (print 'b)
;    (print state)
    (while (and ;; (elt state 4)
	    (< (point) to))
      (if (elt state 2)
	  (setq last-sexp (elt state 2)))
      (if (elt state 1)
	  (setq containing-sexp (elt state 1)))
;;;      (print (point))
;;;      (print 'to)
;;;      (print to)
      (forward-line 1)
      (beginning-of-line)
;;;      (print state)
;;;      (print 'aaa)
;;;      (print (point))
      (if (< (point) to)
	  (progn
	    (setcar (cdr (cdr (cdr (cdr state)))) nil)
	    (setq state (parse-partial-sexp (point) to 0 nil state t)))))
;;;    (if (null (elt state 2)) (setcar (cdr (cdr state)) last-sexp))
    (if (null (elt state 1)) (setcar (cdr state) containing-sexp))
    (modify-syntax-entry ?% "<")
;;;    (lilfes-mode)
;;;    (setq lisp-mode-hook l-hook)
    state))

(defun calculate-lilfes-indent (&optional parse-start)
  "Return appropriate indentation for current line as Lisp code.
In usual case returns an integer: the column to indent to.
Can instead return a list, whose car is the column to indent to.
This means that following lines at the same level of indentation
should not necessarily be indented the same way.
The second element of the list is the buffer position
of the start of the containing expression."
  (interactive "P")
  (save-excursion
    (beginning-of-line)
    (let ((indent-point (point))
          state paren-depth
          ;; setting this to a number inhibits calling hook
          (desired-indent nil)
          (retry t)
	  (extra-indent 0)	;; Outside the outermost bracket (:-) or (<-)
          last-sexp containing-sexp)
      (if parse-start
          (goto-char parse-start)
          (setq extra-indent (lilfes-beginning-of-defun)))
      ;; Find outermost containing sexp
;;;      (print (point))
;;;      (print indent-point)
      (while (< (point) indent-point)
        (setq state ;;;(parse-partial-sexp (point) indent-point 0)
	      (lilfes-parse-partial-sexp (point) indent-point)))
;;;	      (parse-partial-sexp (point) indent-point 0 nil nil t)))	;; Jun 3	;; last argument t -> nil Aug 14
;;;      (print retry)
;;;      (print 'zzz)
;      (print state)
      ;; Find innermost containing sexp
      (while (and retry
		  state
                  (or (> (setq paren-depth (elt state 0)) 0)))
;;;		      (and (= (setq paren-depth (elt state 0)) 0)
;;;			   (not (elt state 1)))
        (setq retry nil)
        (setq last-sexp (elt state 2))
        (setq containing-sexp (elt state 1))
;;;	(print 'here)
        ;; Position following last unclosed open.
	(if containing-sexp
	    (goto-char (1+ containing-sexp)))
;	(print last-sexp)
;	(print containing-sexp)
;	(if (search-backward "->" containing-sexp 'move 1)
;	    (aaa)
;	    (forward-char 4))
;;;	(print (point))
        ;; Is there a complete sexp since then?
        (if (and last-sexp (> last-sexp (point)))
            ;; Yes, but is there a containing sexp after that?
            (let ((peek ;;; (parse-partial-sexp last-sexp indent-point 0)))
		   (lilfes-parse-partial-sexp last-sexp indent-point)))
              (if (setq retry (car (cdr peek))) (setq state peek)))))
      (if retry
	  (progn
;;;	    (print (point))
;;;	    (print (current-column))
	    (goto-char indent-point)
	    (beginning-of-line)
	    (indent-to extra-indent))
        ;; Innermost containing sexp found
	  (if containing-sexp
	      (goto-char (1+ containing-sexp)))
        (if t ;;; (not last-sexp)	;; Modified by MITSUISHI Yutaka
	    ;; indent-point immediately follows open paren.
	    ;; Don't call hook.
            (setq desired-indent (current-column))
	  ;; Find the start of first element of containing sexp.
	  (parse-partial-sexp (point) last-sexp 0 t)
	  (cond ((looking-at "\\s(")
		 ;; First element of containing sexp is a list.
		 ;; Indent under that list.
		 )
		((> (save-excursion (forward-line 1) (point))
		    last-sexp)
		 ;; This is the first line to start within the containing sexp.
		 ;; It's almost certainly a function call.
		 (if (= (point) last-sexp)
		     ;; Containing sexp has nothing before this line
		     ;; except the first element.  Indent under that element.
		     nil
		   ;; Skip the first element, find start of second (the first
		   ;; argument of the function call) and indent under.
		   (progn (forward-sexp 1)
			  (parse-partial-sexp (point) last-sexp 0 t)))
		 (backward-prefix-chars))
		(t
		 ;; Indent beneath first sexp on same line as last-sexp.
		 ;; Again, it's almost certainly a function call.
		 (goto-char last-sexp)
		 (beginning-of-line)
		 (parse-partial-sexp (point) last-sexp 0 t)
		 (backward-prefix-chars)))))
      ;; Point is at the point to indent under unless we are inside a string.
      ;; Call indentation hook except when overridden by lisp-indent-offset
      ;; or if the desired indentation has already been computed.
      (let ((normal-indent (current-column)))
        (cond ((elt state 3)
               ;; Inside a string, don't change indentation.
               (goto-char indent-point)
               (skip-chars-forward " \t")
               (current-column))
              ((and (integerp lisp-indent-offset) containing-sexp)
               ;; Indent by constant offset
               (goto-char containing-sexp)
               (+ (current-column) lisp-indent-offset))
              (desired-indent)
              ((and (boundp 'lisp-indent-function)
                    lisp-indent-function
                    (not retry))
               (or (funcall lisp-indent-function indent-point state)
                   normal-indent))
              (t
               normal-indent))))))
