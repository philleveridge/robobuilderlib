!do
(DEF DX (X W S) (- (/ (* S X) W) (/ S 2)))

(DEF PT ()
(setq s22 (getservo 22))
(setq s24 (getservo 24))
(setq loop 1)
(while  loop
  (pr "hi " k "," s22 "," s24)
  (setq k (key))
  (if (= k 113) (setq loop 0))
  (if (= k 97)  (setq s22 (+ s22 2)))
  (if (= k 115) (setq s22 (- s22 2)))
  (if (= k 119) (setq s24 (+ s24 2)))
  (if (= k 122) (setq s24 (- s24 2)))
  (setservo 22 s22)
  (setservo 24 s24)
  (image WAIT 100)
  (setq I (image RAW "test.jpg"))
  (setq IS (IMAGE SCALE I 32 32))
  
  (IMAGE SHOW IS)
  (setq (x y) (IMAGE MOMENT IS))
  (pr x "," y ": " (dx x 320 4) ", " (dx y 160 5))
  
)
(setservo 22 'P)
(setservo 24 'P)
(pr "Done"))

