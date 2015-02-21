(ns gamma.core)

;
; REGISTERS
;

(defn make-register
	"Create a register"
	[name]
		(let [contents (atom '*unassigned*)]
		(defn dispatch [message]
			(cond (= message 'get) @contents
						(= message 'set)  #(reset! contents %)
						:else (print "Unknown request -- REGISTER" message))))
	dispatch)

(defn get-contents [register]
  (register 'get))

(defn set-contents! [register value]
  ((register 'set) value))

;
; STACK
;

(defn make-stack
	"create a stack"
	[]
  (let [s (atom '())]
    (defn spush [x]
      (reset! s (conj @s x)))
    (defn spop []
      (if (empty? @s)
          (print "Empty stack -- POP")
          (let [top (first @s)]
            (swap! s pop)
            top)))
    (defn sinit []
      (reset! s '())
      'done)
    (defn dispatch [message]
      (cond (= message 'push) spush
            (= message 'pop)  (spop)    ; TODO: (annoying) spop alone use to return just the function
            (= message 'init) (sinit)   ; TODO: same thing for sinit as spop
            (= message 'bt) @s 					; prints out stack
            (= message 'top) (first @s)
            :else (print "Unknown request -- STACK" message)))
    dispatch))

(defn stack-pop [stack]
	(stack 'pop))

(defn stack-push [stack value]
	((stack 'push) value))

;
; MACHINE
;

(defn make-new-machine
	"Create machine with the following state:
			- a stack
			- an instruction sequence
			- a register table"
	[]
  (let [pc (atom (make-register 'pc))
        flag (atom (make-register 'flag))
        stack (atom (make-stack))
        the-instruction-sequence (atom '())]
    (let [the-ops
          (atom ['(:initialize-stack (fn [] (stack 'init)))])
          register-table
          (atom {:pc pc :flag flag})]

      (defn allocate-register [name]
        (if (contains? @register-table name)
            (print "Multiply defined register: " name)
            ((swap! register-table
                   assoc name (make-register name))
                        register-table))
        'register-allocated)

      (defn lookup-register [name]
        (let [value (name @register-table)]
          (if value
              value
              (print "Unknown register:" name))))

      (defn execute []
        (let [insts (get-contents @pc)]
          (if (nil? insts)
              'done
              (do
 ;               (instruction-execution-proc (first insts))
                (execute)))))

      (defn dispatch [message]
        (cond (= message 'start)
               (do
								(set-contents! pc the-instruction-sequence)
               	(execute))

              (= message 'install-instruction-sequence)
               (fn [instr-seq] (reset! the-instruction-sequence instr-seq))

              (= message 'allocate-register) allocate-register

              (= message 'get-register) lookup-register

              (= message 'install-operations)
               (fn [ops] (swap! the-ops conj ops))

              (= message 'stack) @stack
              (= message 'operations) @the-ops
              :else (print "Unknown request -- MACHINE" message)))

      dispatch)))
