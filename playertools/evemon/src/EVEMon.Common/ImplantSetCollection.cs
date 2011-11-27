using System.Linq;
using System.Collections.Generic;

using EVEMon.Common.Collections;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents a collection of implants sets.
    /// </summary>
    public sealed class ImplantSetCollection : ReadonlyVirtualCollection<ImplantSet>
    {
        private Character m_owner;
        private ImplantSet m_none;
        private ImplantSet m_api;
        private ImplantSet m_oldAPI;
        private ImplantSet m_current;
        private List<ImplantSet> m_customSets;

        /// <summary>
        /// Internal constructor
        /// </summary>
        /// <param name="owner"></param>
        internal ImplantSetCollection(Character owner)
        {
            m_owner = owner;
            m_customSets = new List<ImplantSet>();
            m_oldAPI = new ImplantSet(owner, "Previous implants from the API");
            m_api = new ImplantSet(owner, "Implants from API");
            m_none = new ImplantSet(owner, "<None>");
            m_current = m_api;
        }

        /// <summary>
        /// Gets the implant set by its index. First items are <see cref="API"/>, <see cref="OldAPI"/>, then the custom sets.
        /// </summary>
        /// <param name="set"></param>
        /// <returns></returns>
        public ImplantSet this[int index]
        {
            get { return Enumerate().ElementAt(index); }
        }

        /// <summary>
        /// Gets the none im
        /// </summary>
        public ImplantSet None
        {
            get { return m_none; }
        }

        /// <summary>
        /// Gets the implants retrieved from the API.
        /// </summary>
        public ImplantSet API
        {
            get { return m_api; }
        }

        /// <summary>
        /// Gets the implants previously retrieved from the API.
        /// </summary>
        public ImplantSet OldAPI
        {
            get { return m_oldAPI; }
        }

        /// <summary>
        /// Gets or sets the current implant set.
        /// </summary>
        public ImplantSet Current
        {
            get { return m_current; }
            set
            {
                if (m_current == value)
                    return;
                m_current = value;
                EveClient.OnCharacterChanged(m_owner);
            }
        }

        /// <summary>
        /// Adds a new implant set.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public ImplantSet Add(string name)
        {
            var set = new ImplantSet(m_owner, name);
            m_customSets.Add(set);
            EveClient.OnCharacterChanged(m_owner);
            return set;
        }

        /// <summary>
        /// Removes the given set.
        /// </summary>
        /// <param name="set"></param>
        public void Remove(ImplantSet set)
        {
            m_customSets.Remove(set);
            EveClient.OnCharacterChanged(m_owner);
        }

        /// <summary>
        /// Core method on which everything else relies.
        /// </summary>
        /// <returns></returns>
        protected override IEnumerable<ImplantSet> Enumerate()
        {
            yield return m_api;
            yield return m_oldAPI;
            foreach (var set in m_customSets)
            {
                yield return set;
            }
        }

        /// <summary>
        /// Imports data from a deserialization object
        /// </summary>
        /// <param name="serial"></param>
        public void Import(SerializableImplantSetCollection serial)
        {
            m_api.Import(serial.API, false);
            m_oldAPI.Import(serial.API, false);

            m_customSets.Clear();
            foreach(var serialSet in serial.CustomSets)
            {
                var set = new ImplantSet(m_owner, serialSet.Name);
                set.Import(serialSet, true);
                m_customSets.Add(set);
            }

            // Imports selection
            m_current = Enumerate().ElementAt(serial.SelectedIndex);

            EveClient.OnSettingsChanged();
        }

        /// <summary>
        /// Exports this collection to a serialization object.
        /// </summary>
        /// <returns></returns>
        public SerializableImplantSetCollection Export()
        {
            var serial = new SerializableImplantSetCollection();
            serial.API = m_api.Export();
            serial.OldAPI = m_oldAPI.Export();
            serial.CustomSets.AddRange(m_customSets.Select(x => x.Export()));
            serial.SelectedIndex = Enumerate().IndexOf(m_current);
            return serial;
        }

        /// <summary>
        /// Imports data from an API serialization object provided by CCP
        /// </summary>
        /// <param name="serial"></param>
        internal void Import(SerializableImplantSet serial)
        {
            // Search whether the api infos are different from the ones currently stored.
            var newSet = new ImplantSet(m_owner, "temp");
            newSet.Import(serial);

            bool isDifferent = false;
            var oldArray = m_api.ToArray();
            var newArray = newSet.ToArray();
            for (int i = 0; i < oldArray.Length; i++)
            {
                isDifferent |= (oldArray[i] != newArray[i]);
            }

            // Imports the API and make a backup
            if (isDifferent) m_oldAPI.Import(m_api.Export(), false);
            m_api.Import(serial);

            EveClient.OnSettingsChanged();
        }
    }
}
