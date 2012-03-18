using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;
using System.Globalization;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.SettingsUI
{
    /// <summary>
    /// Configuration editor for the tray icon popup window
    /// </summary>
    public partial class TrayPopupConfigForm : EVEMonForm
    {
        private TrayPopupSettings m_settings = null;
        private string[] m_characterGrouping = { "None", "Training / Not Training", "Not Training / Training", "Account" };
        private string[] m_sortOrder = { "Training completion, earliest at bottom",
                                         "Training completion, earliest at top",
                                         "Alphabetical, first at top",
                                         "Alphabetical, first at bottom"};
        private string[] m_portraitSize;

        public TrayPopupConfigForm()
        {
            InitializeComponent();
            m_portraitSize = Enum.GetValues(typeof(PortraitSizes)).Cast<PortraitSizes>().Select(x =>
                {
                    // Transforms x64 to 64 by 64
                    var size = x.ToString().Substring(1);
                    return size + " by " + size;
                }).ToArray();
        }

        public TrayPopupConfigForm(TrayPopupSettings settings) 
            : this()
        {
            m_settings = settings;
            cbGroupBy.Items.AddRange(m_characterGrouping);
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            if (m_settings != null)
            {
                DisplayConfig(m_settings);
            }
        }

        private void DisplayConfig(TrayPopupSettings config)
        {
            cbHideNotTraining.Checked = !config.ShowCharNotTraining;
            cbGroupBy.SelectedIndex = (int)config.GroupBy;
            UpdateDisplayOrders();
            if (cbHideNotTraining.Checked)
            {
                cbDisplayOrder1.SelectedItem = m_sortOrder[(int)config.PrimarySortOrder];
            }
            else
            {
                if (config.GroupBy == TrayPopupGrouping.None)
                {
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)config.PrimarySortOrder];
                }
                else if (config.GroupBy == TrayPopupGrouping.Account)
                {
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)config.PrimarySortOrder];
                }
                else
                {
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)config.PrimarySortOrder];
                    cbDisplayOrder2.SelectedItem = m_sortOrder[(int)config.SecondarySortOrder];
                }
            }
            cbShowSkill.Checked = config.ShowSkillInTraining;
            cbShowTimeToCompletion.Checked = config.ShowRemainingTime;
            cbShowCompletionTime.Checked = config.ShowCompletionTime;
            cbShowSkillQueueTrainingTime.Checked = config.ShowSkillQueueTrainingTime;
            cbHighLightConflicts.Checked = config.HighlightConflicts;
            cbShowWallet.Checked = config.ShowWallet;
            cbShowPortrait.Checked = config.ShowPortrait;
            cbPortraitSize.Items.AddRange(m_portraitSize);
            cbPortraitSize.SelectedIndex = (int)config.PortraitSize;
            cbShowWarning.Checked = config.ShowWarning;
            cbShowServerStatus.Checked = config.ShowServerStatus;
            cbShowEveTime.Checked = config.ShowEveTime;
			cbIndentGroupedAccounts.Checked = config.IndentGroupedAccounts;
            cbUseIncreasedContrast.Checked = config.UseIncreasedContrast;
            UpdateEnables();
        }

        private void ApplyToConfig()
        {
            m_settings.ShowCharNotTraining = !cbHideNotTraining.Checked;
            m_settings.GroupBy = (TrayPopupGrouping)cbGroupBy.SelectedIndex;
            m_settings.PrimarySortOrder = GetSortOrder(cbDisplayOrder1.SelectedItem as string);
            m_settings.SecondarySortOrder = GetSortOrder(cbDisplayOrder2.SelectedItem as string);
            m_settings.ShowSkillInTraining = cbShowSkill.Checked;
            m_settings.ShowRemainingTime = cbShowTimeToCompletion.Checked;
            m_settings.ShowCompletionTime = cbShowCompletionTime.Checked;
            m_settings.ShowSkillQueueTrainingTime = cbShowSkillQueueTrainingTime.Checked;
            m_settings.HighlightConflicts = cbHighLightConflicts.Checked;
            m_settings.ShowWallet = cbShowWallet.Checked;
            m_settings.ShowPortrait = cbShowPortrait.Checked;
            m_settings.PortraitSize = (PortraitSizes)cbPortraitSize.SelectedIndex;
            m_settings.ShowWarning = cbShowWarning.Checked;
            m_settings.ShowServerStatus = cbShowServerStatus.Checked;
            m_settings.ShowEveTime = cbShowEveTime.Checked;
			m_settings.IndentGroupedAccounts = cbIndentGroupedAccounts.Checked;
            m_settings.UseIncreasedContrast = cbUseIncreasedContrast.Checked;
        }

        private TrayPopupSort GetSortOrder(string selectedSortOrder)
        {
            var sortOrder = TrayPopupSort.TrainingCompletionTimeDESC;
            for (int i = 0; i < m_sortOrder.Length; i++)
            {
                if (selectedSortOrder == m_sortOrder[i])
                    sortOrder = (TrayPopupSort)i;
            }
            return sortOrder;
        }

        private void UpdateEnables()
        {
            cbGroupBy.Enabled = !cbHideNotTraining.Checked;
            cbPortraitSize.Enabled = cbShowPortrait.Checked;
        }

        private void UpdateDisplayOrders()
        {
            if (cbHideNotTraining.Checked)
            {
                lblDisplayOrder1.Text = "Characters in training:";
                cbDisplayOrder1.Items.Clear();
                cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC]);
                cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeASC]);
                cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameASC]);
                cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameDESC]);
                cbDisplayOrder1.SelectedItem = m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC];
                lblDisplayOrder2.Visible = false;
                cbDisplayOrder2.Visible = false;
            }
            else
            {
                TrayPopupGrouping groupBy = (TrayPopupGrouping)cbGroupBy.SelectedIndex;
                if (groupBy == TrayPopupGrouping.None)
                {
                    lblDisplayOrder1.Text = "All characters:";
                    cbDisplayOrder1.Items.Clear();
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameASC]);
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameDESC]);
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)TrayPopupSort.NameASC];
                    lblDisplayOrder2.Visible = false;
                    cbDisplayOrder2.Visible = false;
                }
                else if (groupBy == TrayPopupGrouping.Account)
                {
                    lblDisplayOrder1.Text = "Accounts:";
                    cbDisplayOrder1.Items.Clear();
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC]);
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeASC]);
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC];
                    lblDisplayOrder2.Visible = false;
                    cbDisplayOrder2.Visible = false;
                }
                else
                {
                    lblDisplayOrder1.Text = "Characters in training:";
                    cbDisplayOrder1.Items.Clear();
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC]);
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeASC]);
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameASC]);
                    cbDisplayOrder1.Items.Add(m_sortOrder[(int)TrayPopupSort.NameDESC]);
                    cbDisplayOrder1.SelectedItem = m_sortOrder[(int)TrayPopupSort.TrainingCompletionTimeDESC];
                    lblDisplayOrder2.Text = "Characters not training:";
                    cbDisplayOrder2.Items.Clear();
                    cbDisplayOrder2.Items.Add(m_sortOrder[(int)TrayPopupSort.NameASC]);
                    cbDisplayOrder2.Items.Add(m_sortOrder[(int)TrayPopupSort.NameDESC]);
                    cbDisplayOrder2.SelectedItem = m_sortOrder[(int)TrayPopupSort.NameASC];
                    lblDisplayOrder2.Visible = true;
                    cbDisplayOrder2.Visible = true;
                }
            }
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            ApplyToConfig();
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void cbHideNotTraining_CheckedChanged(object sender, EventArgs e)
        {
            UpdateEnables();
            UpdateDisplayOrders();
        }

        private void cbGroupBy_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateDisplayOrders();
        }

        private void cbShowPortrait_CheckedChanged(object sender, EventArgs e)
        {
            UpdateEnables();
        }

        private void btnUseDefaults_Click(object sender, EventArgs e)
        {
            DisplayConfig(new TrayPopupSettings());
        }

    }
}
