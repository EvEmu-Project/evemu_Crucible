using System.Xml.Serialization;

namespace EVEMon.Common.Serialization.Datafiles
{
    /// <summary>
    /// Represents our blueprints datafile
    /// </summary>
    [XmlRoot("blueprints")]
    public sealed class BlueprintsDatafile
    {
        [XmlElement("group")]
        public SerializableBlueprintGroup[] Groups
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a blueprint group in our datafile
    /// </summary>
    public sealed class SerializableBlueprintGroup
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlElement("blueprint")]
        public SerializableBlueprint[] Blueprints
        {
            get;
            set;
        }

        [XmlArray("subGroups")]
        [XmlArrayItem("subGroup")]
        public SerializableBlueprintGroup[] SubGroups
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a blueprint in our datafile
    /// </summary>
    public sealed class SerializableBlueprint
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("icon")]
        public string Icon
        {
            get;
            set;
        }

        [XmlAttribute("metaGroup")]
        public ItemMetaGroup MetaGroup
        {
            get;
            set;
        }

        [XmlAttribute("productTypeID")]
        public short ProduceItemID
        {
            get;
            set;
        }

        [XmlAttribute("productionTime")]
        public int ProductionTime
        {
            get;
            set;
        }

        [XmlAttribute("techLevel")]
        public short TechLevel
        {
            get;
            set;
        }

        [XmlAttribute("researchProductivityTime")]
        public int ResearchProductivityTime
        {
            get;
            set;
        }

        [XmlAttribute("researchMaterialTime")]
        public int ResearchMaterialTime
        {
            get;
            set;
        }

        [XmlAttribute("researchCopyTime")]
        public int ResearchCopyTime
        {
            get;
            set;
        }

        [XmlAttribute("researchTechTime")]
        public int ResearchTechTime
        {
            get;
            set;
        }

        [XmlAttribute("productivityModifier")]
        public int ProductivityModifier
        {
            get;
            set;
        }

        [XmlAttribute("wasteFactor")]
        public short WasteFactor
        {
            get;
            set;
        }

        [XmlAttribute("maxProductionLimit")]
        public int MaxProductionLimit
        {
            get;
            set;
        }

        [XmlElement("inventTypeID")]
        public int[] InventionTypeID
        {
            get;
            set;
        }

        [XmlElement("s")]
        public SerializablePrereqSkill[] PrereqSkill
        {
            get;
            set;
        }

        [XmlElement("m")]
        public SerializableRequiredMaterial[] ReqMaterial
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a prerequisite skill for a blueprint.
    /// </summary>
    public sealed class SerializablePrereqSkill
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("lv")]
        public int Level
        {
            get;
            set;
        }

        [XmlAttribute("activity")]
        public int Activity
        {
            get;
            set;
        }
    }

    /// <summary>
    /// Represents a prerequisite material for a blueprint.
    /// </summary>
    public sealed class SerializableRequiredMaterial
    {
        [XmlAttribute("id")]
        public int ID
        {
            get;
            set;
        }

        [XmlAttribute("quantity")]
        public int Quantity
        {
            get;
            set;
        }

        [XmlAttribute("damagePerJob")]
        public double DamagePerJob
        {
            get;
            set;
        }

        [XmlAttribute("activityId")]
        public int Activity
        {
            get;
            set;
        }

        [XmlAttribute("wasted")]
        public int WasteAffected
        {
            get;
            set;
        }
    }
}
