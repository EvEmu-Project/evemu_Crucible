using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Data;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents an attribute for a character scratchpad
    /// </summary>
    public sealed class CharacterAttribute : ICharacterAttribute
    {
        private readonly EveAttribute m_attrib;
        private readonly Character m_character;
        private int m_base;

        /// <summary>
        /// Constructor from a character attribute.
        /// </summary>
        /// <param name="character"></param>
        /// <param name="attrib"></param>
        internal CharacterAttribute(Character character, EveAttribute attrib)
        {
            m_base = EveConstants.CharacterBaseAttributePoints;
            m_attrib = attrib;
            m_character = character;
        }

        /// <summary>
        /// Gets the base attribute.
        /// </summary>
        public int Base
        {
            get { return m_base; }
            internal set { m_base = value; }
        }

        /// <summary>
        /// Gets the bonus granted by the implant
        /// </summary>
        public int ImplantBonus
        {
            get { return m_character.CurrentImplants[m_attrib].Bonus; }
        }

        /// <summary>
        /// Gets the effective attribute value.
        /// </summary>
        public int EffectiveValue
        {
            get { return m_base + ImplantBonus; }
        }

        /// <summary>
        /// Gets a string representation with the provided format. The following parameters are accepted :
        /// <list type="bullet">
        /// <item>%n for name (lower case)</item>
        /// <item>%N for name (CamelCase)</item>
        /// <item>%B for attribute base value</item>
        /// <item>%b for base bonus</item>
        /// <item>%i for implant bonus</item>
        /// <item>%r for remapping points</item>
        /// <item>%e for effective value</item>
        /// </list>
        /// </summary>
        /// <returns>The formatted string.</returns>
        public string ToString(string format)
        {
            format = format.Replace("%n",   m_attrib.ToString().ToLower(CultureConstants.DefaultCulture));
            format = format.Replace("%N",   m_attrib.ToString());
            format = format.Replace("%B",   EveConstants.CharacterBaseAttributePoints.ToString());
            format = format.Replace("%b",   m_base.ToString());
            format = format.Replace("%i",   ImplantBonus.ToString());
            format = format.Replace("%r",   (m_base - EveConstants.CharacterBaseAttributePoints).ToString());
            format = format.Replace("%e",   EffectiveValue.ToString("0"));
            return format;
        }

        /// <summary>
        /// Gets a string representation with the following format : "<c>Intelligence : 15</c>"
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
 	         return String.Format ("{0} : {1}", m_attrib, EffectiveValue);
        }
    }
}
