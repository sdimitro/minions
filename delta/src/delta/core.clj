(ns delta.core)

;; Most of code below is attributed to kachayev
;; Link: https://gist.github.com/kachayev/b5887f66e2985a21a466

;; takes any input and "consume" first char from it
(defn any [input]
  (if (empty? input) '()
      (list [(first input)
             (apply str (rest input))])))

;; this one doesn't accept any input
(defn failure [_] '())

(defn parse [parser input]
  (parser input))

(defn parse-all [parser input]
  (->> input
       (parse parser)
       (filter #(= "" (second %)))
       ffirst))

;; MONADS

;; builds parser that always returns given element without consuming (changing) input
(defn return [v]
  (fn [input] (list [v input])))

;; takes parser and function that builds new parsers from (each) result of applying first one
(defn >>= [m f]
  (fn [input]
    (->> input
         (parse m)
         (mapcat (fn [[v tail]] (parse (f v) tail))))))

;; for Haskell-like do-notation
(defn merge-bind [body bind]
  (if (and (not= clojure.lang.Symbol (type bind))
           (= 3 (count bind))
           (= '<- (second bind)))
    `(>>= ~(last bind) (fn [~(first bind)] ~body))
    `(>>= ~bind (fn [~'_] ~body))))

(defmacro do* [& forms]
  (reduce merge-bind (last forms) (reverse (butlast forms))))

;; BASIC PARSERS

(defn sat [pred]
	(>>= any (fn [v] (if (pred v) (return v) failure))))

;; just a helper
(defn char-cmp [f]
  (fn [c] (sat (partial f (first c)))))

;; recognizes given char
(def match (char-cmp =))

;; rejects given char
(def noneOf (char-cmp not=))

;; just a helper
(defn from-re [re]
  (sat (fn [v] (not (nil? (re-find re (str v)))))))

;; recognizes any digit
(def digit (from-re #"[0-9]"))

;; recognizes any letter
(def letter (from-re #"[a-zA-Z]"))

;; COMBINATORS

;; (ab)
(defn and-then [p1 p2]
  (do*
   (r1 <- p1)
   (r2 <- p2)
   ;; xxx: note, that it's dirty hack to use STR to concat outputs
   ;; Full functional implementation should use MonadPlus protocol
   (return (str r1 r2))))

;; (a|b)
(defn or-else [p1 p2]
  (fn [input]
    (lazy-cat (parse p1 input) (parse p2 input))))

(declare plus)
(declare optional)

;; (a*)
(defn many [parser] (optional (plus parser)))

;; (a+) equals to (aa*)
(defn plus [parser]
  (do*
   (a <- parser)
   (as <- (many parser))
   (return (cons a as))))

;; (a?)
(defn optional [parser] (or-else parser (return "")))

;; COMBINATOR USAGE

;; recognizes space (or newline)
(def space (or-else (match " ") (match "\n")))

;; recognizes empty string or arbitrary number of spaces 
(def spaces (many space))

;; recognizes given string, i.e. "clojure"
(defn string [s] (reduce and-then (map #(match (str %)) s)))
