using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Controls
{
    /// <summary>
    /// This form allow the user to selected and order the columns he wants to use for plans.
    /// </summary>
    public abstract partial class ColumnSelectWindow : EVEMonForm
    {
        private readonly List<IColumnSettings> m_columns = new List<IColumnSettings>();

        /// <summary>
        /// Default constructor.
        /// </summary>
        public ColumnSelectWindow(IEnumerable<IColumnSettings> columns)
        {
            InitializeComponent();
            clbColumns.ItemCheck += new ItemCheckEventHandler(clbColumns_ItemCheck);

            // Fill the columns list
            m_columns.AddRange(columns);
        }

        /// <summary>
        /// Gets the columns settings
        /// </summary>
        public IEnumerable<IColumnSettings> Columns
        {
            get
            {
                foreach (var column in m_columns)
                {
                    yield return column;
                }
            }
        }

        /// <summary>
        /// On load, rebuild the window.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            UpdateContent();
        }

        /// <summary>
        /// Updates the content.
        /// </summary>
        private void UpdateContent()
        {
            clbColumns.Items.Clear();
            foreach (var key in GetAllKeys())
            {
                var column = m_columns.First(x => x.Key == key);
                clbColumns.Items.Add(GetHeader(key), column.Visible);
            }
        }

        /// <summary>
        /// When a checkbox state changed, we update the controls list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void clbColumns_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            bool isChecked = (e.NewValue == CheckState.Checked);

            // Gets the key of the modified column
            var header = (string)clbColumns.Items[e.Index];
            var key = GetAllKeys().First(x => GetHeader(x) == header);

            // Gets the column for this key
            var column = m_columns.First(x => x.Key == key);

            // Add or remove from the list
            if (column.Visible != isChecked)
            {
                column.Visible = isChecked;
                if (isChecked)
                {
                    m_columns.Remove(column);
                    m_columns.Add(column);
                }
            }
        }

        /// <summary>
        /// On cancel, nothing special.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        /// <summary>
        /// On OK, we update the settings.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnOk_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        /// <summary>
        /// On reset button... we reset (how surprising).
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void btnReset_Click(object sender, EventArgs e)
        {
            m_columns.Clear();
            m_columns.AddRange(GetDefaultColumns());
            UpdateContent();
        }

        protected abstract string GetHeader(int key);
        protected abstract IEnumerable<int> GetAllKeys();
        protected abstract IEnumerable<IColumnSettings> GetDefaultColumns();
    }
}