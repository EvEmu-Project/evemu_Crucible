using System;
using System.Globalization;
using System.IO;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common.Serialization.Exportation;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.SettingsObjects;

namespace EVEMon.Common
{
    public enum PlanFormat
    {
        None = 0,
        Emp = 1,
        Xml = 2,
        Text = 3
    }
    
    public static class PlanExporter
    {
        public delegate void ExportPlanEntryActions(StringBuilder builder, PlanEntry entry, PlanExportSettings settings);

        /// <summary>
        /// Exports the plan under a text format.
        /// </summary>
        /// <param name="planToExport"></param>
        /// <param name="settings"></param>
        /// <returns></returns>
        public static string ExportAsText(Plan planToExport, PlanExportSettings settings)
        {
            return ExportAsText(planToExport, settings, null);
        }

        /// <summary>
        /// Exports the plan under a text format.
        /// </summary>
        /// <param name="planToExport"></param>
        /// <param name="settings"></param>
        /// <param name="exportActions"></param>
        /// <returns></returns>
        public static string ExportAsText(Plan planToExport, PlanExportSettings settings, ExportPlanEntryActions exportActions)
        {
            var plan = new PlanScratchpad(planToExport.Character, planToExport);
            plan.Sort(planToExport.SortingPreferences);
            plan.UpdateStatistics();

            var builder = new StringBuilder();
            var timeFormat = DescriptiveTextOptions.FullText | DescriptiveTextOptions.IncludeCommas | DescriptiveTextOptions.SpaceText;
            var character = (Character)plan.Character;

            // Initialize constants
            string lineFeed = Environment.NewLine;
            string boldStart = String.Empty;
            string boldEnd = String.Empty;

            switch (settings.Markup)
            {
                default:
                    break;
                case MarkupType.Forum:
                    boldStart = "[b]";
                    boldEnd = "[/b]";
                    break;
                case MarkupType.Html:
                    lineFeed = String.Format(CultureInfo.InvariantCulture, "<br />{0}", Environment.NewLine);
                    boldStart = "<b>";
                    boldEnd = "</b>";
                    break;
            }

            // Header
            if (settings.IncludeHeader)
            {
                builder.Append(boldStart);
                if (settings.ShoppingList)
                {
                    builder.AppendFormat(CultureConstants.DefaultCulture, "Shopping list for {0}", character.Name);
                }
                else
                {
                    builder.AppendFormat(CultureConstants.DefaultCulture, "Skill plan for {0}", character.Name);
                }
                builder.Append(boldEnd);
                builder.Append(lineFeed);
                builder.Append(lineFeed);
            }

            // Scroll through entries
            int index = 0;
            DateTime endTime = DateTime.Now;
            foreach (PlanEntry entry in plan)
            {
                // Remapping point
                if (!settings.ShoppingList && entry.Remapping != null)
                {
                    builder.AppendFormat(CultureConstants.DefaultCulture, "***{0}***", entry.Remapping);
                    builder.Append(lineFeed);
                }

                // Skip is we're only build a shopping list
                bool shoppingListCandidate = !(entry.CharacterSkill.IsKnown || entry.Level != 1 || entry.CharacterSkill.IsOwned);
                if (settings.ShoppingList && !shoppingListCandidate) 
                    continue;

                // Entry's index
                index++;
                if (settings.EntryNumber)
                    builder.AppendFormat(CultureConstants.DefaultCulture, "{0}. ", index);

                // Name
                builder.Append(boldStart);

                if (settings.Markup.Equals(MarkupType.Html))
                {
                    if (!settings.ShoppingList)
                    {
                        builder.AppendFormat(CultureConstants.DefaultCulture, "<a href=\"\" onclick=\"CCPEVE.showInfo({0})\">", entry.Skill.ID);
                    }
                    else
                    {
                        builder.AppendFormat(CultureConstants.DefaultCulture, "<a href=\"\" onclick=\"CCPEVE.showMarketDetails({0})\">", entry.Skill.ID);
                    }
                }
                builder.Append(entry.Skill.Name);

                if (settings.Markup == MarkupType.Html)
                    builder.Append("</a>");

                if (!settings.ShoppingList)
                    builder.AppendFormat(CultureConstants.DefaultCulture, " {0}", Skill.GetRomanForInt(entry.Level));

                builder.Append(boldEnd);

                // Training time
                if (settings.EntryTrainingTimes || settings.EntryStartDate || settings.EntryFinishDate || (settings.EntryCost && shoppingListCandidate))
                {
                    builder.Append(" (");
                    bool needComma = false;

                    // Training time
                    if (settings.EntryTrainingTimes)
                    {
                        needComma = true;
                        builder.Append(entry.TrainingTime.ToDescriptiveText(timeFormat));
                    }

                    // Training start date
                    if (settings.EntryStartDate)
                    {
                        if (needComma)
                            builder.Append("; ");

                        needComma = true;

                        builder.AppendFormat(CultureConstants.DefaultCulture, "Start: {0}", entry.StartTime);
                    }

                    // Training end date
                    if (settings.EntryFinishDate)
                    {
                        if (needComma)
                            builder.Append("; ");

                        needComma = true;

                        builder.AppendFormat(CultureConstants.DefaultCulture, "Finish: {0}", entry.EndTime);
                    }

                    // Skill cost
                    if (settings.EntryCost && shoppingListCandidate)
                    {
                        if (needComma)
                            builder.Append("; ");

                        needComma = true;

                        builder.AppendFormat(CultureConstants.DefaultCulture, "{0} ISK",  entry.Skill.FormattedCost);
                    }

                    builder.Append(')');
                }

                if (exportActions != null)
                    exportActions(builder, entry, settings);

                builder.Append(lineFeed);

                // End time
                endTime = entry.EndTime;
            }

            // Footer
            if (settings.FooterCount || settings.FooterTotalTime || settings.FooterDate || settings.FooterCost)
            {
                builder.AppendLine(lineFeed);
                bool needComma = false;

                // Skills count
                if (settings.FooterCount)
                {
                    builder.AppendFormat(CultureConstants.DefaultCulture, "{0}{1}{2}", boldStart, index, boldEnd);
                    builder.Append((index == 1 ? " skill" : " skills"));
                    needComma = true;
                }

                // Plan's training duration
                if (settings.FooterTotalTime)
                {
                    if (needComma)
                        builder.Append("; ");

                    needComma = true;

                    builder.AppendFormat(CultureConstants.DefaultCulture, "Total time: {0}{1}{2}",
                        boldStart, plan.GetTotalTime(null, true).ToDescriptiveText(timeFormat), boldEnd);
                }

                // End training date
                if (settings.FooterDate)
                {
                    if (needComma)
                        builder.Append("; ");

                    needComma = true;

                    builder.AppendFormat(CultureConstants.DefaultCulture, "Completion: {0}{1}{2}", boldStart, endTime, boldEnd);
                }

                // Total books cost
                if (settings.FooterCost)
                {
                    if (needComma)
                        builder.Append("; ");

                    needComma = true;

                    string formattedIsk = String.Format(CultureConstants.TidyInteger, "{0:n}", plan.NotKnownSkillBooksCost);
                    builder.AppendFormat(CultureConstants.DefaultCulture, "Cost: {0}{1}{2}", boldStart, formattedIsk, boldEnd);
                }

                // Warning about skill costs
                builder.Append(lineFeed);
                if (settings.FooterCost || settings.EntryCost)
                    builder.Append("N.B. Skill costs are based on CCP's database and are indicative only");
            }

            // Returns the text representation.
            return builder.ToString();
        }

