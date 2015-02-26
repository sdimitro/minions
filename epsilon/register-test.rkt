(require rackunit "register.rkt")

(define register-tests
  (test-suite
    "Tests for the Register"

    (test-case "Initial Register Properties"
    (let  ((r (make-register 2)))
      (check-equal? (r 'get) '*unassigned* "register initially should be unassigned.")
      (check-equal? (get-contents r) '*unassigned* "register initially should be unassigned.")))

    (test-case "Setting Register State"
    (let  ((r (make-register 2)))
			(set-contents! r 23)
      (check-equal? (r 'get)  23 "register should be changed to 23.")
      (check-equal? (get-contents r) 23 "register should be changed to 23.")))
))

(require rackunit/text-ui)
(run-tests register-tests)
