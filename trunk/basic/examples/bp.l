!DO 

{PRINT "Step 1&3) set up"}

{SET MW [-1.0 1.0 2.0 -1.0]}
{SET MX [0.6 0.8 0.0 1.0]}
{SET MV [2.0 1.0 0.0 ;1.0 2.0 2.0 ;0.0 3.0 1.0 ;0.0 0.0 -1.0]}

{PRINT "Feed forward"}
{PRINT "Step 4) input layer"}
{PRINT "Step 5) hidden layer"}
{SET MS {+ [0 0 0 1] {SIG {* MX MV}}}}

{PR MS}

{PRINT "back propagate"}
{SET ERR 0.9}
{PRINT "Step 6) output layer"}
{SET MY {SIG {* MW {TRN MS}}}}

{SET MY {MAPCAR {LAMBDA {X} {* {- ERR X} {DSIG X}}} MY}}

{PRINT "Step 7) hidden layer"}
{SET MZ {.* {* MY MW} {DSIG MS}}}


{PRINT "Step  8) update weights"}
{SET ALPHA 0.3}
