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
    public abstract class Cell : INotifyPropertyChanged
    {
        // Constructor to get called when a derived class is instantiated
        protected Cell(int row, int col)
        {
            text = "";
            val = "";
            RowIndex = row;
            ColumnIndex = col;
        }

        protected string text;
        protected string val;

        public string Text
        {
            get => text;
            set
            {
                if (value != text)
                {
                    text = value;
                    OnPropertyChanged("Text"); // Is this a good way to do this?
                }
            }
        }

        public string Val
        {
            get => val;
        }

        public int RowIndex { get; }
        public int ColumnIndex { get; }

        // With all the dependencies between the two classes, should this be public or protected? Friends?
        // List of references to cells?
        public LinkedList<Cell> cellRefList = new LinkedList<Cell>();

        //public delegate void EventHandler(object sender, EventArgs args);

        public event PropertyChangedEventHandler PropertyChanged;

        // Sends out the property changed event
        protected void OnPropertyChanged(string changed)
        {
            //PropertyChangedEventHandler handler = PropertyChanged;
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(changed));
            }
        }

        public void ReEvaluate()
        {
            // Calls the method that sends out the event
            OnPropertyChanged("Val");
        }
    }
}
