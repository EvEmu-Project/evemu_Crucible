using System;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    public sealed class EvePropertyCategory : ReadonlyCollection<EveProperty>
    {
        private readonly string m_name;
        private readonly string m_description;
        private readonly string m_displayName;

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="serial"></param>
        internal EvePropertyCategory(SerializablePropertyCategory serial)
            : base(serial.Properties.Length)
        {
            m_name = serial.Name;
            m_description = serial.Description;

            foreach (var serialProp in serial.Properties)
            {
                m_items.Add(new EveProperty(this, serialProp));
            }

            // Sets the display name
            switch (m_name)
            {
                default:
                    m_displayName = m_name;
                    break;
                case "NULL":
                    m_displayName = "System";
                    break;
            }
        }

        /// <summary>
        /// Gets this category's name
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets this category's description
        /// </summary>
        public string Description
        {
            get { return m_description; }
        }

        /// <summary>
        /// Gets the category's display name.
        /// </summary>
        public string DisplayName
        {
            get { return m_displayName; }
        }
    }
}
