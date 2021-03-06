(module machine racket

	(require "register.rkt")
	(require "stack.rkt")

  (provide make-new-machine start get-register
					 get-register-contents set-register-contents!
					 make-machine)

	(define (make-new-machine)
		(let ((pc (make-register 'pc))
					(flag (make-register 'flag))
					(stack (make-stack))
					(the-instruction-sequence '()))
			(let ((the-ops
						 (list (list 'initialize-stack
												 (lambda () (stack 'initialize)))))
						(register-table
						 (list (list 'pc pc) (list 'flag flag))))

				;; Adds register to the register table
				(define (allocate-register name)
					(if (assoc name register-table)
							(error "Multiply defined register: " name)
							(set! register-table
										(cons (list name (make-register name))
													register-table)))
					'register-allocated)

				;; Return value of register
				(define (lookup-register name)
					(let ((val (assoc name register-table)))
						(if val
								(cadr val)
								(error "Unknown register:" name))))

				;; Starts executing instructions pointed by the pc
				(define (execute)
					(let ((insts (get-contents pc)))
						(if (null? insts)
								'done
								(begin
									((instruction-execution-proc (car insts)))
									(execute)))))

				;; TODO: define execute-step
				;; TODO: define dump-state

				(define (dispatch message)
					(cond ((eq? message 'start)
								 (set-contents! pc the-instruction-sequence)
								 (execute))
								((eq? message 'install-instruction-sequence)
								 (lambda (seq) (set! the-instruction-sequence seq)))
								((eq? message 'allocate-register) allocate-register)
								((eq? message 'get-register) lookup-register)
								((eq? message 'install-operations)
								 (lambda (ops) (set! the-ops (append the-ops ops))))
								((eq? message 'stack) stack)
								((eq? message 'operations) the-ops)
								(else (error "Unknown request -- MACHINE" message))))
				dispatch)))

	(define (get-register machine reg-name)
		((machine 'get-register) reg-name))

	(define (start machine)
		(machine 'start))

	(define (get-register-contents machine register-name)
		(get-contents (get-register machine register-name)))

	(define (set-register-contents! machine register-name value)
		(set-contents! (get-register machine register-name) value)
		'done)

	;; TODO: rest of the wrappers for the new-machine's internal data

	(define (make-machine register-names ops controller-text)
		(let ((machine (make-new-machine)))
			(for-each (lambda (register-name)
									((machine 'allocate-register) register-name))
								register-names)
			((machine 'install-operations) ops)    
			((machine 'install-instruction-sequence)
			 (assemble controller-text machine))
			machine))

	;; TODO: wrappers for the machine's internal data
)
