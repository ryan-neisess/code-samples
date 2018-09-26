/* WinForms Spreadsheet
 * Ryan Neisess, WSU
 * CptS 321, Spring 2018, Dr. Aaron Crandall
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CptS321
{
    public class ExpTree
    {
        // Builds the tree with the given expression
        public ExpTree(string expression, Spreadsheet srcSs, Cell srcCell)
        {
            sourceSpreadsheet = srcSs;
            sourceCell = srcCell;
            SetExpr(expression);
        }

        // Parses the expression and builds the tree
        // For now, parses infix expressions
        public void SetExpr(string newExpr)
        {
            ConvertToRPN(newExpr);
            ParseAndBuildPostfix(ref root);
        }

        // Converts an infix expression string into a RPN/postfix
        //   expression stack of strings, where each string 
        //   represents either an operator, a value, or a variable
        // Additional precondition: parameter expr must have = removed
        private void ConvertToRPN(string expression)
        {
            expression = expression.Replace(" ", ""); // Remove all spaces
            string tempStr1 = "", tempStr2 = "";
            StringBuilder tempStr3 = new StringBuilder();

            // Put delimiting commas (',') on both sides of each operator
            foreach (char character in operators)
            {
                tempStr1 = character.ToString();
                if (character == '(')
                {
                    tempStr3.Append(tempStr1).Append(",");
                }
                else if (character == ')')
                {
                    tempStr3.Append(",").Append(tempStr1);
                }
                else
                {
                    tempStr3.Append(",").Append(tempStr1).Append(",");
                }
                tempStr2 = tempStr3.ToString();

                expression = expression.Replace(tempStr1, tempStr2);

                tempStr3.Clear();
            }

            // Create an array of strings from the infix expression
            string[] infixArr = expression.Split(',');

            // Converts the infix expression to RPN via implementing Dijkstra's
            //   shunting-yard algorithm
            Stack<string> opStack = new Stack<string>();
            foreach (string strItem in infixArr)
            {
                if (!operators.Contains(strItem[0])) // Not an operator
                {
                    expr.Push(strItem);
                }
                else if (operators.Contains(strItem[0]) && 
                    strItem[0] != '(' && strItem[0] != ')')
                {
                    // Temporary solution to precedence
                    if (strItem[0] == '*' || strItem[0] == '/')
                    {
                        while (opStack.Count > 0 && 
                            (opStack.Peek()[0] == '*' || opStack.Peek()[0] == '/') &&
                            opStack.Peek()[0] != '(')
                        {
                            expr.Push(opStack.Pop());
                        }
                    }
                    else if (strItem[0] == '+' || strItem[0] == '-')
                    {
                        while (opStack.Count > 0 &&
                            (opStack.Peek()[0] == '*' || opStack.Peek()[0] == '/' || opStack.Peek()[0] == '+' || opStack.Peek()[0] == '-') &&
                            opStack.Peek()[0] != '(')
                        {
                            expr.Push(opStack.Pop());
                        }
                    }
                    opStack.Push(strItem);
                }
                else if (strItem[0] == '(')
                {
                    opStack.Push(strItem);
                }
                else if (strItem[0] == ')')
                {
                    while (opStack.Peek()[0] != '(')
                    {
                        expr.Push(opStack.Pop());
                    }
                    opStack.Pop();
                }
            }

            foreach (string opItem in opStack)
            {
                expr.Push(opItem);
            }
        }

        // Creates the expression tree from a postfix expression string
        private void ParseAndBuildPostfix(ref ExpTreeNode tree)
        {
            if (expr.Count == 0)
            {
                return;
            }

            string currItem = expr.Peek();
            if (Char.IsDigit(currItem[0])) // The item is a numerical value
            {
                double num = 0.0;
                if (Double.TryParse(currItem, out num))
                {
                    tree = new ExpTreeValNode(num);
                    expr.Pop();
                }
                else
                {
                    // If the number is invalid, treat the node like a VarNode
                    // Tree will still be built, and exception will be thrown during evaluation
                    tree = new ExpTreeVarNode(currItem, sourceSpreadsheet);
                    expr.Pop();
                }
            }
            // Note: Variables are assumed to be a single letter 
            //   immediately followed by a number
            // Precondition: The number following the letter must be an int
            else if (Char.IsLetter(currItem[0]) && 
                Char.IsDigit(currItem[1])) // The item is a variable
            {
                tree = new ExpTreeVarNode(currItem, sourceSpreadsheet);

                // Adds the cell containing the formula to the var node cell's
                //   list of cells that depend on the var node cell
                sourceSpreadsheet.GetCell(currItem).cellRefList.AddFirst(sourceCell);

                expr.Pop();
            }
            else if (operators.Contains(currItem[0])) // The item is an operator
            {
                switch (currItem[0])
                {
                    case '+':
                        tree = new ExpTreeAddNode();
                        break;
                    case '-':
                        tree = new ExpTreeSubNode();
                        break;
                    case '*':
                        tree = new ExpTreeMultNode();
                        break;
                    case '/':
                        tree = new ExpTreeDivNode();
                        break;
                }
                ExpTreeOpNode temp = (ExpTreeOpNode)tree;
                expr.Pop();
                ParseAndBuildPostfix(ref temp.GetRight());
                ParseAndBuildPostfix(ref temp.GetLeft());
            }
            else // Throw an exception for incorrect input format
            {
                // Tree will still be built, and exception will be thrown during evaluation
                tree = new ExpTreeVarNode(currItem, sourceSpreadsheet);
                expr.Pop();
            }
        }

        // Precondition: tree is not empty
        public double Eval()
        {
            // Is this good/safe polymorphism style?
            return root.Evaluate();
        }

        public bool IsEmpty()
        {
            return root == null;
        }

        private ExpTreeNode root = null;
        private Stack<string> expr = new Stack<string>(); // Will store final RPN expression
        private char[] operators = { '+', '-', '*', '/', '(', ')' };

        // Allows ExpTree to pass these along to VarNodes
        private Spreadsheet sourceSpreadsheet;
        private Cell sourceCell;
    }
}
