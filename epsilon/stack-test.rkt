(require rackunit "stack.rkt")

(define stack-tests
  (test-suite
    "Tests for the Stack"

    (test-case "Initial Stack Properties"
    (let  ((s (make-stack)))
      (check-equal? (stack-size s) 0 "stack initially should have size 0.")
      (check-equal? (backtrace s) '() "stack initially should be empty.")))

    (test-case "Manipulating Stack State"
    (let  ((s (make-stack)))
			(push s 23)
      (check-equal? (top s) 23 "top should give the most recently pushed item")
			(check-equal? (backtrace s) '(23) "first backtrace pass")
      (check-equal? (stack-size s) 1 "stack should have size 1.")

			(push s 32)
      (check-equal? (top s) 32 "top should give the most recently pushed item")
			(check-equal? (backtrace s) '(32 23) "second backtrace pass")
      (check-equal? (stack-size s) 2 "stack should have size 2.")

			(check-equal? (pop s) 32 "pop should extract and return the top")
      (check-equal? (top s) 23 "top should give the most recently pushed item")
			(check-equal? (backtrace s) '(23) "third backtrace pass")
      (check-equal? (stack-size s) 1 "stack should have size 1.")

			(reinit-stack s)
			(check-equal? (backtrace s) '() "stack should be empty after reinitialization.")
      (check-equal? (stack-size s) 0 "a reinitialized stack should have size 0.")))
))

(require rackunit/text-ui)
(run-tests stack-tests)
