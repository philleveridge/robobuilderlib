using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace RobobuilderLib
{
public  class ReversePolishNotation 
{
    // Associativity constants for operators
    private static int LEFT_ASSOC = 0, RIGHT_ASSOC = 1;
 
    // Supported operators
    private Dictionary<String, int[]> OPERATORS = new Dictionary<String, int[]>();
    
    public ReversePolishNotation()
    {
        // Map<"token", []{precendence, associativity}>
        OPERATORS.Add("+", new int[] { 0, LEFT_ASSOC });
        OPERATORS.Add("-", new int[] { 0, LEFT_ASSOC });
        OPERATORS.Add("*", new int[] { 5, LEFT_ASSOC });
        OPERATORS.Add("/", new int[] { 5, LEFT_ASSOC });
        OPERATORS.Add("%", new int[] { 5, LEFT_ASSOC });
        OPERATORS.Add("^", new int[] { 10, RIGHT_ASSOC });
        OPERATORS.Add("$", new int[] { 10, RIGHT_ASSOC });
    }
 
    /**
     * Test if a certain is an operator .
     * @param token The token to be tested .
     * @return True if token is an operator . Otherwise False .
     */
    private bool isOperator(String token) {
        if (token == "") return false;
        token = token.Substring(0, 1);
        return OPERATORS.ContainsKey(token);
    }
 
    /**
     * Test the associativity of a certain operator token .
     * @param token The token to be tested (needs to operator).
     * @param type LEFT_ASSOC or RIGHT_ASSOC
     * @return True if the tokenType equals the input parameter type .
     */
    private bool isAssociative(String token, int type) {
        token = token.Substring(0, 1);
        if (!isOperator(token)) {
            throw new Exception("Invalid token: " + token);
        }
        if (OPERATORS[token][1] == type) {
            return true;
        }
        return false;
    }
 
    /**
     * Compare precendece of two operators.
     * @param token1 The first operator .
     * @param token2 The second operator .
     * @return A negative number if token1 has a smaller precedence than token2,
     * 0 if the precendences of the two tokens are equal, a positive number
     * otherwise.
     */
     private int cmpPrecedence(String token1, String token2) 
     {
        token1 = token1.Substring(0, 1);
        token2 = token2.Substring(0, 1);

        if (!isOperator(token1) || !isOperator(token2)) {
            throw new Exception("Invalied tokens: " + token1
                    + " " + token2);
        }
        return OPERATORS[token1][0] - OPERATORS[token2][0];
      }

     public String[] infixToRPN(string s) 
     {
         List<string> t = new List<string>();
         string tok = "";
         for (int i = 0; i < s.Length; i++)
         {
             char c = s[i];

             if (char.IsLetterOrDigit(c) || c == '$')
                 tok += c;
             else
             {
                 t.Add(tok);
                 t.Add(c.ToString());
                 tok = "";
             }
         }
         t.Add(tok);
         return infixToRPN( t.ToArray());
     }

     public String[] infixToRPN(String[] inputTokens)
     {
            List<String> outl = new List<String>();
            Stack<String> stack = new Stack<String>();
            // For all the input tokens [S1] read the next token [S2]
            foreach (String token in inputTokens) 
            {
                if (isOperator(token)) {
                    // If token is an operator (x) [S3]
                    while (stack.Count>0 && isOperator(stack.Peek())) {
                        // [S4]
                        if ((isAssociative(token, LEFT_ASSOC) && cmpPrecedence(
                                token, stack.Peek()) <= 0)
                                || (isAssociative(token, RIGHT_ASSOC) && cmpPrecedence(
                                        token, stack.Peek()) < 0)) {
                            outl.Add(stack.Pop());   // [S5] [S6]
                            continue;
                        }
                        break;
                    }
                    // Push the new operator on the stack [S7]
                    stack.Push(token);
                } else if (token=="(") {
                    stack.Push(token);  // [S8]
                } else if (token==")") {
                    // [S9]
                    while (stack.Count>0 && stack.Peek()!="(") {
                        outl.Add(stack.Pop()); // [S10]
                    }
                    stack.Pop(); // [S11]
                } else {
                    outl.Add(token); // [S12]
                }
            }
            while (stack.Count>0) {
                outl.Add(stack.Pop()); // [S13]
            }

            return outl.ToArray();
        }
    }



}
