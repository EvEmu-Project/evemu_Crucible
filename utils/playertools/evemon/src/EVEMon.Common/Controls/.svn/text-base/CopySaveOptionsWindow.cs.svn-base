using System;
using System.IO;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common.Controls
{
    public partial class CopySaveOptionsWindow : EVEMonForm
    {
        public CopySaveOptionsWindow()
        {
            InitializeComponent();
        }

        public CopySaveOptionsWindow(PlanExportSettings pto, Plan p, bool isForCopy)
            : this()
        {
            m_planTextOptions = pto;
            m_plan = p;
            m_isForCopy = isForCopy;
        }

        private PlanExportSettings m_planTextOptions;
        private Plan m_plan;
        private bool m_isForCopy = false;
        private bool m_setAsDefault = false;

        private void CopySaveOptionsWindow_Load(object sender, EventArgs e)
        {
            if (m_isForCopy)
            {
                this.Text = "Copy Options";
            }
            else
            {
                this.Text = "Save Options";
            }

            cbIncludeHeader.Checked = m_planTextOptions.IncludeHeader;
            cbEntryNumber.Checked = m_planTextOptions.EntryNumber;
            cbEntryTrainingTimes.Checked = m_planTextOptions.EntryTrainingTimes;
            cbEntryCost.Checked = m_planTextOptions.EntryCost;
            cbEntryStartDate.Checked = m_planTextOptions.EntryStartDate;
            cbEntryFinishDate.Checked = m_planTextOptions.EntryFinishDate;
            cbFooterCount.Checked = m_planTextOptions.FooterCount;
            cbFooterTotalTime.Checked = m_planTextOptions.FooterTotalTime;
            cbFooterDate.Checked = m_planTextOptions.FooterDate;
            cbFooterCost.Checked = m_planTextOptions.FooterCost;
            cbShoppingList.Checked = m_planTextOptions.ShoppingList;
            cmbFormatting.SelectedIndex = m_planTextOptions.Markup == MarkupType.Forum ? 0 :
                    m_planTextOptions.Markup == MarkupType.Html ? 1 : 2;

            RecurseUnder(this);
            OptionChange();
        }

        private void RecurseUnder(Control parent)
        {
            foreach (Control c in parent.Controls)
            {
                if (c is CheckBox && c != cbRememberOptions)
                {
                    CheckBox cb = (CheckBox) c;
                    cb.CheckedChanged += new EventHandler(cb_CheckedChanged);
                }
                RecurseUnder(c);
            }
        }

        private void cb_CheckedChanged(object sender, EventArgs e)
        {
            OptionChange();
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            OptionChange();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void UpdateOptions()
        {
            m_planTextOptions.IncludeHeader = cbIncludeHeader.Checked;
            m_planTextOptions.EntryNumber = cbEntryNumber.Checked;
            m_planTextOptions.EntryTrainingTimes = cbEntryTrainingTimes.Checked;
            m_planTextOptions.EntryStartDate = cbEntryStartDate.Checked;
            m_planTextOptions.EntryFinishDate = cbEntryFinishDate.Checked;
            m_planTextOptions.EntryCost = cbEntryCost.Checked;
            m_planTextOptions.FooterCount = cbFooterCount.Checked;
            m_planTextOptions.FooterTotalTime = cbFooterTotalTime.Checked;
            m_planTextOptions.FooterCost = cbFooterCost.Checked;
            m_planTextOptions.FooterDate = cbFooterDate.Checked;
            m_planTextOptions.ShoppingList = cbShoppingList.Checked;
            m_planTextOptions.Markup = cmbFormatting.SelectedIndex == 0 ? MarkupType.Forum :
                    cmbFormatting.SelectedIndex == 1 ? MarkupType.Html : MarkupType.None;
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            UpdateOptions();

            m_setAsDefault = cbRememberOptions.Checked;
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void OptionChange()
        {
            UpdateOptions();
            tbPreview.Text = PlanExporter.ExportAsText(m_plan, m_planTextOptions);
        }

        public bool SetAsDefault
        {
            get { return m_setAsDefault; }
        }
    }
}
