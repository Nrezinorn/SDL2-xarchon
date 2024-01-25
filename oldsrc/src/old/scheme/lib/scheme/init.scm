(if (not (defined? 'provide))
   (primitive-load-path 'ice-9/boot-9.scm)
)

(primitive-load-path 'ice-9/slib.scm)
(require 'struct)

;;;; actor
;
;      side            == light dark
;      name            == knight sorceress ... etc
;      board-actor     == (points at board-actor record)
;      field-actor     == (points at field-actor)
;
;;;; board-actor
;
;      class           == ground fly master elemental
;      distance        == 3 4 5
;      health          == ?
;
;;;; field-actor
;
;      x , y           == 0 .. 8
;      speed           == normal slow
;      weapon          == (points at field-weapon)
;
;;;; field-weapon
;
;      x , y           == 0 .. 8
;      speed           == normal fast
;      recharge        == 0 (ready to fire) .. whatever (recharging)

(define-record actor (side name board-actor field-actor))
(define-record board-actor (class distance health))
(define-record field-actor (x y speed weapon))
(define-record field-weapon (x y speed ready? recharge))

;;;; board-cell
;
;      luminance       == 0 (dark) .. 7 (light)
;      power           == #t (cell is power point)
;      actor           == (points at actor record)

(define-record board-cell (luminance power actor))

#t
