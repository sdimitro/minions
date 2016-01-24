(defvar *db* nil)

(defun make-course (title desc start-time end-time)
  (list :title title :desc desc :start-time start-time :end-time end-time))

(defun add-course (course) (push course *db*))

(defun dump-db ()
  (dolist (course *db*)
    (format t "~{~a:~10t~a~%~}~%" course)))


;; dump-db could have also been like this:
;; (defun dump-db ()
;;   (format t "~{~{~a:~10t~a~%~}~%~}" *db*)
;;
(defun prompt-read (prompt)
  (format *query-io* "~a: " prompt)
  (force-output *query-io*)
    (read-line *query-io*))

(defun prompt-for-course ()
  (make-course
   (prompt-read "Title")
   (prompt-read "Description")
   (or (parse-integer (prompt-read "Start Time") :junk-allowed t) 0)
   (or (parse-integer (prompt-read "End Time") :junk-allowed t) 0)))

(defun add-courses ()
  (loop (add-course (prompt-for-course))
     (if (not (y-or-n-p "Another? [y/n]: ")) (return))))

(defun save-db (&optional (filename "course.db"))
  (with-open-file (out filename
		       :direction :output
		       :if-exists :supersede)
    (with-standard-io-syntax
            (print *db* out))))

(defun load-db (&optional (filename "course.db"))
  (with-open-file (in filename)
    (with-standard-io-syntax
            (setf *db* (read in)))))

(defun make-comparison-expr (field value)
  `(equal (getf course ,field) ,value))

(defun make-comparisons-list (fields)
  (loop while fields
     collecting (make-comparison-expr (pop fields) (pop fields))))

(defmacro where (&rest clauses)
  `#'(lambda (course) (and ,@(make-comparisons-list clauses))))

(defun select (selector-fn)
  (remove-if-not selector-fn *db*))

(defun make-assign-expr (field value)
  `(setf (getf course ,field) ,value))

(defun make-assign-list (fields)
  (loop while fields
     collecting (make-assign-expr (pop fields) (pop fields))))

(defmacro sql-set (&rest clauses)
  `#'(lambda (course) (progn ,@(make-assign-list clauses))))

(defun update (selector-fn mutator-fn)
  (setf *db*
	(mapcar
	 #'(lambda (row)
	     (when (funcall selector-fn row)
	       (funcall mutator-fn row))
	     row) *db*)))

(defun delete-rows (selector-fn)
    (setf *db* (remove-if selector-fn *db*)))
