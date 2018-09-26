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
    public abstract class ExpTreeOpNode : ExpTreeNode
    {
        //public abstract double Evaluate();

        public ref ExpTreeNode GetLeft() => ref left;
        public ref ExpTreeNode GetRight() => ref right;

        protected string opSymbol = "?";
        protected ExpTreeNode left = null;
        protected ExpTreeNode right = null;
    }
}
