using System;
using System.Collections.Generic;
using System.Text;
using System.Linq;
using System.Xml.Serialization;

using EVEMon.Common.Attributes;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Settings for Industry Jobs
    /// </summary>
    public sealed class IndustryJobSettings
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="IndustryJobSettings"/> class.
        /// </summary>
        public IndustryJobSettings()
        {
            // Add default columns
            var defaultColumns = new IndustryJobColumn[]
            { 
                IndustryJobColumn.State,
                IndustryJobColumn.TTC,
                IndustryJobColumn.InstalledItem,
                IndustryJobColumn.OutputItem,
            };

            Columns = EnumExtensions.GetValues<IndustryJobColumn>().Where(x => x != IndustryJobColumn.None).Select(x =>
                new IndustryJobColumnSettings { Column = x, Visible = defaultColumns.Contains(x), Width = -1 }).ToArray();

            HideInactiveJobs = true;
        }

        [XmlArray("columns")]
        [XmlArrayItem("column")]
        public IndustryJobColumnSettings[] Columns
        {
            get;
            set;
        }

        [XmlElement("hideInactiveJobs")]
        public bool HideInactiveJobs
        {
            get;
            set;
        }

        public IndustryJobSettings Clone()
        {
            IndustryJobSettings clone = (IndustryJobSettings)MemberwiseClone();
            clone.Columns = Columns.Select(x => x.Clone()).ToArray();
            clone.HideInactiveJobs = HideInactiveJobs;
            return clone;
        }
    }
}
