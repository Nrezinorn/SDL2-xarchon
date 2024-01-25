;; constants

(define knight 101)
(define archer 102)
(define valkyrie 103)

;; turn callback

(define (joeblow-board-turn side data parms)
   (display "Inside joeblow-BOARD-turn, side is ")
   (display side)
   (display " and data is ")
   (display data)
   (newline)
   (if (eq? data '()) (begin
         (display "First time around!")
         (set! data 1)
         (display "After assingment, data is ==> ")
         (display data)
      ) (begin
         (display "Not First Time.  Data is ")
         (display data)
         (set! data (+ data 1))
      )
   )
   (newline)
   data
)

(define (joeblow-field-turn side data parms)
   (display "Inside joeblow-FIELD-turn side is ")
   (display side)
   (display " and data is ")
   (display data)
   (newline)
   (if (eq? data '()) (begin
         (display "First time around!")
         (set! data 1)
         (display "After assingment, data is ==> ")
         (display data)
      ) (begin
         (display "Not First Time.  Data is ")
         (display data)
         (set! data (+ data 1))
      )
   )
   (newline)
   data
)

;; main
;; returns (list BOARD FIELD)
;; BOARD is the board procedure
;; FIELD is the field procedure

(define (joeblow-start side1)
   (let ((side side1) (data ()))
      (list
         (lambda (parms)
            (set! data (joeblow-board-turn side data parms))
         )
         (lambda (parms)
            (set! data (joeblow-field-turn side data parms))
         )
      )
   )
)
