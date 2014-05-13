!do
(DEF DX (X W S) (- (/ (* S X) (- W 1)) (/ S 2)))

(DEF TH (X) (> X 50))

(DEF PT2 ()
(setq fn "test.jpg")
(setq s22 (getservo 22) s24 (getservo 24) vm 1 loop 1 filt 0 )
(setq bk (image scale (image raw fn) 16 10))
(while  loop
  (setq k (key 0))
  (cond (= k `q) (setq loop 0)
        (= k `a) (setq s22 (+ s22 2))
        (= k `s) (setq s22 (- s22 2))
        (= k `w) (setq s24 (+ s24 2))
        (= k `z) (setq s24 (- s24 2))
        (= k `v) (setq vm   (- 1 vm))
  )

  (pr  k "," s22 "," s24 "," filt "," vm)

  (image WAIT )
  (setq ni (image scale (image raw fn) 16 10))
 
  (setq di (- bk ni)) 
  
  (image show di 1)
  
  (pr "Diff= " (sum di))
  
  (setq bk ni)

  (setservo 22 s22)
  (setservo 24 s24)
)
(setservo 22 'P)
(setservo 24 'P)
(pr "Done"))


