using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace EVEMon.Controls
{
    /// <summary>
    /// 
    /// </summary>
    public sealed class IntuitiveDataGridView : DataGridView
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public IntuitiveDataGridView()
            : base()
        {
        }

        /// <summary>
        /// Processes F2 and Enter correctly.
        /// </summary>
        /// <param name="keyData"></param>
        /// <returns></returns>
        protected override bool ProcessDialogKey(Keys keyData)
        {
            Keys key = (keyData & Keys.KeyCode);
            if (key == Keys.Return)
            {
                if (!this.IsCurrentCellInEditMode) return this.ProcessF2Key(keyData);
                else
                {
                    DataGridViewCell currentCell = CurrentCell;
                    CurrentCell = null;
                    CurrentCell = currentCell;
                    EndEdit();
                    return true;
                }
            }
            if (key == Keys.F2)
            {
                if (!this.IsCurrentCellInEditMode) return this.ProcessF2Key(keyData);
                else
                {
                    DataGridViewCell currentCell = CurrentCell;
                    CurrentCell = null;
                    CurrentCell = currentCell;
                    EndEdit();
                    return true;
                }
            }

            return base.ProcessDialogKey(keyData);
        }

        /// <summary>
        /// Processes F2 and Enter correctly.
        /// </summary>
        /// <param name="e"></param>
        /// <returns></returns>
        protected override bool ProcessDataGridViewKey(KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Return)
            {
                if (!this.IsCurrentCellInEditMode) return this.ProcessF2Key(e.KeyData);
                //else return this.EndEdit();
            }
            if (e.KeyCode == Keys.F2)
            {
                if (!this.IsCurrentCellInEditMode) return this.ProcessF2Key(e.KeyData);
                else return this.EndEdit();
            }

            return base.ProcessDataGridViewKey(e);
        }

        /// <summary>
        /// When the user clicks the cell content, we begin editing.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnCellClick(DataGridViewCellEventArgs e)
        {
            base.OnCellClick(e);
            if (e.ColumnIndex >= 0) this.BeginEdit(true);
        }
    }
}
