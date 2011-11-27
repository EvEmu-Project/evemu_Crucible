using System;
using System.Drawing.Imaging;
using System.IO;
using System.IO.Compression;
using System.Text;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Controls;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Controls
{
    /// <summary>
    /// Saves a couple of repetitive tasks.
    /// </summary>
    public static class UIHelper
    {
        /// <summary>
        /// Displays the plan exportation window and then exports it.
        /// </summary>
        /// <param name="plan"></param>
        public static void ExportPlan(Plan plan)
        {
            var character = (Character)plan.Character;

            // Assemble an initial filename and remove prohibited characters
            string planSaveName = character.Name + " - " + plan.Name;
            char[] invalidFileChars = Path.GetInvalidFileNameChars();
            int fileInd = planSaveName.IndexOfAny(invalidFileChars);
            while (fileInd != -1)
            {
                planSaveName = planSaveName.Replace(planSaveName[fileInd], '-');
                fileInd = planSaveName.IndexOfAny(invalidFileChars);
            }

            // Prompt the user to pick a file name
            SaveFileDialog sfdSave = new SaveFileDialog();
            sfdSave.FileName = planSaveName;
            sfdSave.Title = "Save to File";
            sfdSave.Filter = "EVEMon Plan Format (*.emp)|*.emp|XML  Format (*.xml)|*.xml|Text Format (*.txt)|*.txt";
            sfdSave.FilterIndex = (int)PlanFormat.Emp;

            DialogResult dr = sfdSave.ShowDialog();
            if (dr == DialogResult.Cancel) return;


            // Serialize
            try
            {
                var format = (PlanFormat)sfdSave.FilterIndex;
                
                string content;
                switch (format)
                {
                    case PlanFormat.Emp:
                    case PlanFormat.Xml:
                        content = PlanExporter.ExportAsXML(plan);
                        break;
                    case PlanFormat.Text:
                        // Prompts the user and returns if he canceled
                        var settings = PromptUserForPlanExportSettings(plan);
                        if (settings == null)
                            return;

                        content = PlanExporter.ExportAsText(plan, settings);                        
                        break;
                    default:
                        throw new NotImplementedException();
                }

                // Moves to the final file
                FileHelper.OverwriteOrWarnTheUser(sfdSave.FileName, fs =>
                {
                    Stream s = fs;
                    // Emp is actually compressed text
                    if (format == PlanFormat.Emp)
                        s = new GZipStream(fs, CompressionMode.Compress);

                    using (s)
                    using (var writer = new StreamWriter(s, Encoding.UTF8))
                    {
                        writer.Write(content);
                        writer.Flush();
                        s.Flush();
                        fs.Flush();
                    }
                    return true;
                });
            }
            catch (IOException err)
            {
                ExceptionHandler.LogException(err, true);
                MessageBox.Show("There was an error writing out the file:\n\n" + err.Message,
                    "Save Failed", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        /// <summary>
        /// Prompt the user to select plan exportation settings.
        /// </summary>
        /// <returns></returns>
        public static PlanExportSettings PromptUserForPlanExportSettings(Plan plan)
        {
            PlanExportSettings settings = Settings.Exportation.PlanToText.Clone();
            using (CopySaveOptionsWindow f = new CopySaveOptionsWindow(settings, plan, false))
            {
                if (settings.Markup == MarkupType.Undefined)
                {
                    settings.Markup = MarkupType.None;
                }

                f.ShowDialog();
                if (f.DialogResult == DialogResult.Cancel) return null;

                // Save the new settings
                if (f.SetAsDefault)
                {
                    Settings.Exportation.PlanToText = settings;
                    Settings.Save();
                }

                return settings;
            }
        }

        /// <summary>
        /// Displays the character exportation window and then exports it as it would be after the plan finish.
        /// </summary>
        /// <param name="character"></param>
        public static void ExportAfterPlanCharacter(Character character, Plan plan)
        {
            // Open the dialog box
            using (var characterSaveDialog = new SaveFileDialog())
            {
                characterSaveDialog.Title = "Save After Plan Character Info";
                characterSaveDialog.Filter = "Text Format|*.txt|CHR Format (EFT)|*.chr|HTML Format|*.html|XML Format (EVEMon)|*.xml";
                characterSaveDialog.FileName = String.Format(CultureConstants.DefaultCulture, " {0} (after plan {1})", character.Name, plan.Name);
                characterSaveDialog.FilterIndex = (int)CharacterSaveFormat.EVEMonXML;

                var result = characterSaveDialog.ShowDialog();
                if (result == DialogResult.Cancel)
                    return;
                
                // Serialize
                try
                {
                    // Save character to string with the chosen format
                    CharacterSaveFormat format = (CharacterSaveFormat)characterSaveDialog.FilterIndex;
                    var content = CharacterExporter.Export(format, character, plan);
                    // Save character with the chosen format to our file
                    FileHelper.OverwriteOrWarnTheUser(characterSaveDialog.FileName, fs =>
                    {
                        using (var sw = new StreamWriter(fs, Encoding.UTF8))
                        {
                            sw.Write(content);
                            sw.Flush();
                            sw.Close();
                        }
                        return true;
                    });
                }
                // Handle exception
                catch (IOException ex)
                {
                    ExceptionHandler.LogException(ex, true);
                    MessageBox.Show("A problem occurred during exportation. The operation has not been completed.");
                }
            }
        }

        /// <summary>
        /// Displays the character exportation window and then exports it.
        /// </summary>
        /// <param name="character"></param>
        public static void ExportCharacter(Character character)
        {
            // Open the dialog box
            using(var characterSaveDialog = new SaveFileDialog())
            {
                characterSaveDialog.Title = "Save Character Info";
                characterSaveDialog.Filter = "Text Format|*.txt|CHR Format (EFT)|*.chr|HTML Format|*.html|XML Format (EVEMon)|*.xml|XML Format (CCP API)|*.xml|PNG Image|*.png";
                characterSaveDialog.FileName = character.Name;
                characterSaveDialog.FilterIndex = (int)CharacterSaveFormat.CCPXML;

                var result = characterSaveDialog.ShowDialog();
                if (result == DialogResult.Cancel) return;

                // Serialize
                try
                {
                    CharacterSaveFormat format = (CharacterSaveFormat)characterSaveDialog.FilterIndex;
                    // Save character with the chosen format to our file
                    FileHelper.OverwriteOrWarnTheUser(characterSaveDialog.FileName, fs =>
                    {
                        if (format == CharacterSaveFormat.PNG)
                        {
                            var monitor = Program.MainWindow.GetCurrentMonitor();
                            var bmp = monitor.GetCharacterScreenshot();
                            bmp.Save(fs, ImageFormat.Png);
                            return true;
                        }

                        var content = CharacterExporter.Export(format, character, null);
                        if ((format == CharacterSaveFormat.CCPXML) && string.IsNullOrEmpty(content))
                        {
                            MessageBox.Show("This character has never been downloaded from CCP, cannot find it in the XML cache.", "Cannot export the character", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                            return false;
                        }

                        using (var sw = new StreamWriter(fs, Encoding.UTF8))
                        {
                            sw.Write(content);
                            sw.Flush();
                            sw.Close();
                        }
                        return true;
                    });
                }
                // Handle exception
                catch (IOException exc)
                {
                    ExceptionHandler.LogException(exc, true);
                    MessageBox.Show("A problem occurred during exportation. The operation has not been completed.");
                }
            }
        }

        /// <summary>
        /// Adds the plans as tool strip items to the list.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="list"></param>
        /// <param name="initialize"></param>
        public static void AddTo(this PlanCollection plans, ToolStripItemCollection list, Action<ToolStripMenuItem, Plan> initialize)
        {
            //Scroll through plans
            foreach (var plan in plans)
            {
                ToolStripMenuItem planItem = new ToolStripMenuItem(plan.Name);
                initialize(planItem, plan);
                list.Add(planItem);
            }
        }
    }
}
