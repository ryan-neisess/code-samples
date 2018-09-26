/* WinForms Spreadsheet
 * Ryan Neisess, WSU
 * CptS 321, Spring 2018, Dr. Aaron Crandall
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using CptS321;

namespace ExpTreeDemo
{
    class Program
    {
        static void Main(string[] args)
        {
            /*
            // Commented out due to updated implementation of ExpTree for Spreadsheets

            string demoExpr = "4.5/1.5+((5-3)*1+2)", tempVarName = "";
            ExpTree demoTree = new ExpTree(demoExpr);
            int option = 0;
            double tempVarVal = 0.0, result = 0.0;
            do
            {
                Console.WriteLine("Expression Tree Demo Menu");
                Console.WriteLine("    Current expression: {0}", demoExpr);
                Console.WriteLine("    (Note: Variable names must be in spreadsheet cell format; e.g., C12)");
                Console.WriteLine("  1 - Enter new expression");
                Console.WriteLine("  2 - Set variable value");
                Console.WriteLine("  3 - Evaluate tree");
                Console.WriteLine("  4 - Quit demo");
                option = Convert.ToInt32(Console.ReadLine());

                switch (option)
                {
                    case 1:
                        Console.Write("Enter a new expression: ");
                        demoExpr = Console.ReadLine();
                        demoTree.SetExpr(demoExpr);
                        break;
                    case 2:
                        Console.Write("Enter a new variable name: ");
                        tempVarName = Console.ReadLine();
                        Console.Write("Enter a new value for {0}: ", tempVarName);
                        tempVarVal = Convert.ToDouble(Console.ReadLine());
                        demoTree.varTable.Add(tempVarName, tempVarVal);
                        break;
                    case 3:
                        result = demoTree.Eval();
                        Console.WriteLine("  The value of the current expression is: {0}", result);
                        break;
                    default:
                        break;
                }
                Console.WriteLine();
            } while (option != 4);
            */
        }
    }
}
