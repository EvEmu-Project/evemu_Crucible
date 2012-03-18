using System.Linq;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class EveNotificationsSettings
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="EveNotificationsSettings"/> class.
        /// </summary>
        public EveNotificationsSettings()
        {
            // Add default columns
            var defaultColumns = new EveNotificationsColumn[]
            {
                EveNotificationsColumn.SenderName,
                EveNotificationsColumn.Type,
                EveNotificationsColumn.SentDate,
            };

            Columns = EnumExtensions.GetValues<EveNotificationsColumn>().Where(x => x != EveNotificationsColumn.None).Select(x =>
                new EveNotificationsColumnSettings { Column = x, Visible = defaultColumns.Contains(x), Width = -1 }).ToArray();

            ReadingPanePosition = ReadingPanePositioning.Off;
        }

        [XmlArray("columns")]
        [XmlArrayItem("column")]
        public EveNotificationsColumnSettings[] Columns
        {
            get;
            set;
        }

        [XmlElement("readingPanePosition")]
        public ReadingPanePositioning ReadingPanePosition
        {
            get;
            set;
        }

        public EveNotificationsSettings Clone()
        {
            EveNotificationsSettings clone = (EveNotificationsSettings)MemberwiseClone();
            clone.Columns = Columns.Select(x => x.Clone()).ToArray();
            clone.ReadingPanePosition = ReadingPanePosition;
            return clone;
        }
    }
}