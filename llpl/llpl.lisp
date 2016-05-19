;; UTILITY FUNCTIONS

(defun round-to (number precision &optional (what #'round))
  (let ((div (expt 10 precision)))
    (/ (funcall what (* number div)) div)))

(defun rounded-fsub (&rest numbers)
  (float (round-to (apply #'- numbers) 3)))

(defun merge-assoc-with (fn alist)
  (let ((res '()))
    (dolist (current alist res)
      (let ((existing (assoc (car current) res :test #'equal)))
	(if existing
	    (setf (cadr (assoc (car current) res :test #'equal))
		    (funcall fn (cadr existing) (cadr current)))
	    (push current res))))))

;; LLPL CODE

(defstruct param name values)

;; LLPL: Predicates

(defun param-cond-p (p)
  (find #\| (param-name p)))

(defun param-joint-p (p)
  (find #\, (param-name p)))

(defun param-pjoint-p (p)
  (let* ((name (param-name p))
	 (in-question (subseq name 0 (position #\| name)))
	 (given (subseq name (position #\| name))))
    (and (find #\, in-question) (not (find #\, given)))))

(defun param-cjoint-p (p)
  (let* ((name (param-name p))
	 (given (subseq name (position #\| name))))
    (find #\, given)))

(defun param-sjoint-p (p)
  (and (param-joint-p p) (not (param-cond-p p))))

(defun param-norm-p (p)
  (= 1.0 (param-sum-vals p)))

;; LLPL: Parameter - Internal Info

(defun param-sum-vals (par)
  (if (param-p par)
      (let ((prob-vals
	     (mapcar #'(lambda (x) (cadr x))
			       (param-values par))))
	(reduce #'+ prob-vals))))

;; LLPL: Parameter - Internal State Operations

(defun param-push-val (val par)
  (if (and (listp val) (param-p par))
      (push val (param-values par))
      NIL))

(defun param-get-prob (val par)
  (if (param-p par)
      (cadr (assoc val (param-values par) :test #'equal))))

(defun param-set-prob (val par n)
  (if (param-p par)
      (setf (cadr (assoc val (param-values par) :test #'equal)) n)))

(defun param-get-prob-not (val par)
  (let ((result (param-get-prob val par)))
    (if result (rounded-fsub 1.0 result))))

(defun param-generate-last-val (p)
  (let ((remaining (rounded-fsub 1.0 (param-sum-vals p))))
    (param-push-val `(GENERATED-CASE ,remaining) p)))

(defun param-get-cases (p)
  (length (param-values p)))

;;; JOINT DISTRIBUTION STUFF

(defvar *joint-example*
  (make-param :name '(A B C)
	      :values '(((T T T) 0.1)
		        ((T T F) 0.1)
		        ((T F T) 0.1)
		        ((T F F) 0.1)
		        ((F T T) 0.1)
		        ((F T F) 0.1)
		        ((F F T) 0.1)
		        ((F F F) 0.3))))

(defun joint-get-indeces (rvar-names joint)
  (mapcar #'(lambda (x) (position x (param-name joint)))
	  rvar-names))

(defun joint-get-values-by-index (index joint)
  (remove-duplicates
   (mapcar #'(lambda (x) (nth index (car x)))
	   (param-values joint))))

(defun joint-sum-by-at (symbol index joint)
  (apply #'+
	 (mapcar #'(lambda (x) (cadr x))
		 (remove-if-not
		  #'(lambda (x) (equal symbol (nth index (car x))))
		  (param-values joint)))))

;; LLPL: Parameter - Generational Operations

(defun generate-subjoint-with-dups (index-list joint)
  (mapcar #'(lambda (x) (list (mapcar #'(lambda (y) (nth y (car x))) index-list) (cadr x))) (param-values joint)))
