using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    /// <summary>
    /// Stores all the data regarding geography.
    /// </summary>
    public static class StaticGeography
    {
        private static readonly Dictionary<long, Region> s_regionsByID = new Dictionary<long, Region>();
        private static readonly Dictionary<long, Constellation> s_constellationsByID = new Dictionary<long, Constellation>();
        private static readonly Dictionary<long, SolarSystem> s_solarSystemsByID = new Dictionary<long, SolarSystem>();
        private static readonly Dictionary<long, Station> s_stationsByID = new Dictionary<long, Station>();
        private static bool m_initialized = false;

        /// <summary>
        /// Gets an enumeration of all the systems in the universe.
        /// </summary>
        public static IEnumerable<SolarSystem> AllSystems
        {
            get
            {
                EnsureInitialized();
                foreach (var system in s_solarSystemsByID.Values)
                {
                    yield return system;
                }
            }
        }

        /// <summary>
        /// Gets the system with the provided ID.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static SolarSystem GetSystem(long id)
        {
            EnsureInitialized();
            SolarSystem result = null;
            s_solarSystemsByID.TryGetValue(id, out result);
            return result;
        }

        /// <summary>
        /// Gets an enumeration of all the stations in the universe.
        /// </summary>
        public static IEnumerable<Station> AllStations
        {
            get
            {
                EnsureInitialized();
                foreach (var station in s_stationsByID.Values)
                {
                    yield return station;
                }
            }
        }

        /// <summary>
        /// Gets the station with the provided ID.
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        public static Station GetStation(long id)
        {
            EnsureInitialized();
            Station result = null;
            s_stationsByID.TryGetValue(id, out result);
            return result;
        }

        /// <summary>
        /// Ensures the datafile has been intialized.
        /// </summary>
        public static void EnsureInitialized()
        {
            if (m_initialized)
                return;
            var datafile = Util.DeserializeDatafile<GeoDatafile>(DatafileConstants.GeographyDatafile);

            // Generate the nodes
            foreach (var srcRegion in datafile.Regions)
            {
                var region = new Region(srcRegion);
                s_regionsByID[srcRegion.ID] = region;

                // Store the children into their dictionaries
                foreach (var constellation in region)
                {
                    s_constellationsByID[constellation.ID] = constellation;

                    foreach (var solarSystem in constellation)
                    {
                        s_solarSystemsByID[solarSystem.ID] = solarSystem;

                        foreach (var station in solarSystem)
                        {
                            s_stationsByID[station.ID] = station;
                        }
                    }
                }
            }

            // Connects the systems
            foreach (var srcJump in datafile.Jumps)
            {
                var a = s_solarSystemsByID[srcJump.FirstSystemID];
                var b = s_solarSystemsByID[srcJump.SecondSystemID];
                a.AddNeighbor(b);
                b.AddNeighbor(a);
            }
            foreach (var system in s_solarSystemsByID.Values)
            {
                system.TrimNeighbors();
            }

            // Mark as initialized
            m_initialized = true;

        }
    }
}
