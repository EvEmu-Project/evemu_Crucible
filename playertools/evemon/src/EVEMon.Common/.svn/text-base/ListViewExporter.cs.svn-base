using System;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace EVEMon.Common
{
    public static class ListViewExporter
    {
        private static SaveFileDialog s_saveFileDialog = new SaveFileDialog();

        /// <summary>
        /// This method takes a list view and returns a multi-line string that represents the listview as a CSV (comma-delimited) file.  The major
        /// difference is that the list view assumes to contain units, so if the values in each column contain two values seperated by a space
        /// and the second value is the same in every column (1 and beyond), the unit is seperated out and placed in column "2".  This allows the
        /// values to be imported into the spreadsheet software as a number, instead of a string enabling numerical analysis of the export.
        /// </summary>
        /// <param name="aListViewToExport">as noted.</param>
        /// <returns>A CSV text file.</returns>
        public static void CreateCSV(ListView aListViewToExport)
        {
            s_saveFileDialog.Filter = "Comma Delimited Files (*.csv)|*.csv";
            if (s_saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                StringBuilder sb = new StringBuilder();

                // Export the column headers
                bool aFirst = true;
                for (int I = 0; I < aListViewToExport.Columns.Count; I++)
                {
                    ColumnHeader myColumn = aListViewToExport.Columns[I];
                    sb.Append(MakeCSVString(aFirst, myColumn.Text));
                    aFirst = false;

                    if (I == 0)
                        sb.Append(MakeCSVString(aFirst, "Unit"));
                }
                sb.AppendLine();

                for (int line = 0; line < aListViewToExport.Items.Count; line++)
                {
                    // Test to see if each value of the line column 1 to N contains one space and the unit is the same throughout
                    bool hasCommonUnit = true;
                    string unit = String.Empty;
                    for (int column = 1; (column < aListViewToExport.Items[line].SubItems.Count) && hasCommonUnit; column++)
                    {
                        string[] elements = aListViewToExport.Items[line].SubItems[column].Text.Split(" ".ToCharArray());
                        hasCommonUnit = elements.Length == 2;
                        if (hasCommonUnit)
                            if (unit == String.Empty)
                                unit = elements[1];
                            else
                                hasCommonUnit = (elements[1] == unit);
                    }

                    // Export the lines
                    aFirst = true;
                    int maxElements = aListViewToExport.Columns.Count;
                    if (aListViewToExport.Items[line].SubItems.Count < maxElements)
                        maxElements = aListViewToExport.Items[line].SubItems.Count;

                    for (int subitem = 0; (subitem < maxElements); subitem++)
                    {
                        try
                        {
                            string[] elements = aListViewToExport.Items[line].SubItems[subitem].Text.Split(" ".ToCharArray());
                            if (hasCommonUnit && elements.Length == 2 && elements[1] == unit)
                            {
                                sb.Append(MakeCSVNumber(aFirst, Convert.ToDouble(elements[0].Replace(",", String.Empty)).ToString()));
                            }
                            else
                            {
                                sb.Append(MakeCSVNumber(aFirst, Convert.ToDouble(aListViewToExport.Items[line].SubItems[subitem].Text).ToString()));
                            }
                        }
                        catch
                        {
                            sb.Append(MakeCSVString(aFirst, aListViewToExport.Items[line].SubItems[subitem].Text));
                        }
                        aFirst = false;
                        if (subitem == 0)
                        {
                            sb.Append(MakeCSVString(aFirst, unit));
                        }
                    }
                    sb.AppendLine();
                }

                File.WriteAllText(s_saveFileDialog.FileName, sb.ToString());
            }
        }

        /// <summary>
        /// Makes the CSV string.
        /// </summary>
        /// <param name="aIgnoreComma">if set to <c>true</c> [a ignore comma].</param>
        /// <param name="aWhat">A what.</param>
        /// <returns></returns>
        private static string MakeCSVString(bool aIgnoreComma, string aWhat)
        {
            return MakeCSVNumber(aIgnoreComma, @"""" + aWhat.Replace("\"", "\"\"") + @"""");
        }

        /// <summary>
        /// Makes the CSV number.
        /// </summary>
        /// <param name="aIgnoreComma">if set to <c>true</c> [a ignore comma].</param>
        /// <param name="aWhat">A what.</param>
        /// <returns></returns>
        private static string MakeCSVNumber(bool aIgnoreComma, string aWhat)
        {
            return ((aIgnoreComma) ? String.Empty : ",") + aWhat;

        }
    }
}
