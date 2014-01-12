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

{FUNC PP {I} {FOR X '{0 15} {FOR Y '{0 15} {PR {IF {CELL I X Y} 'O '_  } ; }} {PR}}}

{PP IM}

{FUNC CENTER {I} 
     {SET SX 0.0 SY 0.0 SM {SUM IM}} 
     {FOR X '{0 15} 
      {FOR Y '{0 15} 
        {IF {CELL I X Y} {DO {SET SX {+ SX X}} {SET SY {+ SY Y}}}  }  } 
     }
     {LIST {/ SX SM} {/ SY SM}}
}

{FUNC ORIENTATION {I} 
     {SET SA 0.0 SB 0.0 SC 0.0 {X0 Y0} {CENTER IM}}
     {FOR X '{0 15} 
      {FOR Y '{0 15} 
        {IF {CELL I X Y} 
           {DO 
             {LET SA = SA + ((X - X0) * (X - X0))     }
             {LET SB = SB + (2 * (X - X0) * (Y - Y0)) }
             {LET SC = SC + ((Y - Y0) * (Y - Y0))     }
           }  
        }  
      } 
     }
     {LIST X0 Y0 {/ {/ {ATAN  SB} {- SA SC }} 2.0}}
}

{FUNC EULER {I} 
  {SET S1 0 S2 0}
  {FOR X '{0 14} 
    {FOR Y '{0 14} 
      {IF {NOT {CELL I X Y}} 
        {DO 
        {SET S1 { + S1 { AND {NOT {CELL I {+ X 1} Y}} {NOT {CELL I X {+ Y 1}}}  {CELL I {+ X 1} {+ Y 1}}}}} 
        {SET S2 { + S2 { AND {CELL I {+ X 1} Y} {CELL I X {+ Y 1}}  {CELL I {+ X 1} {+ Y 1}} }}}   }
      }  
    }
  }
  {PR S1 ; S2 ; {- S1 S2}}
}

{FUNC PERIMETER {I}
  {SET S1 0}
  {FOR X '{0 14} 
    {FOR Y '{0 14} {DO
        {IF {AND {CELL I X Y} {NOT {CELL I {+ X 1} Y}}} {INC S1}}
   	{IF {AND {NOT {CELL I X Y}} {CELL I {+ X 1} Y}} {INC S1}}
        {IF {AND {CELL I X Y} {NOT {CELL I X {+ Y 1}}}} {INC S1}}
        {IF {AND {NOT {CELL I X Y}} {CELL I X {+ Y 1}}} {INC S1}}
      }  
    }
  }
  {PR S1}
}









