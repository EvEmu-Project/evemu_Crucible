using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;
using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Represents a solar system of the EVE universe.
    /// </summary>
    public sealed class SolarSystem : ReadonlyCollection<Station>, IComparable<SolarSystem>
    {
        // Do not set this as readonly !
        private FastList<SolarSystem> m_jumps;

        private readonly long m_id;
        private readonly string m_name;
        private readonly float m_securityLevel;
        private readonly Constellation m_owner;
        private readonly int m_x;
        private readonly int m_y;
        private readonly int m_z;

        /// <summary>
        /// Constructor.
        /// </summary>
        public SolarSystem(Constellation owner, SerializableSolarSystem src)
            : base(src.Stations == null ? 0 : src.Stations.Length)
        {
            m_id = src.ID;
            m_owner = owner;
            m_name = src.Name;
            m_securityLevel = src.SecurityLevel;
            m_jumps = new FastList<SolarSystem>(0);

            m_x = src.X;
            m_y = src.Y;
            m_z = src.Z;

            if (src.Stations == null)
                return;
            foreach (var srcStation in src.Stations)
            {
                m_items.Add(new Station(this, srcStation));
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
        /// Gets the real security level, between -1.0 and +1.0
        /// </summary>
        public float SecurityLevel
        {
            get { return m_securityLevel; }
        }

        /// <summary>
        /// Gets the constellation this solar system is located.
        /// </summary>
        public Constellation Constellation
        {
            get { return m_owner; }
        }

        /// <summary>
        /// Gets something like Heimatar > Constellation > Pator.
        /// </summary>
        public string FullLocation
        {
            get { return m_owner.FullLocation + " > " + m_name; }
        }

        /// <summary>
        /// Gets the square distance with the given system.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public int GetSquareDistanceWith(SolarSystem other)
        {
            var dx = m_x - other.m_x;
            var dy = m_y - other.m_y;
            var dz = m_z - other.m_z;

            return dx * dx + dy * dy + dz * dz;
        }

        /// <summary>
        /// Gets the solar systems within the given range.
        /// </summary>
        /// <param name="maxInclusiveNumberOfJumps">The maximum, inclusive, number of jumps from this system.</param>
        /// <returns></returns>
        public List<SolarSystemRange> GetSystemsWithinRange(int maxInclusiveNumberOfJumps)
        {
            return SolarSystemRange.GetSystemRangesFrom(this, maxInclusiveNumberOfJumps);
        }

        /// <summary>
        /// Find the guessed shortest path using a A* (heuristic) algorithm.
        /// </summary>
        /// <param name="target">The target system.</param>
        /// <param name="minSecurityLevel">The mininmum, inclusive, real security level. Systems have levels between -1 and +1.</param>
        /// <returns>The list of systems, beginning with this one and ending with the provided target.</returns>
        public List<SolarSystem> GetFastestPathTo(SolarSystem target, float minSecurityLevel)
        {
            return PathFinder.FindBestPath(this, target, minSecurityLevel);
        }

        /// <summary>
        /// Gets the systems which have a jumpgate connection with his one.
        /// </summary>
        public IEnumerable<SolarSystem> Neighbors
        {
            get
            {
                foreach (var system in m_jumps)
                {
                    yield return system;
                }
            }
        }

        /// <summary>
        /// Adds a neighbor with a jumpgate connection to this system.
        /// </summary>
        /// <param name="system"></param>
        internal void AddNeighbor(SolarSystem system)
        {
            m_jumps.Add(system);
        }

        /// <summary>
        /// Trims the neighbors list
        /// </summary>
        internal void TrimNeighbors()
        {
            m_jumps.Trim();
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
        /// Gets the ID of the object.
        /// </summary>
        /// <returns></returns>
        public override int GetHashCode()
        {
            return (int)m_id;
        }

        /// <summary>
        /// Compares this system with another one.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        public int CompareTo(SolarSystem other)
        {
            if (this.Constellation != other.Constellation)
                return this.Constellation.CompareTo(other.Constellation);

            return m_name.CompareTo(other.m_name);
        }
    }
}
