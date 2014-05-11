(DEF DX (X W S) (- (/ (* S X) (- W 1)) (/ S 2)))
                                                
(DEF TH (X) (> X 50))                           
                    
(DEF PT3 ()                                                                    
(setq fn "test.jpg")                                                           
(setq s22 (getservo 22) s24 (getservo 24) vm 1 loop 1 filt 0 bk (image raw fn))
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
                          
  (setq ni (image raw fn))
                                       
  (setq di  (- ni bk) )                
                                       
  (setq di2 (mapcar th di))            
  (image show (image scale di2 16 10) 1)
                        
  (pr "Diff= " (sum di2))
                   
  (setq bk ni)     
                   
  (setservo 22 s22)
  (setservo 24 s24)
  (WAIT 200)    
)               
(setservo 22 'P)
(setservo 24 'P)
(pr "Done"))
