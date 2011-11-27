using System;
using System.Linq;
using System.Collections.Generic;

using EVEMon.Common.Data;
using EVEMon.Common.Attributes;
using EVEMon.Common.SettingsObjects;
using EVEMon.Common.Serialization.Settings;
using EVEMon.Common.Serialization.API;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a base class for character
    /// </summary>
    [EnforceUIThreadAffinity]
    public abstract class Character : BaseCharacter
    {
        protected readonly Guid m_guid;

        protected CharacterIdentity m_identity;
        protected long m_characterID;

        // Attributes
        protected decimal m_balance;
        protected string m_name;
        protected DateTime m_birthday;
        protected string m_race;
        protected string m_bloodLine;
        protected string m_ancestry;
        protected string m_gender;
        protected string m_corporationName;
        protected int m_corporationID;
        protected string m_allianceName;
        protected int m_allianceID;
        protected string m_cloneName;
        protected int m_cloneSkillPoints;
        protected bool m_isUpdatingPortrait;

        // Attributes
        protected readonly CharacterAttribute[] m_attributes = new CharacterAttribute[5];

        // These are the new replacements for m_attributeBonuses
        protected readonly ImplantSetCollection m_implants;

        // Certificates
        protected readonly CertificateCategoryCollection m_certificateCategories;
        protected readonly CertificateClassCollection m_certificateClasses;
        protected readonly CertificateCollection m_certificates;

        // Skills
        protected readonly SkillGroupCollection m_skillGroups;
        protected readonly SkillCollection m_skills;

        // Plans
        protected readonly PlanCollection m_plans;

        // Settings
        protected CharacterUISettings m_uiSettings;

        // Skill Point Caching
        protected DateTime m_skillPointTotalUpdated = DateTime.MinValue;
        protected int m_lastSkillPointTotal;


        #region Initialization

        /// <summary>
        /// Default constructor
        /// </summary>
        /// <param name="identity">The identitiy for this character</param>
        /// <param name="guid"></param>
        protected Character(CharacterIdentity identity, Guid guid)
        {
            m_characterID = identity.CharacterID;
            m_name = identity.Name;
            m_identity = identity;
            m_guid = guid;

            m_skillGroups = new SkillGroupCollection(this);
            m_skills = new SkillCollection(this);

            m_certificateCategories = new CertificateCategoryCollection(this);
            m_certificateClasses = new CertificateClassCollection(this);
            m_certificates = new CertificateCollection(this);
            m_implants = new ImplantSetCollection(this);
            m_plans = new PlanCollection(this);

            for (int i = 0; i < m_attributes.Length; i++)
            {
                m_attributes[i] = new CharacterAttribute(this, (EveAttribute)i);
            }

            m_uiSettings = new CharacterUISettings();
        }

        #endregion


        #region Bio

        /// <summary>
        /// Gets a global identifier for this character
        /// </summary>
        public Guid Guid
        {
            get { return m_guid; }
        }

        /// <summary>
        /// Gets the identity for this character
        /// </summary>
        public CharacterIdentity Identity
        {
            get { return m_identity; }
        }

        /// <summary>
        /// Gets or sets true if the character is monitored and displayed on the main window.
        /// </summary>
        public bool Monitored
        {
            get 
            {
                return EveClient.MonitoredCharacters.Contains(this);
            }
            set
            {
                EveClient.MonitoredCharacters.OnCharacterMonitoringChanged(this, value);
            }
        }

        /// <summary>
        /// Gets the ID for this character
        /// </summary>
        public long CharacterID
        {
            get { return m_identity.CharacterID; }
        }

        /// <summary>
        /// Gets or sets the source's name. By default, it's the character's name but it may be overriden to help distinct tabs on the main window.
        /// </summary>
        public string Name
        {
            get { return m_name; }
            set
            {
                if (m_name != value)
                {
                    m_name = value;
                    EveClient.OnCharacterChanged(this);
                }
            }
        }

        /// <summary>
        /// Gets an adorned name, with (file), (url) or (cached) labels.
        /// </summary>
        public virtual string AdornedName
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets the character's birthday.
        /// </summary>
        public DateTime Birthday
        {
            get { return m_birthday; }
        }

        /// <summary>
        /// Gets the character's race
        /// </summary>
        public string Race
        {
            get { return m_race; }
        }

        /// <summary>
        /// Gets the character's bloodline
        /// </summary>
        public string Bloodline
        {
            get { return m_bloodLine; }
        }

        /// <summary>
        /// Gets the character's ancestry.
        /// </summary>
        public string Ancestry
        {
            get { return m_ancestry; }
        }

        /// <summary>
        /// Gets the character's gender
        /// </summary>
        public string Gender
        {
            get { return m_gender; }
        }

        /// <summary>
        /// Gets the name of the character's corporation
        /// </summary>
        public string CorporationName
        {
            get { return m_corporationName; }
        }

        /// <summary>
        /// Gets the id of the character's corporation
        /// </summary>
        public int CorporationID
        {
            get { return m_corporationID; }
        }

        /// <summary>
        /// Gets the name of the character's alliance
        /// </summary>
        public string AllianceName
        {
            get { return m_allianceName; }
        }

        /// <summary>
        /// Gets the id of the character's alliance
        /// </summary>
        public int AllianceID
        {
            get { return m_allianceID; }
        }

        /// <summary>
        /// Gets the name of the clone.
        /// </summary>
        /// <value>The name of the clone.</value>
        public string CloneName
        {
            get { return m_cloneName; }
        }

        /// <summary>
        /// Gets the clone's capacity
        /// </summary>
        /// <value>The clone skill points.</value>
        public int CloneSkillPoints
        {
            get { return m_cloneSkillPoints; }
        }

        /// <summary>
        /// Gets the current character's wallet balance
        /// </summary>
        public decimal Balance
        {
            get { return m_balance; }
        }

        /// <summary>
        /// Gets true whether the portrait is currently updating.
        /// </summary>
        internal bool IsUpdatingPortrait
        {
            get { return m_isUpdatingPortrait; }
            set { m_isUpdatingPortrait = value; }
        }

        #endregion


        #region Attributes

        /// <summary>
        /// Gets the base attribute value for the given attribute
        /// </summary>
        /// <param name="attribute">The attribute to retrieve</param>
        /// <returns></returns>
        protected override ICharacterAttribute GetAttribute(EveAttribute attribute)
        {
            return m_attributes[(int)attribute];
        }

        #endregion


        #region Implants

        /// <summary>
        /// Gets the implants sets of the character and its clones
        /// </summary>
        public ImplantSetCollection ImplantSets
        {
            get { return m_implants; }
        }

        /// <summary>
        /// Gets the current implants' bonuses
        /// </summary>
        public ImplantSet CurrentImplants
        {
            get { return m_implants.Current; }
        }

        #endregion


        #region Skills

        /// <summary>
        /// Gets the collection of skills
        /// </summary>
        public SkillCollection Skills
        {
            get { return m_skills; }
        }

        /// <summary>
        /// Gets the collection of skill groups
        /// </summary>
        public SkillGroupCollection SkillGroups
        {
            get { return m_skillGroups; }
        }

        /// <summary>
        /// gets the total skill points for this character
        /// </summary>
        /// <returns></returns>
        protected override int GetTotalSkillPoints()
        {
            // we only do the calculation once every second to avoid
            // excessive CPU utilization.
            if (m_skillPointTotalUpdated > DateTime.UtcNow.AddSeconds(-1))
                return m_lastSkillPointTotal;
            
            int sp = 0;
            foreach (var skill in m_skills)
            {
                sp += skill.SkillPoints;
            }
            
            m_lastSkillPointTotal = sp;
            m_skillPointTotalUpdated = DateTime.UtcNow;

            return sp; 
        }

        /// <summary>
        /// Gets the number of skills this character knows
        /// </summary>
        public int KnownSkillCount
        {
            get
            {
                int count = 0;
                foreach (var skill in m_skills)
                {
                    if (skill.IsKnown)
                        count++;
                }
                return count;
            }
        }

        /// <summary>
        /// Gets the number of skills currently known at the same level than the one specified
        /// </summary>
        /// <param name="level"></param>
        /// <returns></returns>
        public int GetSkillCountAtLevel(int level)
        {
            int count = 0;
            foreach (var skill in m_skills.Where(x => x.IsKnown))
            {
                if (skill.LastConfirmedLvl == level)
                    count++;
            }
            return count;
        }

        /// <summary>
        /// Gets the level of the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public override int GetSkillLevel(StaticSkill skill)
        {
            return m_skills[skill].Level;
        }

        /// <summary>
        /// Gets the level of the given skill
        /// </summary>
        /// <param name="skill"></param>
        /// <returns></returns>
        public override int GetSkillPoints(StaticSkill skill)
        {
            return m_skills[skill].SkillPoints;
        }

        #endregion


        #region Certificates

        /// <summary>
        /// Gets the collection of certificate categories.
        /// </summary>
        public CertificateCategoryCollection CertificateCategories
        {
            get { return m_certificateCategories; }
        }

        /// <summary>
        /// Gets the collection of certificate classes.
        /// </summary>
        public CertificateClassCollection CertificateClasses
        {
            get { return m_certificateClasses; }
        }

        /// <summary>
        /// Gets the collection of certificates.
        /// </summary>
        public CertificateCollection Certificates
        {
            get { return m_certificates; }
        }

        #endregion


        #region Plans

        /// <summary>
        /// Gets the collection of plans.
        /// </summary>
        public PlanCollection Plans
        {
            get { return m_plans; }
        }

        #endregion


        #region Training

        /// <summary>
        /// Gets true when the character is currently training, false otherwise
        /// </summary>
        public virtual bool IsTraining
        {
            get { return false; }
        }

        /// <summary>
        /// Gets the skill currently in training
        /// </summary>
        public virtual QueuedSkill CurrentlyTrainingSkill
        {
            get { return null; }
        }

        #endregion


        #region Importation / exportation

        /// <summary>
        /// Create a serializable character sheet for this character
        /// </summary>
        /// <returns></returns>
        public abstract SerializableSettingsCharacter Export();


        /// <summary>
        /// Fetches the data to the given serialization object, used by inheritors.
        /// </summary>
        /// <param name="ci"></param>
        protected void Export(SerializableSettingsCharacter serial)
        {
            serial.Guid = m_guid;
            serial.ID = m_identity.CharacterID;
            serial.Name = m_name;
            serial.Birthday = m_birthday;
            serial.Race = m_race;
            serial.Ancestry = m_ancestry;
            serial.Gender = m_gender;
            serial.BloodLine = m_bloodLine;
            serial.CorporationName = m_corporationName;
            serial.CorporationID = m_corporationID;
            serial.AllianceName = m_allianceName;
            serial.AllianceID = m_allianceID;
            serial.CloneSkillPoints = m_cloneSkillPoints;
            serial.CloneName = m_cloneName;
            serial.Balance = m_balance;

            // Attributes
            serial.Attributes.Intelligence = Intelligence.Base;
            serial.Attributes.Perception = Perception.Base;
            serial.Attributes.Willpower = Willpower.Base;
            serial.Attributes.Charisma = Charisma.Base;
            serial.Attributes.Memory = Memory.Base;

            // Implants sets
            serial.ImplantSets = ImplantSets.Export();

            // Certificates
            serial.Certificates = new List<SerializableCharacterCertificate>();
            foreach(var cert in Certificates.Where(x => x.IsGranted))
            {
                serial.Certificates.Add(new SerializableCharacterCertificate{ CertificateID = cert.ID });
            }

            // Skills
            serial.Skills = new List<SerializableCharacterSkill>();
            foreach(var skill in Skills.Where(x => x.IsKnown || x.IsOwned))
            {
                serial.Skills.Add(skill.Export());
            }
        }

        /// <summary>
        /// Imports data from the given character sheet informations
        /// </summary>
        /// <param name="serial">The serialized character sheet</param>
        internal void Import(APIResult<SerializableAPICharacterSheet> serial)
        {
            if (serial.HasError)
                return;

            Import(serial.Result);
            EveClient.OnCharacterChanged(this);
        }

        /// <summary>
        /// Imports data from the given character sheet informations
        /// </summary>
        /// <param name="serial">The serialized character sheet</param>
        protected void Import(SerializableSettingsCharacter serial)
        {
            Import((SerializableCharacterSheetBase)serial);

            // Implants
            m_implants.Import(serial.ImplantSets);
        }

        /// <summary>
        /// Imports data from the given character sheet informations
        /// </summary>
        /// <param name="serial">The serialized character sheet</param>
        protected void Import(SerializableAPICharacterSheet serial)
        {
            Import((SerializableCharacterSheetBase)serial);

            // Implants
            m_implants.Import(serial.Implants);
        }

        /// <summary>
        /// Imports data from the given character sheet informations
        /// </summary>
        /// <param name="serial">The serialized character sheet</param>
        protected void Import(SerializableCharacterSheetBase serial)
        {
            bool fromCCP = (serial is SerializableAPICharacterSheet);

            // Bio
            m_name = serial.Name;
            m_birthday = serial.Birthday;
            m_race = serial.Race;
            m_ancestry = serial.Ancestry;
            m_gender = serial.Gender;
            m_balance = serial.Balance;
            m_bloodLine = serial.BloodLine;
            m_corporationName = serial.CorporationName;
            m_corporationID = serial.CorporationID;
            m_allianceName = serial.AllianceName;
            m_allianceID = serial.AllianceID;
            m_cloneName = serial.CloneName;
            m_cloneSkillPoints = serial.CloneSkillPoints;

            // Attributes
            m_attributes[(int)EveAttribute.Intelligence].Base = serial.Attributes.Intelligence;
            m_attributes[(int)EveAttribute.Perception].Base = serial.Attributes.Perception;
            m_attributes[(int)EveAttribute.Willpower].Base = serial.Attributes.Willpower;
            m_attributes[(int)EveAttribute.Charisma].Base = serial.Attributes.Charisma;
            m_attributes[(int)EveAttribute.Memory].Base = serial.Attributes.Memory;

            // Skills : reset all > update all
            foreach (var skill in m_skills)
            {
                skill.Reset(fromCCP);
            }
            
            foreach (var serialSkill in serial.Skills.Where(x => m_skills[x.ID] != null))
            {
                // Take care of the new skills not in our datafiles yet. Update if it exists.
                m_skills[serialSkill.ID].Import(serialSkill, fromCCP);
            }

            // Certificates : reset > mark the granted ones > update the other ones
            foreach (var cert in m_certificates)
            {
                cert.Reset();
            }

            foreach (var serialCert in serial.Certificates.Where(x => m_certificates[x.CertificateID] != null))
            {
                // Take care of the new certs not in our datafiles yet. Mark as granted if it exists.
                m_certificates[serialCert.CertificateID].MarkAsGranted();
            }

            while (true)
            {
                bool updatedAnything = false;
                foreach (var cert in m_certificates)
                {
                    updatedAnything |= cert.TryUpdateCertificateStatus();
                }
                if (!updatedAnything)
                    break;
            }
        }

        /// <summary>
        /// Imports the given plans
        /// </summary>
        /// <param name="plans"></param>
        internal void ImportPlans(IEnumerable<SerializablePlan> plans)
        {
            m_plans.Import(plans);
        }

        /// <summary>
        /// Export the plans to the given list
        /// </summary>
        /// <param name="list"></param>
        internal void ExportPlans(List<SerializablePlan> list)
        {
            foreach (var plan in m_plans)
            {
                list.Add(plan.Export());
            }
        }

        #endregion


        /// <summary>
        /// Gets the UI settings for this character.
        /// </summary>
        public CharacterUISettings UISettings
        {
            get { return m_uiSettings; }
            internal set { m_uiSettings = value; }
        }

        /// <summary>
        /// Updates the character on a timer tick
        /// </summary>
        internal virtual void UpdateOnOneSecondTick()
        {

        }

        /// <summary>
        /// Clean the obsolete entries in the plan.
        /// </summary>
        internal void CleanObsoleteEntries(ObsoleteRemovalPolicy policy)
        {
            foreach (var plan in m_plans)
            {
                plan.CleanObsoleteEntries(policy);
            }
        }

        /// <summary>
        /// Gets a unique hascode for this character.
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            return m_guid.GetHashCode();
        }

        /// <summary>
        /// Gets the name fo the character.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_name;
        }
    }
}
