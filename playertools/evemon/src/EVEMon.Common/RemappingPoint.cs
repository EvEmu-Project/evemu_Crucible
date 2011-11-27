using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a remapping operation attached to a plan entry
    /// </summary>
    public sealed class RemappingPoint
    {
        /// <summary>
        /// Describes whether it has already been computed or not
        /// </summary>
        public enum PointStatus
        {
            NotComputed,
            UpToDate
        }

        private PointStatus m_status;
        private string m_description = String.Empty;
        private int[] m_attributes = new int[5];
        private Guid m_guid = Guid.NewGuid();

        /// <summary>
        /// Default constructor.
        /// </summary>
        public RemappingPoint()
        {
        }

        /// <summary>
        /// Deserialization constructor
        /// </summary>
        /// <param name="serial"></param>
        public RemappingPoint(SerializableRemappingPoint serial)
        {
            m_attributes[(int)EveAttribute.Intelligence] = serial.Intelligence;
            m_attributes[(int)EveAttribute.Perception] = serial.Perception;
            m_attributes[(int)EveAttribute.Willpower] = serial.Willpower;
            m_attributes[(int)EveAttribute.Charisma] = serial.Charisma;
            m_attributes[(int)EveAttribute.Memory] = serial.Memory;
            m_description = serial.Description;
            m_status = serial.Status;
        }

        /// <summary>
        /// Gets a global identified of this remapping point
        /// </summary>
        public Guid Guid
        {
            get { return m_guid; }
        }

        /// <summary>
        /// Gets the new base value for the given attribute
        /// </summary>
        /// <param name="attrib"></param>
        /// <returns></returns>
        public int this[EveAttribute attrib]
        {
            get { return m_attributes[(int)attrib]; }
        }

        /// <summary>
        /// Gets the point's status (whether is has been initialized/computed or not)
        /// </summary>
        [System.Xml.Serialization.XmlIgnore]
        public PointStatus Status
        {
            get { return this.m_status; }
            internal set { this.m_status = value; }
        }

        /// <summary>
        /// Gets a short string representation of the point ("i5 p7 c8 w9 m5")
        /// </summary>
        /// <returns></returns>
        private string ToShortString()
        {
            StringBuilder builder = new StringBuilder();
            builder.Append("i").Append(this.m_attributes[(int)EveAttribute.Intelligence].ToString()).
                Append(" p").Append(this.m_attributes[(int)EveAttribute.Perception].ToString()).
                Append(" c").Append(this.m_attributes[(int)EveAttribute.Charisma].ToString()).
                Append(" w").Append(this.m_attributes[(int)EveAttribute.Willpower].ToString()).
                Append(" m").Append(this.m_attributes[(int)EveAttribute.Memory].ToString());

            return builder.ToString();
        }

        /// <summary>
        /// Gets a long string representation of this point. Two possible formats : 
        /// <list type="">
        /// <item>"Remapping (not computed, use the attributes optimizer)"</item>
        /// <item>"Remapping (active) : &lt;description&gt;</item>
        /// </list>
        /// </summary>
        /// <returns></returns>
        public string ToLongString()
        {
            switch (m_status)
            {
                case PointStatus.NotComputed:
                    return "Remapping (not computed, use the attributes optimizer)";
                case PointStatus.UpToDate:
                    return String.Format("Remapping : {0}", m_description);
                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Gets a string representation of this point. Two possible formats : 
        /// <list type="">
        /// <item>"Remapping (not computed, use the attributes optimizer)"</item>
        /// <item>"Remapping (active) : i5 p7 c8 w9 m5"</item>
        /// </list>
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            switch (m_status)
            {
                case PointStatus.NotComputed:
                    return "Remapping (not computed, use the attributes optimizer)";
                case PointStatus.UpToDate:
                    return String.Format("Remapping (active) : {0}", ToShortString());
                default:
                    throw new NotImplementedException();
            }
        }

        /// <summary>
        /// Will set the provided character scratchpad's base attributes as the target values to remap.
        /// </summary>
        /// <param name="newScratchpad">The scratchpad with the target base values to assign to this point</param>
        /// <param name="oldScratchpad">The scratchpad before we remapped</param>
        internal void SetBaseAttributes(CharacterScratchpad newScratchpad, CharacterScratchpad oldScratchpad)
        {
            // Update the status
            this.m_status = PointStatus.UpToDate;

            // Initialize the string
            StringBuilder builder = new StringBuilder();

            // Scroll through attributes
            for (int i = 0; i < 5; i++)
            {
                // Compute the new base attribute
                EveAttribute attrib = (EveAttribute)i;
                m_attributes[i] = newScratchpad[attrib].Base;

                // Update description
                builder.AppendLine().Append(GetStringForAttribute(attrib, oldScratchpad, newScratchpad));
            }

            // Return the final string
            this.m_description = builder.ToString();
        }

        /// <summary>
        /// Gets a string representation of the attribute
        /// </summary>
        /// <param name="attrib"></param>
        /// <param name="oldScratchpad"></param>
        /// <param name="newScratchpad"></param>
        /// <returns></returns>
        public static string GetStringForAttribute(EveAttribute attrib, CharacterScratchpad oldScratchpad, CharacterScratchpad newScratchpad)
        {
            int bonusDifference = newScratchpad[attrib].Base - oldScratchpad[attrib].Base;

            if (bonusDifference == 0)
            {
                return newScratchpad[attrib].ToString("%N (0) = %e = (%B + %r + %i)");
            }
            else if (bonusDifference > 0)
            {
                return newScratchpad[attrib].ToString(String.Format("%N (+{0}) = %e = (%B + %r + %i)", bonusDifference));
            }
            else
            {
                return newScratchpad[attrib].ToString(String.Format("%N ({0}) = %e = (%B + %r + %i)", bonusDifference));
            }
        }

        /// <summary>
        /// Gets a hash code from the GUID.
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            return m_guid.GetHashCode();
        }

        /// <summary>
        /// Clones the remapping point.
        /// </summary>
        /// <returns></returns>
        public RemappingPoint Clone()
        {
            RemappingPoint clone = new RemappingPoint();
            Array.Copy(m_attributes, clone.m_attributes, 5);
            clone.m_status = m_status;
            clone.m_guid = m_guid;
            return clone;
        }

        /// <summary>
        /// Creates a serialization object
        /// </summary>
        /// <returns></returns>
        internal SerializableRemappingPoint Export()
        {
            var serial = new SerializableRemappingPoint();
            serial.Intelligence = m_attributes[(int)EveAttribute.Intelligence];
            serial.Perception = m_attributes[(int)EveAttribute.Perception];
            serial.Willpower = m_attributes[(int)EveAttribute.Willpower];
            serial.Charisma = m_attributes[(int)EveAttribute.Charisma];
            serial.Memory = m_attributes[(int)EveAttribute.Memory];
            serial.Description = m_description;
            serial.Status = m_status;
            return serial;
        }
    }
}
