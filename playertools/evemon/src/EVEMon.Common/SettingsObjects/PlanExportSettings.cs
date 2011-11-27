using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Represents the set of options for plan exporting
    /// </summary>
    public class PlanExportSettings
    {
        [XmlAttribute]
        public bool IncludeHeader
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryNumber
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryTrainingTimes
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryStartDate
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryFinishDate
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryNotes
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool EntryCost
        {
            get;
            set;
        }
	
        [XmlAttribute]
        public bool FooterCount
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool FooterTotalTime
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool FooterDate
        {
            get;
            set;
        }

        [XmlAttribute]
        public bool FooterCost
        {
            get;
            set;
        }

        /// <summary>
        /// Output markup type.
        /// </summary>
        public MarkupType Markup
        {
            get;
            set;
        }

        /// <summary>
        /// If <code>true</code>, known skills are filtered out.
        /// </summary>
        public bool ShoppingList
        {
            get;
            set;
        }

        /// <summary>
        /// Cloning
        /// </summary>
        /// <returns></returns>
        public PlanExportSettings Clone()
        {
            return (PlanExportSettings)MemberwiseClone();
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public enum MarkupType
    {
        Undefined,
        None,
        Forum,
        Html
    }

}
