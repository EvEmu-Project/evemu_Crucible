using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    public sealed class MonitoredCharacterSettings
    {
        /// <summary>
        /// Deserialization constructor
        /// </summary>
        public MonitoredCharacterSettings()
        {
            Settings = new CharacterUISettings();
        }

        /// <summary>
        /// Creation for new settings for this character
        /// </summary>
        public MonitoredCharacterSettings(Character character)
        {
            CharacterGuid = character.Guid;
            Settings = new CharacterUISettings();
        }

        [XmlAttribute("characterGuid")]
        public Guid CharacterGuid
        {
            get;
            set;
        }

        [XmlElement("settings")]
        public CharacterUISettings Settings
        {
            get;
            set;
        }
    }
}
