using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    #region EvePropertyValue
    /// <summary>
    /// Describes a property of a ship/item (e.g. CPU size)
    /// </summary>
    public struct EvePropertyValue
    {
        private EveProperty m_property;
        private string m_value;

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="src"></param>
        internal EvePropertyValue(SerializablePropertyValue src)
        {
            m_property = StaticProperties.GetPropertyById(src.ID);
            m_value = String.Intern(src.Value);
        }

        /// <summary>
        /// Gets the property
        /// </summary>
        public EveProperty Property
        {
            get { return m_property; }
        }

        /// <summary>
        /// Gets the property value
        /// </summary>
        public string Value
        {
            get { return m_value; }
        }

        /// <summary>
        /// Gets the integer value.
        /// </summary>
        public int IValue
        {
            get { return Int32.Parse(m_value); }
        }

        /// <summary>
        /// Gets the floating point value.
        /// </summary>
        public float FValue
        {
            get { return Single.Parse(m_value); }
        }

        /// <summary>
        /// Gets a string representation of this prerequisite
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_property.Name;
        }
    }
    #endregion


    #region PropertiesCollection
    /// <summary>
    /// Represents the collection of prerequisites an object must satisfy to be used
    /// </summary>
    public sealed class PropertiesCollection : ReadonlyCollection<EvePropertyValue>
    {
        /// <summary>
        /// Deserialization consructor
        /// </summary>
        /// <param name="src"></param>
        internal PropertiesCollection(SerializablePropertyValue[] src)
            : base(src == null ? 0 : src.Length)
        {
            if (src == null) return;

            m_items.Capacity = src.Length;
            foreach (var srcProp in src)
            {
                var prop = new EvePropertyValue(srcProp);
                if(prop.Property != null) m_items.Add(prop);
            }
            m_items.Trim();
        }

        /// <summary>
        /// Gets a property from its name. If not found, return null.
        /// </summary>
        /// <param name="property">The property we're searching for.</param>
        /// <returns>The wanted property when found; null otherwise.</returns>
        public Nullable<EvePropertyValue> this[EveProperty property]
        {
            get
            {
                foreach (var prop in m_items)
                {
                    if (prop.Property == null)
                        return null;

                    if (prop.Property == property) return prop;
                }
                return null;
            }
        }

        /// <summary>
        /// Gets a property from its name. If not found, return null.
        /// </summary>
        /// <param name="property">The property we're searching for.</param>
        /// <returns>The wanted property when found; null otherwise.</returns>
        public Nullable<EvePropertyValue> this[int id]
        {
            get
            {
                foreach (var prop in m_items)
                {
                    if (prop.Property == null)
                        return null;

                    if (prop.Property.ID == id) return prop;
                }
                return null;
            }
        }
    }
    #endregion
}
