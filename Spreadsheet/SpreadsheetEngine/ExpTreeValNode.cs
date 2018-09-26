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
    class ExpTreeValNode : ExpTreeNode
    {
        public ExpTreeValNode(double newVal)
        {
            val = newVal;
        }

        public override double Evaluate()
        {
            return val;
        }

        private double val;
    }
}
