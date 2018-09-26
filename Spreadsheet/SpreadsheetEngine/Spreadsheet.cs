/* WinForms Spreadsheet
 * Ryan Neisess, WSU
 * CptS 321, Spring 2018, Dr. Aaron Crandall
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;

namespace CptS321
{
    public class Spreadsheet
    {
        public Spreadsheet(int numRows, int numCols)
        {
            sheet1 = new BasicCell[numRows, numCols];

            for (int i = 0; i < numRows; i++)
            {
                for (int j = 0; j < numCols; j++)
                {
                    sheet1[i, j] = new BasicCell(i, j);

                    // Allows each cell to update itself when changes occur
                    sheet1[i, j].PropertyChanged += SpreadsheetPropertyChanged;
                }
            }

            rowCount = numRows;
            columnCount = numCols;
        }

        protected class BasicCell : Cell
        {
            public BasicCell(int row, int col) : base(row, col) { }

            public void SetValue(string newValue)
            {
                if (newValue != val)
                {
                    val = newValue;
                    //OnPropertyChanged("Val"); // Is this needed?
                }
            }
        }

        private BasicCell[,] sheet1; // reference to a 2D array of BasicCells
        private int rowCount;
        private int columnCount;

        public int RowCount
        {
            get => rowCount;
            set => rowCount = value;
        }

        public int ColumnCount
        {
            get => columnCount;
            set => columnCount = value;
        }

        // Begin implementation of subscribing to events

        public event EventHandler CellPropertyChanged;

        // Invoked as a method called in response to a Cell PropertyChanged event; 
        //   re-evaluates the cell's value based on its text
        private void SpreadsheetPropertyChanged(object sender, PropertyChangedEventArgs args)
        {
            // Create a reference to the cell of interest
            var currCell = (BasicCell)sender;

            // Update the value based on the text
            currCell.SetValue(EvaluateCell(currCell.Text, currCell));

            // Update all cells that depend on currCell
            if (currCell.cellRefList.Count > 0)
            {
                // Updates all cells that depend on currCell

                // Old Implementation
                /*
                // Line below throws System.InvalidOperationException:
                //   'Collection was modified after the enumerator was instantiated.'
                foreach (Cell dependentCell in currCell.cellRefList)
                {
                    dependentCell.ReEvaluate();
                }
                */

                // New implementation
                LinkedList<Cell> cellRefListCopy = new LinkedList<Cell>(currCell.cellRefList);
                foreach (Cell dependentCell in cellRefListCopy)
                {
                    dependentCell.ReEvaluate();
                }
            }

            // Pass on the event if a change occurred
            if (CellPropertyChanged != null)
            {
                // Must be currCell and NOT the this ref
                // Does the second argument need to be e or is args fine?
                CellPropertyChanged(currCell, args);
            }
        }

        // End implementation of subscribing to events

        // Should this return a reference?
        // Returns the cell at the given indices
        // Precondition: 26 column max for current implementation (A through Z)
        public Cell GetCell(int rowIndex, int columnIndex)
        {
            return sheet1[rowIndex, columnIndex];
        }

        // Accepts an address in spreadsheet format (e.g. A12) and returns 
        //   the corresponding cell; overloaded for convenience
        // Precondition: 26 column max for current implementation (A through Z)
        public Cell GetCell(string cellAddress)
        {
            // Evaluate row and column
            char letter = '\0';
            int colIndex = 0;
            string num = "";
            int rowIndex = 0;

            letter = cellAddress[0];
            colIndex = letter - 'A';
            num = cellAddress.Substring(1);
            rowIndex = Convert.ToInt32(num) - 1;

            return sheet1[rowIndex, colIndex];
        }

        // Evaluates = formulae without functions
        private string EvaluateCell(string cellText, BasicCell currCell)
        {
            string updatedValue = cellText;

            // Should add in unsubscription; extra list per cell needed for that?

            // Short-circuit eval added for when cell is set to empty
            if (updatedValue != "" && updatedValue[0] == '=')
            {
                updatedValue = updatedValue.Substring(1); // Take off =

                // Create the expression tree
                // Also add the currCell to the cellRefList of each cell it depends 
                //   on, so if they change, they know to update this cell
                ExpTree cellExpTree = new ExpTree(updatedValue, this, currCell);

                // If there is no expression tree built (empty or non-mathematical string)
                if (cellExpTree.IsEmpty())
                {
                    return updatedValue;
                }

                /*
                if (currCell.cellRefList.Count > 0)
                {
                    // Updates all cells that depend on currCell
                    foreach (Cell dependentCell in currCell.cellRefList)
                    {
                        dependentCell.ReEvaluate();
                    }
                }
                */

                // Handle input that is invalid for expression tree
                // Referenced https://stackoverflow.com/questions/136035/catch-multiple-exceptions-at-once
                //   for good practices
                try
                {
                    updatedValue = cellExpTree.Eval().ToString();
                }
                catch (System.Exception ex)
                {
                    if (ex is System.FormatException || ex is System.IndexOutOfRangeException)
                    {
                        updatedValue = "#REF!";
                        return updatedValue;
                    }
                    throw;
                }
            }
            return updatedValue;
        }
    }
}