        /// <summary>
        /// Exports the plan under a XML format.
        /// </summary>
        /// <param name="planToExport"></param>
        /// <returns></returns>
        public static string ExportAsXML(Plan plan)
        {
            // Generates a settings plan and transforms it to an output plan
            var serial = plan.Export();
            var output = new OutputPlan { Name = serial.Name, Owner = serial.Owner, Revision = Settings.Revision };
            output.Entries.AddRange(serial.Entries);

            // Serializes to XML document and gets a string representation
            var doc = Util.SerializeToXmlDocument(typeof(OutputPlan), output);
            return Util.GetXMLStringRepresentation(doc);
        }

        /// <summary>
        /// Imports a <see cref="SerializablePlan"/> from the given filename. Works with old and new formats.
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static SerializablePlan ImportFromXML(String filename)
        {
            SerializablePlan result = null;
            try
            {
                // Is the format compressed ? 
                if (filename.EndsWith(".emp"))
                {
                    string tempFile = Util.UncompressToTempFile(filename);
                    try
                    {
                        return ImportFromXML(tempFile);
                    }
                    finally
                    {
                        File.Delete(tempFile);
                    }
                }

                // Reads the revision number from the file
                int revision = Util.GetRevisionNumber(filename);

                // Old format
                if (revision == 0)
                {
                    result = Util.DeserializeXML<SerializablePlan>(filename, Util.LoadXSLT(Properties.Resources.SettingsAndPlanImport));
                }
                // New format
                else
                {
                    result = Util.DeserializeXML<OutputPlan>(filename);
                }
            }
            catch (UnauthorizedAccessException exc)
            {
                MessageBox.Show("Couldn't read the given file, access was denied. Maybe the directory was under synchronization.");
                ExceptionHandler.LogException(exc, true);
            }
            catch (InvalidDataException exc)
            {
                MessageBox.Show("The file seems to be corrupted, wrong gzip format.");
                ExceptionHandler.LogException(exc, true);
            }

            if (result == null)
            {
                MessageBox.Show("There was a problem with the format of the document.");
            }

            return result;
        }
    }
}
