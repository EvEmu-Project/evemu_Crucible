using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Root Blueprint Browser Settings Class
    /// </summary>
    public sealed class BlueprintBrowserSettings
    {

        #region Constructors

        /// <summary>
        /// Constructor
        /// </summary>
        public BlueprintBrowserSettings()
        {
            UsabilityFilter = ObjectUsabilityFilter.All;
            MetagroupFilter = ItemMetaGroup.All;
            ActivityFilter = ObjectActivityFilter.Any;
        }

        #endregion


        #region Public Properties

        [XmlElement("textSearch")]
        public string TextSearch
        {
            get;
            set;
        }

        [XmlElement("usabilityFilter")]
        public ObjectUsabilityFilter UsabilityFilter
        {
            get;
            set;
        }

        [XmlElement("activityFilter")]
        public ObjectActivityFilter ActivityFilter
        {
            get;
            set;
        }

        [XmlElement("metaGroupFilter")]
        public ItemMetaGroup MetagroupFilter
        {
            get;
            set;
        }

        [XmlElement("productionFacilityIndex")]
        public int ProductionFacilityIndex
        {
            get;
            set;
        }

        [XmlElement("researchFacilityIndex")]
        public int ResearchFacilityIndex
        {
            get;
            set;
        }

        [XmlElement("implantSetIndex")]
        public int ImplantSetIndex
        {
            get;
            set;
        }

        #endregion


        #region Helper Methods

        internal BlueprintBrowserSettings Clone()
        {
            return (BlueprintBrowserSettings)MemberwiseClone();
        }

        #endregion

    }
}
