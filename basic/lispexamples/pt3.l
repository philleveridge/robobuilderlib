!DO

(DEF DX (X W S) (- (/ (* S X) (- W 1)) (/ S 2)))
                                                
(DEF TH (X) (> X 50))                           
                    
(DEF PT3 ()                                                                    
(setq fn   "test.jpg" 
      s22  (getservo 22) 
      s24  (getservo 24) 
      vm   1 
      loop 1 
      filt 0 
      bk   (image raw fn)
)
(while  1 
  (setq k (key 0)) 
  (cond (= k `q) (break)
        (= k `a) (setq s22 (+ s22 2))
        (= k `s) (setq s22 (- s22 2))
        (= k `w) (setq s24 (+ s24 2))
        (= k `z) (setq s24 (- s24 2))
        (= k `v) (setq vm   (- 1 vm))
  )

  (setq i (IMAGE raw fn 1))
  (if vm (image show (image scale i 32 24)))

  (setq fd (IMAGE F-DETECT i 1)   )

  (if fd (do (pr "FD=" fd) (break)))
  (image wait 1000)                                 
                                         
)                
(setservo 22 'P)
(setservo 24 'P)
(pr "Done")
(exit)
)

(pt3)

