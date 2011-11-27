using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    public sealed class EveProperty
    {
        private static EveProperty s_cpuProperty;
        private static EveProperty s_powergridProperty;

        private readonly EvePropertyCategory m_owner;

        private readonly long m_id;
        private readonly string m_icon;
        private readonly string m_unit;
        private readonly int m_unitID;
        private readonly string m_name;
        private readonly string m_description;
        private readonly string m_defaultValue;
        private readonly bool m_higherIsBetter;

        private float m_value;
        private bool m_alwaysVisibleForShips;
        private bool m_hideIfDefault;

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="owner"></param>
        /// <param name="serial"></param>
        internal EveProperty(EvePropertyCategory owner, SerializableProperty serial)
        {
            m_owner = owner;
            m_id = serial.ID;
            m_icon = serial.Icon;
            m_unit = serial.Unit;
            m_unitID = serial.UnitID;
            m_description = serial.Description;
            m_defaultValue = serial.DefaultValue;
            m_higherIsBetter = serial.HigherIsBetter;

            m_name = serial.Name;

            switch (m_id)
            {
                case DBConstants.CPUNeedPropertyID:
                    s_cpuProperty = this;
                    break;
                case DBConstants.PGNeedPropertyID:
                    s_powergridProperty = this;
                    break;
            }
        }

        /// <summary>
        /// Gets the property's ID.
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets the property name (the displayName in the CCP tables).
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets the property's description.
        /// </summary>
        public string Description
        {
            get { return m_description; }
        }

        /// <summary>
        /// Gets the string representation of the default value.
        /// </summary>
        public string DefaultValue
        {
            get { return m_defaultValue; }
        }

        /// <summary>
        /// Gets the icon for this property.
        /// </summary>
        public string Icon
        {
            get { return m_icon; }
        }

        /// <summary>
        /// Gets the unit for this property.
        /// </summary>
        public string Unit
        {
            get { return m_unit; }
        }

        /// <summary>
        /// Gets the unitID for this property.
        /// </summary>
        public int UnitID
        {
            get { return m_unitID; }
        }

        /// <summary>
        /// When true, the higher the value, the better it is.
        /// </summary>
        public bool HigherIsBetter
        {
            get { return m_higherIsBetter; }
        }

        /// <summary>
        /// When true, the property is always visible in the ships browsers, even when an object has no value for this property.
        /// </summary>
        public bool AlwaysVisibleForShips
        {
            get { return m_alwaysVisibleForShips; }
            internal set { m_alwaysVisibleForShips = value; }
        }

        /// <summary>
        /// When true, the property is hidden in the ships/items browsers if the value is the same then the default value.
        /// </summary>
        public bool HideIfDefault
        {
            get { return m_hideIfDefault; }
            internal set { m_hideIfDefault = value; }
        }

        /// <summary>
        /// Gets the CPU Output property.
        /// </summary>
        public static EveProperty CPU
        {
            get { return s_cpuProperty; }
        }

        /// <summary>
        /// Gets the Powergrid Output property.
        /// </summary>
        public static EveProperty Powergrid
        {
            get { return s_powergridProperty; }
        }

        /// <summary>
        /// Gets the label of this property's value for the given item or, when the property is absent, the default value.
        /// The returned string will include the unit.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public string GetLabelOrDefault(Item obj)
        {
            var value = obj.Properties[this];
            if (value == null)
                return Format(m_defaultValue);

            return Format(value.Value.Value);
        }

        /// <summary>
        /// Format a property value as shown in EVE
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        private string Format(string value)
        {
            if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out m_value))
            {
                try
                {
                    // Format a value of Structure Volume
                    if (m_id == 161)
                        return String.Format(CultureConstants.DefaultCulture, "{0:#,##0.0##} {1}", m_value, m_unit);

                    // Format a value of Capacitor Capacity
                    if (m_id == 482)
                        return String.Format(CultureConstants.DefaultCulture, "{0:#,##0} {1}", Math.Floor(m_value), m_unit);

                    // Format a value of Ships Warp Speed
                    if (m_id == 1281)
                        return String.Format(CultureConstants.DefaultCulture, "{0:0.0#} {1}", m_value, m_unit);

                    switch (m_unitID)
                    {
                        // Format a value of Mass
                        case 2:
                            if (m_value <= 1000)
                            {
                                return String.Format(CultureConstants.DefaultCulture, "{0:#,##0.0#} {1}", m_value, m_unit);
                            }
                            else
                            {
                                return String.Format(CultureConstants.DefaultCulture, "{0:#,##0} {1}", m_value, m_unit);
                            }

                        // Format a value of Millseconds
                        case 101:
                            return String.Format(CultureConstants.DefaultCulture, "{0:#,##0.00} {1}", m_value / 1000, m_unit);

                        // Format a value of Absolute Percentage
                        case 127:
                            return String.Format(CultureConstants.DefaultCulture, "{0} {1}", (m_value) * 100, m_unit);

                        // Format a value of Inverse Absolute Percentage
                        case 108:
                            return String.Format(CultureConstants.DefaultCulture, "{0} {1}", (1 - m_value) * 100, m_unit);

                        // Format a value of Modifier Percentage
                        case 109:
                            return String.Format(CultureConstants.DefaultCulture, "{0:0.###} {1}", (m_value - 1) * 100, m_unit);

                        // Format a value of Inverse Modifier Percentage
                        case 111:
                            return String.Format(CultureConstants.DefaultCulture, "{0:0.###} {1}", (1 - m_value) * 100, m_unit);

                        // A reference to a group, it has been pre-transformed on XmlGenerator.
                        case 115: // "groupID"
                            return value;

                        // A reference to an item or a skill.
                        case 116: // "typeID"
                            int id = Int32.Parse(value);
                            return StaticItems.GetItemByID(id).Name;

                        // Format a Sizeclass
                        case 117: // "1=small 2=medium 3=l"
                            int size = Int32.Parse(value);
                            switch (size)
                            {
                                case 1:
                                    return "Small";
                                case 2:
                                    return "Medium";
                                case 3:
                                    return "Large";
                                case 4:
                                    return "Extra Large";
                                default:
                                    return "Unknown";
                            }

                        // Format all other values (use of thousand and decimal separator)
                        default:
                            return String.Format(CultureConstants.DefaultCulture, "{0:#,##0.###} {1}", m_value, m_unit);
                    }
                }
                catch
                {
                    return "Error";
                }
            }
            return value;
        }

        /// <summary>
        /// Gets the numeric interpretation of the object's property value, or NaN if the property value is not a numeric. 
        /// When the given object has no value for this property, we use the default one.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        public float GetNumericValue(Item obj)
        {
            // Retrieve the string for the number
            string number = String.Empty;
            var value = obj.Properties[this];
            number = (value == null ? m_defaultValue : value.Value.Value);

            // Try to parse it as a float
            float result = float.NaN;
            float.TryParse(number, NumberStyles.Number, CultureInfo.InvariantCulture, out result);
            return result;
        }
    }
}
