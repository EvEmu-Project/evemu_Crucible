using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.BlankCharacter
{
    public partial class BlankCharacterControl : UserControl
    {
        private readonly Dictionary<int, int> s_allRaceSkills = new Dictionary<int, int>()
            { { 3426, 3 }, { 3413, 3 }, { 3416, 2 }, { 3300, 2 }, { 3386, 2 }, { 3392, 2 }, { 3449, 3 }, { 3402, 3 }, { 3327, 3 } };

        private readonly Dictionary<int, int> s_amarrRaceSkills = new Dictionary<int, int>() { { 3303, 3 }, { 3331, 2 } };
        private readonly Dictionary<int, int> s_caldariRaceSkills = new Dictionary<int, int>() { { 3301, 3 }, { 3330, 2 } };
        private readonly Dictionary<int, int> s_gallenteRaceSkills = new Dictionary<int, int>() { { 3301, 3 }, { 3328, 2 } };
        private readonly Dictionary<int, int> s_minmatarRaceSkills = new Dictionary<int, int>() { { 3302, 3 }, { 3329, 2 } };

        private Race m_race;
        private Bloodline m_bloodline;
        private Ancestry m_ancestry;
        private Gender m_gender;

        private Font m_AmarrFont;
        private Font m_CaldariFont;
        private Font m_GallenteFont;
        private Font m_MinmatarFont;

        /// <summary>
        /// Initializes a new instance of the <see cref="BlankCharacterControl"/> class.
        /// </summary>
        public BlankCharacterControl()
        {
            InitializeComponent();
        }


        /// <summary>
        /// Handles the Load event of the BlankCharacterControl.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void BlankCharacterControl_Load(object sender, EventArgs e)
        {
            ForeColor = SystemColors.GrayText;

            m_AmarrFont = FontFactory.GetFont("Niagara Solid", 10f, FontStyle.Regular);
            m_CaldariFont = FontFactory.GetFont("Impact", 8.25f, FontStyle.Regular);
            m_GallenteFont = FontFactory.GetFont("Arial Rounded MT Bold", 8.25f, FontStyle.Regular);
            m_MinmatarFont = FontFactory.GetFont("Moolboran", 9f, FontStyle.Regular);
            
            m_race = Race.Amarr;

            UpdateBloodlineControl();
        }


        #region Update Methods

        /// <summary>
        /// Updates the bloodline controls.
        /// </summary>
        private void UpdateBloodlineControl()
        {
            int index = 0;

            switch (m_race)
            {
                default:
                case Race.Amarr:
                    lblAncestry1.Font = lblAncestry2.Font = lblAncestry3.Font = m_AmarrFont;
                    break;

                case Race.Caldari:
                    index = 3;
                    lblAncestry1.Font = lblAncestry2.Font = lblAncestry3.Font = m_CaldariFont;
                    break;

                case Race.Gallente:
                    index = 6;
                    lblAncestry1.Font = lblAncestry2.Font = lblAncestry3.Font = m_GallenteFont;
                    break;

                case Race.Minmatar:
                    index = 9;
                    lblAncestry1.Font = lblAncestry2.Font = lblAncestry3.Font = m_MinmatarFont;
                    break;
            }

            pbBloodline1.Image = ilBloodline.Images[index];
            pbBloodline2.Image = ilBloodline.Images[index + 1];
            pbBloodline3.Image = ilBloodline.Images[index + 2];
            pbBloodline1.Tag = (Bloodline)Enum.ToObject(typeof(Bloodline), index);
            pbBloodline2.Tag = (Bloodline)Enum.ToObject(typeof(Bloodline), index + 1);
            pbBloodline3.Tag = (Bloodline)Enum.ToObject(typeof(Bloodline), index + 2);

            if (rbBloodline1.Checked)
                m_bloodline = (Bloodline)pbBloodline1.Tag;
            if (rbBloodline2.Checked)
                m_bloodline = (Bloodline)pbBloodline2.Tag;
            if (rbBloodline3.Checked)
                m_bloodline = (Bloodline)pbBloodline3.Tag;

            UpdateAncestryControl();
        }

        /// <summary>
        /// Updates the ancestry controls.
        /// </summary>
        private void UpdateAncestryControl()
        {
            int index = 0;

            switch (m_bloodline)
            {
                default:
                case Bloodline.Amarr:
                    break;
                case Bloodline.Ni_Kunni:
                    index = 3;
                    break;
                case Bloodline.Khanid:
                    index = 6;
                    break;
                case Bloodline.Deteis:
                    index = 9;
                    break;
                case Bloodline.Civire:
                    index = 12;
                    break;
                case Bloodline.Achura:
                    index = 15;
                    break;
                case Bloodline.Gallente:
                    index = 18;
                    break;
                case Bloodline.Intaki:
                    index = 21;
                    break;
                case Bloodline.Jin_Mei:
                    index = 24;
                    break;
                case Bloodline.Sebiestor:
                    index = 27;
                    break;
                case Bloodline.Brutor:
                    index = 30;
                    break;
                case Bloodline.Vherokior:
                    index = 33;
                    break;
            }

            pbAncestry1.Image = ilAncestry.Images[index];
            pbAncestry2.Image = ilAncestry.Images[index + 1];
            pbAncestry3.Image = ilAncestry.Images[index + 2];
            lblAncestry1.Text = ((Ancestry)Enum.ToObject(typeof(Ancestry), index)).ToString().ToUpper().Replace("_", " ");
            lblAncestry2.Text = ((Ancestry)Enum.ToObject(typeof(Ancestry), index + 1)).ToString().ToUpper().Replace("_", " ");
            lblAncestry3.Text = ((Ancestry)Enum.ToObject(typeof(Ancestry), index + 2)).ToString().ToUpper().Replace("_", " ");
            lblAncestry1.Tag = (Ancestry)Enum.ToObject(typeof(Ancestry), index);
            lblAncestry2.Tag = (Ancestry)Enum.ToObject(typeof(Ancestry), index + 1);
            lblAncestry3.Tag = (Ancestry)Enum.ToObject(typeof(Ancestry), index + 2);

            if (rbAncestry1.Checked)
                m_ancestry = (Ancestry)lblAncestry1.Tag;
            if (rbAncestry2.Checked)
                m_ancestry = (Ancestry)lblAncestry2.Tag;
            if (rbAncestry3.Checked)
                m_ancestry = (Ancestry)lblAncestry3.Tag;
        }
        
        #endregion


        #region Event Handlers

        /// <summary>
        /// Handles the Click event of the rbAmarr control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbAmarr_Click(object sender, EventArgs e)
        {
            m_race = Race.Amarr;
            UpdateBloodlineControl();
        }

        /// <summary>
        /// Handles the Click event of the rbCaldari control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbCaldari_Click(object sender, EventArgs e)
        {
            m_race = Race.Caldari;
            UpdateBloodlineControl();
        }

        /// <summary>
        /// Handles the Click event of the rbGallente control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbGallente_Click(object sender, EventArgs e)
        {
            m_race = Race.Gallente;
            UpdateBloodlineControl();
        }

        /// <summary>
        /// Handles the Click event of the rbMinmatar control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbMinmatar_Click(object sender, EventArgs e)
        {
            m_race = Race.Minmatar;
            UpdateBloodlineControl();
        }

        /// <summary>
        /// Handles the Click event of the rbBloodline1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbBloodline1_Click(object sender, EventArgs e)
        {
            m_bloodline = (Bloodline)pbBloodline1.Tag;

            UpdateAncestryControl();
        }

        /// <summary>
        /// Handles the Click event of the rbBloodline2 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbBloodline2_Click(object sender, EventArgs e)
        {
            m_bloodline = (Bloodline)pbBloodline2.Tag;
            UpdateAncestryControl();
        }

        /// <summary>
        /// Handles the Click event of the rbBloodline3 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbBloodline3_Click(object sender, EventArgs e)
        {
            m_bloodline = (Bloodline)pbBloodline3.Tag;

            UpdateAncestryControl();
        }

        /// <summary>
        /// Handles the Click event of the rbAncestry1 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbAncestry1_Click(object sender, EventArgs e)
        {
            m_ancestry = (Ancestry)lblAncestry1.Tag;
        }

        /// <summary>
        /// Handles the Click event of the rbAncestry2 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbAncestry2_Click(object sender, EventArgs e)
        {
            m_ancestry = (Ancestry)lblAncestry2.Tag;
        }

        /// <summary>
        /// Handles the Click event of the rbAncestry3 control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbAncestry3_Click(object sender, EventArgs e)
        {
            m_ancestry = (Ancestry)lblAncestry3.Tag;
        }

        /// <summary>
        /// Handles the Click event of the rbFemale control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbFemale_Click(object sender, EventArgs e)
        {
            m_gender = Gender.Female;
        }

        /// <summary>
        /// Handles the Click event of the rbMale control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void rbMale_Click(object sender, EventArgs e)
        {
            m_gender = Gender.Male;
        }

        #endregion


        #region Creation

        internal SerializableCCPCharacter CreateCharacter()
        {
            var serial = new SerializableCCPCharacter()
            {
                ID = 9999999,
                Name = tbCharacterName.Text,
                Birthday = DateTime.UtcNow,
                Race = m_race.ToString(),
                BloodLine = m_bloodline.ToString().Replace("_", "-"),
                Ancestry = m_ancestry.ToString().Replace("_", " "),
                Gender = m_gender.ToString(),
                CorporationName = "Blank Character's Corp",
                CorporationID = 9999999,
                CloneName = "Clone Grade Alpha",
                CloneSkillPoints = 900000,
                Balance = 0,

                Attributes = new SerializableCharacterAttributes()
                {
                    Intelligence = EveConstants.CharacterBaseAttributePoints + 3,
                    Memory = EveConstants.CharacterBaseAttributePoints + 3,
                    Perception = EveConstants.CharacterBaseAttributePoints + 3,
                    Willpower = EveConstants.CharacterBaseAttributePoints + 3,
                    Charisma = EveConstants.CharacterBaseAttributePoints + 2
                },

                ImplantSets = new SerializableImplantSetCollection()
                {
                    API = new SerializableSettingsImplantSet() { Name = "Implants from API" },
                    OldAPI = new SerializableSettingsImplantSet() { Name = "Previous implants from the API" },
                },

                Skills = GetSkillsForRace(),

                Certificates = new List<SerializableCharacterCertificate>(),
            };

            return serial;
        }

        #endregion

        
        #region Helper Methods

        /// <summary>
        /// Gets the skills for each race.
        /// </summary>
        /// <returns></returns>
        private List<SerializableCharacterSkill> GetSkillsForRace()
        {
            var skills = new List<SerializableCharacterSkill>();
            var startingSkills = new Dictionary<int, int>();

            switch (m_race)
            {
                case Race.Amarr:
                    startingSkills = s_allRaceSkills.Concat(s_amarrRaceSkills).ToDictionary(x => x.Key, x => x.Value);
                    break;
                case Race.Caldari:
                    startingSkills = s_allRaceSkills.Concat(s_caldariRaceSkills).ToDictionary(x => x.Key, x => x.Value);
                    break;
                case Race.Gallente:
                    startingSkills = s_allRaceSkills.Concat(s_gallenteRaceSkills).ToDictionary(x => x.Key, x => x.Value);
                    break;
                case Race.Minmatar:
                    startingSkills = s_allRaceSkills.Concat(s_minmatarRaceSkills).ToDictionary(x => x.Key, x => x.Value);
                    break;
            }

            foreach (var raceSkill in startingSkills)
            {
                var staticSkill = StaticSkills.GetSkillById(raceSkill.Key);
                if (staticSkill == null)
                    continue;

                var skill = new SerializableCharacterSkill()
                {
                    ID = raceSkill.Key,
                    Level = raceSkill.Value,
                    Name = StaticSkills.GetSkillById(raceSkill.Key).Name,
                    Skillpoints = StaticSkills.GetSkillById(raceSkill.Key).GetPointsRequiredForLevel(raceSkill.Value),
                    IsKnown = true,
                    OwnsBook = false,
                };

                skills.Add(skill);
            }
            
            return skills;
        }
 
	    #endregion

    }
}
