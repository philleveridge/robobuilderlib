!FUNC COUNT (x y) 
     (IF (NULL y) 
          0 
          (+ (= x (CAR y)) (COUNT x (CDR y)) )
     )



