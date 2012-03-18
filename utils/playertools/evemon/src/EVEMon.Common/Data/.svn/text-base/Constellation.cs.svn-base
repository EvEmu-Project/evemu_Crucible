using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a constellation of the EVE universe.
    /// </summary>
    public sealed class Constellation : ReadonlyCollection<SolarSystem>, IComparable<Constellation>
    {
        private readonly long m_id;
        private readonly string m_name;
        private readonly Region m_owner;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="src"></param>
        public Constellation(Region owner, SerializableConstellation src)
            : base(src.Systems.Length)
        {
            m_id = src.ID;
            m_name = src.Name;
            m_owner = owner;

            foreach (var srcSystem in src.Systems)
            {
                m_items.Add(new SolarSystem(this, srcSystem));
            }
        }

        /// <summary>
        /// Gets this object's id.
        /// </summary>
        public long ID
        {
            get { return m_id; }
        }

        /// <summary>
        /// Gets this object's name.
        /// </summary>
        public string Name
        {
            get { return m_name; }
        }

        /// <summary>
        /// Gets the region where this constellation is located.
        /// </summary>
        public Region Region
        {
            get { return m_owner; }
        }

        /// <summary>
        /// Gets something like Heimatar > Constellation.
        /// </summary>
        public string FullLocation
        {
            get { return m_owner.Name + " > " + m_name; }
        }

        /// <summary>
        /// Gets the name of this object.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_name;
        }

        /// <summary>
        /// Compares this system with another one.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public int CompareTo(Constellation other)
        {
            if (this.Region != other.Region) return this.Region.CompareTo(other.Region);
            return m_name.CompareTo(other.m_name);
        }
    }
}
