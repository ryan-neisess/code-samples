/* WinForms Spreadsheet
 * Ryan Neisess, WSU
 * CptS 321, Spring 2018, Dr. Aaron Crandall
 */

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Linq;

using CptS321;

namespace Spreadsheet
{
    public partial class FormSpreadsheet : Form
    {
        public FormSpreadsheet()
        {
            InitializeComponent();

            dataGridViewSheet1.ColumnHeadersDefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;

            // Create columns A - Z
            char letterTemp = 'A';
            string letter = "";
            for(int j = 0; j < 26; j++)
            {
                // Modifying a string creates a new one each time, so just as effective to 
                //   create a new one with specified information each time
                letter = new string(letterTemp, 1);
                dataGridViewSheet1.Columns.Add(letter, letter);
                letterTemp++;
            }

            // Create rows 1 - 50
            dataGridViewSheet1.Rows.Add(50);
            dataGridViewSheet1.RowHeadersWidthSizeMode = 
                DataGridViewRowHeadersWidthSizeMode.AutoSizeToAllHeaders;

            int i = 1;
            foreach (DataGridViewRow currRow in dataGridViewSheet1.Rows)
            {
                currRow.HeaderCell.Value = Convert.ToString(i++);
            }

            sheets = new CptS321.Spreadsheet(50, 26);

            sheets.CellPropertyChanged += new EventHandler(UpdateCell);
        }

        private CptS321.Spreadsheet sheets;

        // 
        private void UpdateCell(object sender, EventArgs args)
        {
            var ssCell = (Cell)sender;
            dataGridViewSheet1.Rows[ssCell.RowIndex].Cells[ssCell.ColumnIndex].Value = ssCell.Val;
        }
        
        // Updated demo functionality
        private void demoButton_MouseClick(object sender, MouseEventArgs e)
        {
            sheets.GetCell("B2").Text = "13";
            sheets.GetCell("A3").Text = "=B2";
            sheets.GetCell("C3").Text = "=A3-D4";
            sheets.GetCell("D4").Text = "5";
            sheets.GetCell("A1").Text = "Cat";
            sheets.GetCell("E2").Text = "=A1";
            sheets.GetCell("F4").Text = "=kitty";

            sheets.GetCell("A6").Text = "B2's text is";
            sheets.GetCell("A7").Text = "A3's text is";
            sheets.GetCell("A8").Text = "C3's text is";
            sheets.GetCell("A9").Text = "D4's text is";
            sheets.GetCell("A10").Text = "A1's text is";
            sheets.GetCell("A11").Text = "E2's text is";
            sheets.GetCell("A12").Text = "F4's text is";

            sheets.GetCell("B6").Text = "\"13\"";
            sheets.GetCell("B7").Text = "\"=B2\"";
            sheets.GetCell("B8").Text = "\"=A3-D4\"";
            sheets.GetCell("B9").Text = "\"5\"";
            sheets.GetCell("B10").Text = "\"Cat\"";
            sheets.GetCell("B11").Text = "\"=A1\"";
            sheets.GetCell("B12").Text = "\"=kitty\"";
        }

        // Used to keep track of what cell the text in the text box belongs to
        private int srcTextCellRow = -1;
        private int srcTextCellCol = -1;

        private void dataGridViewSheet1_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            DataGridViewCell currCell = ((DataGridView)sender).CurrentCell;

            textFormulaBox.Text = (string)currCell.Value;
            srcTextCellRow = currCell.RowIndex;
            srcTextCellCol = currCell.ColumnIndex;

            textFormulaBox.Focus(); // Move cursor/focus to text box
        }

        private void textFormulaBox_KeyDown(object sender, KeyEventArgs e)
        {
            // Provides partial out-of-bounds checking
            if (e.KeyCode == Keys.Enter && srcTextCellRow > -1 && srcTextCellCol > -1)
            {
                // Set the text in the Spreadsheet cell; will cascade down through events
                sheets.GetCell(srcTextCellRow, srcTextCellCol).Text = textFormulaBox.Text;

                // Reset text formula box
                srcTextCellRow = -1;
                srcTextCellCol = -1;
                textFormulaBox.Text = "";
                dataGridViewSheet1.Focus();

                // Used to suppress bell sound; does this affect anything else?
                e.SuppressKeyPress = true;
            }
        }

        /*
        // Old implementation; advantages/drawbacks?
        private void aboutToolStripMenuItem_DropDownItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            this.UpdateStatus(e.ClickedItem);
        }

        private void UpdateStatus(ToolStripItem item)
        {
            if (item != null)
            {
                if (item.Text == "About")
                {
                    Form popup = new Form();
                    popup.Size = new Size(400, 300);
                    popup.ShowDialog();
                }
            }
        }
        */

