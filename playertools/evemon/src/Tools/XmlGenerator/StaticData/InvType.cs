using System;
using System.Xml.Serialization;

namespace EVEMon.XmlGenerator.StaticData
{
    public sealed class InvType : IHasID
    {
        [XmlElement("typeID")]
        public int ID
        {
            get;
            set;
        }

        [XmlElement("groupID")]
        public int GroupID;

        [XmlElement("iconID")]
        public Nullable<int> IconID;

        [XmlElement("typeName")]
        public string Name;

        [XmlElement("description")]
        public string Description;

        [XmlElement("mass")]
        public double Mass;

        [XmlElement("volume")]
        public double Volume;

        [XmlElement("capacity")]
        public double Capacity;

        [XmlElement("raceID")]
        public Nullable<int> RaceID;

        [XmlElement("marketGroupID")]
        public Nullable<int> MarketGroupID;

        [XmlElement("basePrice")]
        public decimal BasePrice;

        [XmlElement("published")]
        public bool Published;

        [XmlIgnore]
        public bool Generated;
    }
}
