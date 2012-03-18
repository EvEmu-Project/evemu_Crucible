using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvBlueprintTypes :IHasID
    {
        [XmlElement("blueprintTypeID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("parentBlueprintTypeID")]
        public Nullable<short> ParentID;

        [XmlElement("productTypeID")]
        public short ProductTypeID;

        [XmlElement("productionTime")]
        public int ProductionTime;

        [XmlElement("techLevel")]
        public short TechLevel;

        [XmlElement("researchProductivityTime")]
        public int ResearchProductivityTime;

        [XmlElement("researchMaterialTime")]
        public int ResearchMaterialTime;

        [XmlElement("researchCopyTime")]
        public int ResearchCopyTime;

        [XmlElement("researchTechTime")]
        public int ResearchTechTime;

        [XmlElement("productivityModifier")]
        public int ProductivityModifier;

        [XmlElement("wasteFactor")]
        public short WasteFactor;

        [XmlElement("maxProductionLimit")]
        public int MaxProductionLimit;
    }
}
