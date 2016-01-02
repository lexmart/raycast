; Thanks to Casey of (handmadehero.org) for helping me on this .emacs

; no toolbar 
(tool-bar-mode 0)


;;; TODO: Remap these to control!!!
; kill to beginning of line
(defun backward-kill-line (arg)
  "Kill ARG lines backward."
  (interactive "p")
  (kill-line (- 1 arg)))
(define-key global-map "\ej" 'backward-kill-line)

; kill to end of line
(define-key global-map "\ek" 'kill-line)

; split window on start
(setq ediff-split-window-function 'split-window-horizontally)
(split-window-horizontally)

; swithcing between windows
(define-key global-map "\ew" 'other-window)

; searching for files
(define-key global-map "\ef" 'find-file)

; copy and paste
(cua-mode t)

; remap undo

(define-key global-map "\e/" 'undo)

; arrows

(defun previous-blank-line ()
  "Moves to the previous line containing nothing but whitespace."
  (interactive)
  (search-backward-regexp "^[ \t]*\n")
)

(defun next-blank-line ()
  "Moves to the next line containing nothing but whitespace."
  (interactive)
  (forward-line)
  (search-forward-regexp "^[ \t]*\n")
  (forward-line -1)
)
(define-key global-map [M-up] 'previous-blank-line)
(define-key global-map [M-down] 'next-blank-line)
(define-key global-map [M-right] 'forward-word)
(define-key global-map [M-left] 'backward-word)

; no indent braces
(setq c-default-style "linux")

; no scroll bar
(scroll-bar-mode -1)

; backup files in folder
(setq backup-directory-alist `(("." . "~/.saves")))

; no tabs (spaces instead)
(setq-default indent-tabs-mode nil)

; tab size
(setq-default c-basic-offset 4)

; delete blocks with backspace
(delete-selection-mode 1)

; saving
(defun save-buffer-always ()
  "Save the buffer even if it is not modified."
  (interactive)
  (set-buffer-modified-p t)
  (save-buffer))
(define-key global-map "\es" 'save-buffer-always)

; find files autocomplete
(setq ido-enable-flex-matching t)
(setq ido-everywhere t)
(ido-mode 1)

(setq aquamacs (featurep 'aquamacs))
(setq linux (featurep 'x))
(setq win32 (not (or aquamacs linux)))

; max on startup
(defun maximize-frame ()
  "Maximize the current frame"
  (interactive)
  (when aquamacs (aquamacs-toggle-full-frame))
  (when win32 (w32-send-sys-command 61488)))

; color stuff
(add-to-list 'default-frame-alist '(font . "Liberation Mono-11"))
(set-face-attribute 'default t :font "Liberation Mono-12.5")
(set-face-attribute 'font-lock-builtin-face nil :foreground "#DAB98F")
(set-face-attribute 'font-lock-comment-face nil :foreground "gray50")
(set-face-attribute 'font-lock-constant-face nil :foreground "olive drab")
(set-face-attribute 'font-lock-doc-face nil :foreground "gray50")
(set-face-attribute 'font-lock-function-name-face nil :foreground "burlywood3")
(set-face-attribute 'font-lock-keyword-face nil :foreground "DarkGoldenrod3")
(set-face-attribute 'font-lock-string-face nil :foreground "olive drab")
(set-face-attribute 'font-lock-type-face nil :foreground "burlywood3")
(set-face-attribute 'font-lock-variable-name-face nil :foreground "burlywood3")

(defun post-load-stuff ()
  (interactive)
  (menu-bar-mode -1)
  (maximize-frame)
  (set-foreground-color "burlywood3")
  (set-background-color "#161616")
  (set-cursor-color "#40FF40"))

(add-hook 'window-setup-hook 'post-load-stuff t)

; Compilation

(setq makescript "build.bat")
(define-key global-map "\en" 'next-error)
(define-key global-map "\ep" 'previous-error)
(defun make-without-asking ()
  "Make the current build."
  (interactive)
  (compile makescript)
  (other-window 1))
(define-key global-map "\em" 'make-without-asking)

; Toggle betwee .h .cpp

(defun find-corresponding-file ()
  "Find the file that corresponds to this one."
  (interactive)
  (setq CorrespondingFileName nil)
  (setq BaseFileName (file-name-sans-extension buffer-file-name))
  (if (string-match "\\.c" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".h")))
  (if (string-match "\\.h" buffer-file-name)
      (if (file-exists-p (concat BaseFileName ".c")) (setq CorrespondingFileName (concat BaseFileName ".c"))
        (setq CorrespondingFileName (concat BaseFileName ".cpp"))))
  (if (string-match "\\.hin" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".cin")))
  (if (string-match "\\.cin" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".hin")))
  (if (string-match "\\.cpp" buffer-file-name)
      (setq CorrespondingFileName (concat BaseFileName ".h")))
  (if CorrespondingFileName (find-file CorrespondingFileName)
    (error "Unable to find a corresponding file")))
(define-key global-map "\ec" 'find-corresponding-file)

; jump to function by name
(define-key global-map "\eh" 'imenu)

; color TODO, NOTE
(setq fixme-modes '(c++-mode c-mode emacs-lisp-mode))
(make-face 'font-lock-fixme-face)
(make-face 'font-lock-study-face)
(make-face 'font-lock-important-face)
(make-face 'font-lock-note-face)
(mapc (lambda (mode)
        (font-lock-add-keywords
         mode
         '(("\\<\\(TODO\\)" 1 'font-lock-fixme-face t)
           ("\\<\\(STUDY\\)" 1 'font-lock-study-face t)
           ("\\<\\(IMPORTANT\\)" 1 'font-lock-important-face t)
           ("\\<\\(NOTE\\)" 1 'font-lock-note-face t))))
      fixme-modes)
(modify-face 'font-lock-fixme-face "Red" nil nil t nil t nil nil)
(modify-face 'font-lock-study-face "Yellow" nil nil t nil t nil nil)
(modify-face 'font-lock-important-face "Yellow" nil nil t nil t nil nil)
(modify-face 'font-lock-note-face "Dark Green" nil nil t nil t nil nil)

; indent switch statements
(c-set-offset 'case-label '+)

; query replace
(define-key global-map "\eq" 'query-replace)