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
    class ExpTreeVarNode : ExpTreeNode
    {
        public ExpTreeVarNode(string newVarName, Spreadsheet source)
        {
            varName = newVarName;
            sourceSpreadsheet = source;
        }

        public override double Evaluate()
        {
            // Will throw a System.FormatException if unable to convert cell val to 
            //   double or if unable to convert row index to int, or 
            //   System.IndexOutOfRangeException if indices are valid int but not in range
            return Convert.ToDouble(sourceSpreadsheet.GetCell(varName).Val);
        }
        private Spreadsheet sourceSpreadsheet;
        private string varName;
    }
}
