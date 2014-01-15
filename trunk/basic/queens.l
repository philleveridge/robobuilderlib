!DO 
{FUNC THREAT {I J A B}
{OR {= I A}
    {= J B}
    {= {- I J} {- A B}}
    {= {+ I J} {+ A B}}
}
}
{FUNC CONFLICT {N M BOARD}


