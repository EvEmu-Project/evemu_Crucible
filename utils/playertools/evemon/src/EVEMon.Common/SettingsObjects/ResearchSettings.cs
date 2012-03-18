using System.Linq;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Settings for Research
    /// </summary>
    public sealed class ResearchSettings
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="ResearchSettings"/> class.
        /// </summary>
        public ResearchSettings()
        {
            // Add default columns
            var defaultColumns = new ResearchColumn[]
            {
                ResearchColumn.Agent,
                ResearchColumn.Field,
                ResearchColumn.CurrentRP,
                ResearchColumn.PointsPerDay,
                ResearchColumn.Station
            };

            Columns = EnumExtensions.GetValues<ResearchColumn>().Where(x => x != ResearchColumn.None).Select(x =>
                new ResearchColumnSettings { Column = x, Visible = defaultColumns.Contains(x), Width = -1 }).ToArray();
        }

        [XmlArray("columns")]
        [XmlArrayItem("column")]
        public ResearchColumnSettings[] Columns
        {
            get;
            set;
        }

        public ResearchSettings Clone()
        {
            ResearchSettings clone = (ResearchSettings)MemberwiseClone();
            clone.Columns = Columns.Select(x => x.Clone()).ToArray();
            return clone;
        }
    }
}
