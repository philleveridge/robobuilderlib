(prn  "Boot loader - robos test")

(reference "HomebrewLib" 
           "System.Windows.Forms")

(using     "RobobuilderLib"
           "System.Windows.Forms"
           "System.IO.Ports")
           
 (load "..\\Lisp\\final.lisp")
 (load "..\\Lisp\\wckutils18.lisp")
 (load "..\\Lisp\\utilities.lisp")
           
(= pcr (new "RobobuilderLib.PCremote" "COM40"))

(.setdbg pcr false)

(prn "ver=" (.readVer pcr))

(do (pr "xyz = " ) (prl (tolist (.readXYZ pcr))))

(prn "psd=" (.readPSD pcr))

(= wck (new "RobobuilderLib.wckMotion" pcr))
(.servoID_readservo wck 0)
(prl (tolist (.pos wck)))

(def standup () (.playpose wck 1000 10 (toByteArray basic18) true) )

(standup)
