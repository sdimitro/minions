(ns gamma.t-core
  (:use midje.sweet)
  (:use [gamma.core]))

(facts "About make-register"
	(let [x (make-register 1)
				y (make-register 2)]
	(fact "get-contents -- initially"
		(get-contents x) => '*unassigned*
		(get-contents y) => '*unassigned*
	)
	(fact "set_contents -- 1st check"
		(set-contents! x 10)
		(get-contents x) => 10
		(get-contents y) => '*unassigned*
	)
	(fact "set_contents -- 2nd check"
		(get-contents x) => 10
		(set-contents! x 11)
		(get-contents x) => 11
		(get-contents y) => '*unassigned*
	))) ; TODO: Error messages?

(facts "about make-stack"
	(let [x (make-stack)]
	(fact "initial-testing"
		(x 'bt) => '())
	(fact "push"
		(stack-push x 23) => '(23)
		(x 'bt) => '(23)
		(stack-push x 32) => '(32 23)
		(x 'bt) => '(32 23)
		(x 'top) => 32)
	(fact "pop"
		(stack-pop x) => 32
		(x 'bt) => '(23)
		(x 'top) => 23)))
