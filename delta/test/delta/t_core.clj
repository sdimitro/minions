(ns delta.t-core
  (:use midje.sweet)
  (:use [delta.core]))

(def test-clj-version (do*
            	          (string "clojure")
            	          (match " ")
            	          (major <- digit)
            	          (match ".")
            	          (minor <- digit)
            	          (return (str "major: " major "; minor: " minor))))

(facts "general"
	(any "clojure-1.7") 							=> '([\c "lojure-1.7"])
	(parse (match "c") "clojure1.7")  => '([\c "lojure1.7"])
	(parse letter "clojure1.7") 			=> '([\c "lojure1.7"])
	(parse digit "1.7clojure") 				=> '([\1 ".7clojure"])
)

(facts "testing clojure version string"
	(parse-all test-clj-version "clojure 1.7")  => "major: 1; minor: 7"
)
