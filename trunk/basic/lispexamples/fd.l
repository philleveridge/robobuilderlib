(IMAGE F-DETECT "img2.jpg" 1)
IMAGE PGM (IMAGE F-DETECT "img2.jpg" 1) "face2.pgm"

SETQ I (IMAGE F-DETECT "img2.jpg" 1)

IMAGE PGM I "face3.pgm"

CELL I 0 0


IMAGE PGM (IMAGE NEW 10 10) "out.pgm"


DRAWLINE (IMAGE NEW 10 10)  1 2 3 4 

IMAGE SHOW (IMAGE DRAWLINE (IMAGE NEW 10 10) 2 2 4 4)
IMAGE SHOW (IMAGE DRAWRECT (IMAGE NEW 10 10) 2 2 4 4)


SETQ I (IMAGE RAW "test.jpg")
SETQ M (MAT 'I 10 10)
SETQ I2 (IMAGE MAT M)

for R '(0 9) (for C '(0 9) (pr (cell i2 c r) ; )) (pr)

setq ma [1 2;3 4]
setc ma 1 1 1.5

SETQ P (PIPE 5)
push p 1
push p 2
push p 3
foreach x p (pr x)
pop p
pop p
pop p
pop p

SETQ I (IMAGE NEW 10 10)
SETC I 1 1 200
SETC I 2 2 200
SETC I 3 3 200
IMAGE SHOW I
IMAGE TRAIN I 1
IMAGE RECOG I

RSHP I 5 5

