!do
; mu = Omega -1 . Chi
; Omega 3x3  Chi 1 x3
; -3 +5 +3
; based on CS373 course on AI from Udacity 

(def update (m1 m2 a b v s)
	(setc m1  a a  (+ (cell m1 a a) s) )
	(setc m1  a b  (- (cell m1 a b) s) ) 
	(setc m1  b a  (- (cell m1 b a) s) ) 
	(setc m1  b b  (+ (cell m1 b b) s) ) 

	(setc m2  0 a  (- (cell m2 0 a) (* s v)) )
	(setc m2  0 b  (+ (cell m2 0 b) (* s v)) )

	(list m1 m2)
)

(def slam (data)
	(print "slam")
	(setq OM (zero 4 4))
	(setq CI (zero 4 1))
	(setq n 0 m 1)

	(setq (x0 z0) (car data))
	(pr   n " .. " x0 " .. " z0)
	(setc OM  0 0  1) 
	(setc CI  0 0  x0)
	(setq (om ci)	(update om ci 0 3 z0 1))

	(while (not (null (setq data (cdr data))))
		(setq (xi zi) (car data))
		(pr m " .. " xi " .. " zi)
		(setq (om ci)	(update om ci n m xi 1))
		(setq (om ci)	(update om ci m 3 zi 1))
		(inc 'n)
		(inc 'm)
	)

;	(pr "omega=" OM "\nchi=" CI)
	(pr "mu=" (* (INV OM) CI))
)


;number of landmarks
(setq num_landmarks     5     )   
   
; time steps
(setq NS                20    ) 
   
(setq world_size        100.0 )   

;range at which we can sense landmarks
(setq measurement_range  50.0 )  
  
;noise in robot motion
(setq motion_noise       2.0  ) 
    
;noise in robot motion
(setq measurement_noise  2.0  )  

;distance by which robot (intends to) move each iteratation    
(setq distance     	20.0  )  

;data = make_data(N num_landmarks world_size measurement_range motion_noise measurement_noise distance)
;
;result = slam(data N num_landmarks motion_noise measurement_noise)   

(setq om 0 ci 0)

(setq test_data1  '((((1 19.457599255548065 23.8387362100849) (2 -13.195807561967236 11.708840328458608) (3 -30.0954905279171 15.387879242505843)) (-12.2607279422326 -15.801093326936487)) (((2 -0.4659930049620491 28.088559771215664) (4 -17.866382374890936 -16.384904503932)) (-12.2607279422326 -15.801093326936487)) (((4 -6.202512900833806 -1.823403210274639)) (-12.2607279422326 -15.801093326936487)) (((4 7.412136480918645 15.388585962142429)) (14.008259661173426 14.274756084260822)) (((4 -7.526138813444998 -0.4563942429717849)) (14.008259661173426 14.274756084260822)) (((2 -6.299793150150058 29.047830407717623) (4 -21.93551130411791 -13.21956810989039)) (14.008259661173426 14.274756084260822)) (((1 15.796300959032276 30.65769689694247) (2 -18.64370821983482 17.380022987031367)) (14.008259661173426 14.274756084260822)) (((1 0.40311325410337906 14.169429532679855) (2 -35.069349468466235 2.4945558982439957)) (14.008259661173426 14.274756084260822)) (((1 -16.71340983241936 -2.777000269543834)) (-11.006096015782283 16.699276945166858)) (((1 -3.611096830835776 -17.954019226763958)) (-19.693482634035977 3.488085684573048)) (((1 18.398273354362416 -22.705102332550947)) (-19.693482634035977 3.488085684573048)) (((2 2.789312482883833 -39.73720193121324)) (12.849049222879723 -15.326510824972983)) (((1 21.26897046581808 -10.121029799040915) (2 -11.917698965880655 -23.17711662602097) (3 -31.81167947898398 -16.7985673023331)) (12.849049222879723 -15.326510824972983)) (((1 10.48157743234859 5.692957082575485) (2 -22.31488473554935 -5.389184118551409) (3 -40.81803984305378 -2.4703329790238118)) (12.849049222879723 -15.326510824972983)) (((0 10.591050242096598 -39.2051798967113) (1 -3.5675572049297553 22.849456408289125) (2 -38.39251065320351 7.288990306029511)) (12.849049222879723 -15.326510824972983)) (((0 -3.6225556479370766 -25.58006865235512)) (-7.8874682868419965 -18.379005523261092)) (((0 1.9784503557879374 -6.5025974151499)) (-7.8874682868419965 -18.379005523261092)) (((0 10.050665232782423 11.026385307998742)) (-17.82919359778298 9.062000642947142)) (((0 26.526838150174818 -0.22563393232425621) (4 -33.70303936886652 2.880339841013677)) (-17.82919359778298 9.062000642947142))))


(def slamv2 (data NS NLM MESN MOTN)
	(print "slam v2")
	(print "Input: " NS ", " NLM "," (len data))
	(print "dim=" (setq dm (* 2 (+ NS NLM))))
	(setq s1  (/ 1.0 MESN) s2  (/ 1.0 MOTN) )

	(setq OM (zero dm dm))
	(setq CI (zero dm 1))

	(setq x0 (/ world_size 2))

	(setc OM  0 0  1) 
	(setc OM  0 1  1) 

	(setc CI  0 0  x0)
	(setc CI  0 1  x0)

	(setq nk 0)
	(foreach x data
		(setq (meas mot) x)

		(foreach m meas  
			(setq (nn mx my) m)
			(pr nk "," (setq ni (* 2 (+ NS nn))))

			(pr nn "x=" mx)
			(setq (om ci)	(update om ci nk ni mx s1))

			(pr nn "y=" my)
			(setq (om ci)	(update om ci (+ nk 1) (+ ni 1) my s1))
		)

		(print "mot x " (car mot) )
		(setq (om ci)	(update om ci nk  (+ nk 2) (car mot)  s2))

		(print "mot y " (cadr mot))
		(setq (om ci)	(update om ci (+ nk 1) (+ nk 3) (cadr mot) s2))

		(setq nk (+ nk 2))
	)

;	(pr "omega=" OM "\nchi=" CI)
	(pr "mu=" )
; (* (INV OM) CI))
)
        
(def pr-result (l)
	(setq i 0)
	(while (< i (row l))
	   (pr (cell l 0 i) "," (cell l 0 (+ i 1))) (setq i (+ i 2)))
)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

;slam (data = (init measurement) (rel. motion measurement) (....))
;(slam '( (-3 10)  ( 5  5)  ( 3  1)))

(slamv2 test_data1 NS num_landmarks  measurement_range motion_noise measurement_noise)





