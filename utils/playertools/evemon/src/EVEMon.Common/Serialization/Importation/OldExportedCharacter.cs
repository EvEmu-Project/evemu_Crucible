using System;
using System.Collections.Generic;
using System.Xml.Serialization;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common.Serialization.Importation
{
    /// <summary>
    /// Facilitates importation of file characters from versions of
    /// EVEMon prior to 1.3.0.
    /// </summary>
    /// <remarks>
    /// These changes were released early 2010, it is safe to assume
    /// that they can be removed from the project early 2012.
    /// </remarks>
    [XmlRoot("character")]
    public sealed class OldExportedCharacter
    {
        [XmlAttribute("name")]
        public string Name
        {
            get;
            set;
        }

        [XmlAttribute("characterID")]
        public int CharacterId
        {
            get;
            set;
        }

        [XmlElement("race")]
        public string Race
        {
            get;
            set;
        }

        [XmlElement("bloodLine")]
        public string BloodLine
        {
            get;
            set;
        }

        [XmlElement("gender")]
        public string Gender
        {
            get;
            set;
        }

        [XmlElement("corporationName")]
        public string CorpName
        {
            get;
            set;
        }

        [XmlElement("cloneName")]
        public string CloneName
        {
            get;
            set;
        }

        [XmlElement("cloneSkillPoints")]
        public int CloneSkillPoints
        {
            get;
            set;
        }

        [XmlElement("balance")]
        public Decimal Balance
        {
            get;
            set;
        }

        [XmlElement("attributes")]
        public OldAttributes Attributes
        {
            get;
            set;
        }

        [XmlElement("attributeEnhancers")]
        public OldExportedAttributeEnhancers AttributeEnhancers
        {
            get;
            set;
        }

        [XmlArray("skills")]
        [XmlArrayItem("skillGroup")]
        public List<OldExportedSkillGroup> SkillGroups
        {
            get;
            set;
        }

        [XmlArray("certificates")]
        public List<OldExportedCertificate> Certificates
        {
            get;
            set;
        }

        public SerializableCCPCharacter ToSerializableCCPCharacter()
        {
            var ccpCharacter = new SerializableCCPCharacter();
            ccpCharacter.Name = Name;
            ccpCharacter.ID = CharacterId;
            ccpCharacter.Race = Race;
            ccpCharacter.BloodLine = BloodLine;
            ccpCharacter.Gender = Gender;
            ccpCharacter.CorporationName = CorpName;
            ccpCharacter.CorporationID = 0;
            ccpCharacter.CloneName = CloneName;
            ccpCharacter.CloneSkillPoints = CloneSkillPoints;
            ccpCharacter.Balance = Balance;
            ccpCharacter.Attributes = Attributes.ToSerializableAttributes();
            ccpCharacter.ImplantSets.API = AttributeEnhancers.ToSerializableImplantSet();
            ccpCharacter.Skills = CreateSerializableCharacterSkillList();
            ccpCharacter.Certificates = CreateSerializableCharacterCertificateList();

            return ccpCharacter;
        }

        private List<SerializableCharacterSkill> CreateSerializableCharacterSkillList()
        {
            var ccpSkills = new List<SerializableCharacterSkill>();

            foreach (var group in SkillGroups)
            {
                foreach (var skill in group.Skills)
                {
                    var ccpSkill = new SerializableCharacterSkill();
                    ccpSkill.ID = skill.Id;
                    ccpSkill.IsKnown = true;
                    ccpSkill.Level = skill.Level;
                    ccpSkill.OwnsBook = true;
                    ccpSkill.Skillpoints = skill.SkillPoints;
                    ccpSkills.Add(ccpSkill);
                }
            }

            return ccpSkills;
        }
        
        private List<SerializableCharacterCertificate> CreateSerializableCharacterCertificateList()
        {
            var ccpCertificates = new List<SerializableCharacterCertificate>();

            foreach (var certificate in Certificates)
            {
                var ccpCertificate = new SerializableCharacterCertificate();
                ccpCertificate.CertificateID = certificate.CertificateID;
                ccpCertificates.Add(ccpCertificate);
            }

            return ccpCertificates;
        }
    }
}
