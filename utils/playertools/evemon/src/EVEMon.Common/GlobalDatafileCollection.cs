using System;
using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common
{
    /// <summary>
    /// Implements a collection of datafiles
    /// </summary>
    public sealed class GlobalDatafileCollection : ReadonlyCollection<Datafile>
    {
        /// <summary>
        /// Default constructor. This class is only instantiated by EveClient
        /// </summary>
        internal GlobalDatafileCollection()
        {
        }

        /// <summary>
        /// Update the datafiles list
        /// </summary>
        public void Refresh()
        {
            try
            {
                m_items.Clear();
                m_items.Add(new Datafile(DatafileConstants.PropertiesDatafile));
                m_items.Add(new Datafile(DatafileConstants.ItemsDatafile));
                m_items.Add(new Datafile(DatafileConstants.SkillsDatafile));
                m_items.Add(new Datafile(DatafileConstants.CertificatesDatafile));
                m_items.Add(new Datafile(DatafileConstants.GeographyDatafile));
                m_items.Add(new Datafile(DatafileConstants.ReprocessingDatafile));
                m_items.Add(new Datafile(DatafileConstants.BlueprintsDatafile));
            }
            // Don't worry if we cant create MD5 maybe they have FIPS enforced.
            catch (Exception)
            {
                EveClient.Trace("Couldn't compute datafiles checksums. FIPS was enforced, the datafiles were missing, or we couldn't copy to %APPDATA%.");
            }
        }
    }
}