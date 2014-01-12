!DO
{SET IM
[ 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
  0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 ;
  0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 0 ;
  0 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 ;
  0 0 0 0 0 0 0 1 1 1 1 1 1 0 0 0 ;
  0 0 0 1 1 1 1 1 1 1 1 1 0 0 0 0 ;
  0 0 0 1 1 1 1 1 1 1 1 1 0 0 0 0 ;
  0 0 0 1 1 0 0 0 0 1 1 0 0 0 0 0 ;
  0 0 0 1 1 0 0 0 0 1 1 0 0 0 0 0 ;
  0 0 0 1 1 0 0 0 0 1 1 0 0 0 0 0 ;
  0 0 0 1 1 1 1 1 1 1 1 0 0 0 0 0 ;
  0 0 0 0 1 1 1 1 1 1 0 0 0 0 0 0 ;
  0 0 0 0 0 1 1 1 1 0 0 0 0 0 0 0 ;
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
  0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ]
}


{FUNC PP {I} {FOR X '{0 15} {FOR Y '{0 15} {PR {IF {CELL I X Y} 'O '_  } ; }} {PR}}
}

{PP IM}

{FUNC CENTER {I} 
     {SET SX 0.0}
     {SET SY 0.0} 
     {SET SM {SUM IM}} 
     {FOR X '{0 15} 
      {FOR Y '{0 15} 
        {IF {CELL I X Y} {DO {SET SX {+ SX X}} {SET SY {+ SY Y}}}  }  } 
     }
     {PR {LIST {/ SX SM} {/ SY SM}}}
}

{FUNC ORIENTATION {I} 
     {SET SA 0.0}
     {SET SB 0.0} 
     {SET SC 0.0} 
     {SET COA {CENTER IM}}
     {SET I0 {CAR COA}}
     {SET J0 {CAR {CDR COA}}}
     {FOR X '{0 15} 
      {FOR Y '{0 15} 
        {IF {CELL I X Y} 
         {DO 
             {SET SA {+ SA {* {- X I0} {- X I0}}}} 
             {SET SB {+ SB {* 2 {* {- X I0} {- Y J0}}}}}
             {SET SC {+ SC {* {- Y J0} {- Y J0}}}}
         }  }  } 
     }
     {LIST COA {/ {/ {ATAN  SB} {- SA SC }} 2.0}}
}


{ORIENTATION IM}

{FUNC EULER {I} 
  {FOR X '{0 15} 
    {FOR Y '{0 15} 
        {IF {NOT {CELL I X Y}}  
        {  SET S { +     
           { AND {NOT {CELL I {+ X 1} Y}} {NOT {CELL I X {+ Y 1}}}  {CELL I {+ X 1} {+ Y 1}} } 
           { AND {CELL I {+ X 1} Y} {CELL I X {+ Y 1}}  {CELL I {+ X 1} {+ Y 1}} }   
        }  
        }
        }
    } 
  }
}






