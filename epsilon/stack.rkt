(module stack racket

  (provide make-stack push pop backtrace top reinit-stack stack-size)

	(define (make-stack)
  (let ((s '()))
    (define (push x)
      (set! s (cons x s)))
    (define (pop)
      (if (null? s)
          (error "Empty stack -- POP")
          (let ((top (car s)))
            (set! s (cdr s))
            top)))
    (define (initialize)
      (set! s '())
      'done)
    (define (dispatch message)
      (cond ((eq? message 'push) push)
            ((eq? message 'pop) (pop))
            ((eq? message 'top) (car s))
            ((eq? message 'bt)  s)
            ((eq? message 'size)  (length s))
            ((eq? message 'initialize) (initialize))
            (else (error "Unknown request -- STACK"
                         message))))
    dispatch))


	(define (pop stack)
  	(stack 'pop))

	(define (push stack value)
  	((stack 'push) value))

	(define (top stack)
  	(stack 'top))

	(define (backtrace stack)
  	(stack 'bt))

	(define (reinit-stack stack)
  	(stack 'initialize))

	(define (stack-size stack)
  	(stack 'size))
)