        // Custom class created to customize and add Controls for About window/popup
        public class AboutWindow : Form
        {
            public AboutWindow()
            {
                this.Size = new Size(400, 300);
                this.StartPosition = FormStartPosition.CenterParent;

                infoBox = new TextBox();
                infoBox.BackColor = this.BackColor;
                infoBox.BorderStyle = BorderStyle.None;
                infoBox.Dock = DockStyle.Fill;
                infoBox.Font = new System.Drawing.Font("Adobe Devanagari", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
                infoBox.Multiline = true;
                infoBox.ReadOnly = true;
                infoBox.TabStop = false;
                infoBox.TextAlign = HorizontalAlignment.Center;

                // Why is \r\n needed instead of one or the other?
                StringBuilder infoBoxText = new StringBuilder("\r\n");
                infoBoxText.Append("CptS 321 Pending-Cat-Approval Spreadsheet Application\r\n")
                    .Append("-Version 5.0-\r\n")
                    .Append("\r\nSupports elementary mathematical operations\r\nand text editing.\r\n")
                    .Append("\r\nProgrammed by Ryan Neisess\r\n")
                    .Append("ryan [dot] neisess [at] wsu [dot] edu\r\n")
                    .Append("\u00A9 2018 Ryan Neisess\r\n")
                    .Append("All rights reserved.");

                infoBox.Text = infoBoxText.ToString();

                this.Controls.Add(infoBox);
            }

            private TextBox infoBox;
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Accepted Intellisense recommendation for simplification/streamlining of code
            Form popup = new AboutWindow
            {
                Text = "CptS 321 Basic Spreadsheet Application"
            };

            popup.ShowDialog();
        }

        // Loads a .xml file into the spreadsheet
        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog openSsPrompt = new OpenFileDialog();
            openSsPrompt.InitialDirectory = System.AppDomain.CurrentDomain.BaseDirectory;
            string ssFileName = "";

            if (openSsPrompt.ShowDialog() == DialogResult.OK)
            {
                ssFileName = openSsPrompt.FileName;
            }
            else // User decides to cancel loading
            {
                return;
            }
            
            XElement loadFile = XElement.Load(ssFileName);

            // loadFile becomes the XElement "Spreadsheet" (the root of the 
            //   XML tree as a result of loading to an XElement instead of 
            //   an XDocument

            // Create an enumerable collection of "Cell" elements
            IEnumerable<XElement> cellCollection =
                from storedCell in loadFile.Elements()
                select storedCell;

            // Clear the current contents of the spreadsheet
            for (int i = 0; i < sheets.RowCount; i++)
            {
                for (int j = 0; j < sheets.ColumnCount; j++)
                {
                    if (sheets.GetCell(i, j).Text != "")
                    {
                        sheets.GetCell(i, j).Text = "";
                    }
                }
            }

            // Tried finding a faster way to clear, but no success
            //sheets = new CptS321.Spreadsheet(50, 26);
            //dataGridViewSheet1 = new DataGridView();
            
            // Extract the data from each of the XElements in each "Cell" XElement and
            //   set the corresponding cell in the Spreadsheet with the info
            int cellRow = -1, cellCol = -1;
            string cellText = "";
            foreach (XElement retrievedCell in cellCollection)
            {
                cellRow = Convert.ToInt32((string)
                    (from cellInfo in retrievedCell.Descendants("Row")
                     select cellInfo).First());
                cellCol = Convert.ToInt32((string)
                    (from cellInfo in retrievedCell.Descendants("Col")
                     select cellInfo).First());
                cellText = (string)
                    (from cellInfo in retrievedCell.Descendants("Text")
                     select cellInfo).First();

                sheets.GetCell(cellRow, cellCol).Text = cellText;
            }
        }

        // Saves the spreadsheet to a .xml file
        // Creates XML trees to organize data
        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog ssSavePrompt = new SaveFileDialog();
            ssSavePrompt.InitialDirectory = System.AppDomain.CurrentDomain.BaseDirectory;
            string ssFileName = "";

            if (ssSavePrompt.ShowDialog() == DialogResult.OK)
            {
                ssFileName = ssSavePrompt.FileName;
            }
            else // User decides to cancel saving
            {
                return;
            }

            XDocument saveFile = new XDocument(
                new XElement("Spreadsheet"));

            Cell currCell = null;
            for (int i = 0; i < sheets.RowCount; i++)
            {
                for (int j = 0; j < sheets.ColumnCount; j++)
                {
                    currCell = sheets.GetCell(i, j);
                    if (currCell.Text != "")
                    {
                        saveFile.Root.Add(new XElement("Cell",
                            new XElement("Row", i),
                            new XElement("Col", j),
                            new XElement("Text", currCell.Text)
                            ));
                    }
                }
            }

            saveFile.Save(ssFileName);
        }
    }
}
