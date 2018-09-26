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
    class ExpTreeDivNode : ExpTreeOpNode
    {
        public ExpTreeDivNode()
        {
            opSymbol = "/";
        }

        public override double Evaluate()
        {
            return left.Evaluate() / right.Evaluate();
        }
    }
}
