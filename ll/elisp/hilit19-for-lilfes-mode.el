;;;; Copyright (c) 2000, MITSUISHI Yutaka
;;;; You may distribute this file under the terms of the Artistic License.
;;;;
;;;; Name:    hilit19-for-lilfes-mode.el
;;;; Author:  MITSUISHI Yutaka (mitsuisi@is.s.u-tokyo.ac.jp)
;;;; Date:    2000.08.30
;;;; Purpose: hilit19 setting for lilfes-mode
;;;;

(hilit-set-mode-patterns
 'lilfes-mode
 '(("/\\*" "\\*/" comment)
;;;   ("\"[^\\\"]*\\(\\\\\\(.\\|\n\\)[^\\\"]*\\)*\"" nil string)
;;;   ("\"\\(\\\\\\(.\\|\n\\)*\\)*\"" nil string)
;;;   ("\"\\(\\\\\\(.\\|\\\\\n\\)*\\)*\"" nil string) ; Modified
   ("\"\\([^\"]\\|\n\\)*\"" nil string) ; Modified
   ("%.*$" nil comment)
   ("!" nil label)
   ("\\(:-\\|<-\\)" nil defun)	;; Modified
;;;   ("[A-Z0-9_]+:" nil define)	;; Added
   ("[A-Za-z0-9_]+\\\\" nil define)	;; Added
   ("+ [A-Z0-9_,() ]*" nil define)	;; Added
   ("'[A-Z][A-Z0-9_]*'" nil define)	;; Added
   ("\\$[A-Za-z0-9_]+" nil crossref)	;; Added
;;;   ("\\([A-Z]\\|[A-Z][A-Z0-9_]+\\)" nil crossref)	;; Added
   ("\\b\\(is\\|mod\\)\\b" nil keyword)
   ("\\(&\\|->\\|-->\\|;\\|==\\|\\\\==\\|=<\\|>=\\|<\\|>\\|=\\|\\\\=\\|=:=\\|=\\\.\\\.\\|\\\\\\\+\\)" nil decl)
   ("\\(\\\[\\||\\|\\\]\\)" nil include)))
