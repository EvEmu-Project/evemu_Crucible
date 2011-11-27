using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a region of the EVE universe.
    /// </summary>
    public sealed class Region : ReadonlyCollection<Constellation>, IComparable<Region>
    {
        private readonly long m_id;
        private readonly string m_name;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="src"></param>
        internal Region(SerializableRegion src)
            : base(src.Constellations.Length)
        {
            m_id = src.ID;
            m_name = src.Name;

            foreach (var srcConstellation in src.Constellations)
            {
                m_items.Add(new Constellation(this, srcConstellation));
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
        /// Gets the name of this object.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_name;
        }

        /// <summary>
        /// Compare two regions by their names.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public int CompareTo(Region other)
        {
            return m_name.CompareTo(other.m_name);
        }
    }
}
