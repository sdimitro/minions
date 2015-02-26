(module assembler racket

	(require scheme/mpair) ;; To use set-mcdr!
	(require "machine.rkt")

  (provide update-insts! extract-labels assemble)

	(define (make-label-entry label-name insts)
		(cons label-name insts))

	(define (lookup-label labels label-name)
		(let ((val (assoc label-name labels)))
			(if val
					(cdr val)
					(error "Undefined label -- ASSEMBLE" label-name))))

	(define (make-instruction text)
		(cons text '()))

	(define (instruction-text inst)
		(car inst))

	(define (instruction-execution-proc inst)
		(cdr inst))

	(define (set-instruction-execution-proc! inst proc)
		(set-mcdr! inst proc))

	(define (update-insts! insts labels machine)
		(let ((pc (get-register machine 'pc))
					(flag (get-register machine 'flag))
					(stack (machine 'stack))
					(ops (machine 'operations)))
			(for-each
			 (lambda (inst)
				 (set-instruction-execution-proc! 
					inst
					(make-execution-procedure
					 (instruction-text inst) labels machine
					 pc flag stack ops)))
			 insts)))

	(define (extract-labels text receive)
		(if (null? text)
				(receive '() '())
				(extract-labels (cdr text)
				 (lambda (insts labels)
					 (let ((next-inst (car text)))
						 (if (symbol? next-inst)
								 (receive insts
													(cons (make-label-entry next-inst
																									insts)
																labels))
								 (receive (cons (make-instruction next-inst)
																insts)
													labels)))))))

	(define (assemble controller-text machine)
		(extract-labels controller-text
			(lambda (insts labels)
				(update-insts! insts labels machine)
				insts)))
)
