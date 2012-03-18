using System;
using System.Collections.Generic;

using EVEMon.Common.Attributes;
using EVEMon.Common.Collections;
using EVEMon.Common.Data;
using EVEMon.Common.Serialization.API;
using EVEMon.Common.Serialization.Settings;

namespace EVEMon.Common
{
    /// <summary>
    /// Represents one of the implant sets this character have (one per clone).
    /// </summary>
    [EnforceUIThreadAffinity]
    public sealed class ImplantSet : ReadonlyVirtualCollection<Implant>
    {
        private string m_name;
        private readonly Character m_owner;
        private readonly Implant[] m_values;

        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <param name="owner"></param>
        /// <param name="set"></param>
        internal ImplantSet(Character owner, string name)
        {
            m_name = name;
            m_owner = owner;

            m_values = new Implant[10];
            for (int i = 0; i < 10; i++)
            {
                m_values[i] = Implant.None;
            }
        }

        /// <summary>
        /// Gets the set's name.
        /// </summary>
        public string Name
        {
            get { return m_name; }
            set 
            {
                if (m_name == value)
                    return;

                m_name = value;

                EveClient.OnCharacterChanged(m_owner);
            }
        }

        /// <summary>
        /// Gets or sets the implant for the given slot.
        /// </summary>
        /// <param name="slot">The slot for the implant to retrieve</param>
        /// <returns>The requested implant when found; null otherwise.</returns>
        public Implant this[ImplantSlots slot]
        {
            get 
            { 
                if (slot == ImplantSlots.None) return null;
                return m_values[(int)slot]; 
            }
            set 
            { 
                if (slot == ImplantSlots.None)
                    throw new InvalidOperationException("Cannot assign 'none' slot");

                if (value != null && value.Slot != slot)
                    throw new InvalidOperationException("Slot mismatch");

                if (value == null)
                {
                    value = Implant.None;
                }
                else
                {
                    m_values[(int)slot] = value;
                }

                EveClient.OnCharacterChanged(m_owner);
            }
        }

        /// <summary>
        /// Gets / sets the implant for the given slot.
        /// </summary>
        /// <param name="attrib">The attribute for the implant to retrieve</param>
        /// <returns>The requested implant when found; null otherwise.</returns>
        public Implant this[EveAttribute attrib]
        {
            get 
            { 
                return this[Implant.AttribToSlot(attrib)];
            }
            set 
            { 
                this[Implant.AttribToSlot(attrib)] = value; 
            }
        }

        /// <summary>
        /// Core method on which everything else relies.
        /// </summary>
        /// <returns></returns>
        protected override IEnumerable<Implant> Enumerate()
        {
            foreach (var implant in m_values)
            {
                yield return implant;
            }
        }

        /// <summary>
        /// Generates a serialization object.
        /// </summary>
        /// <returns></returns>
        internal SerializableSettingsImplantSet Export()
        {
            var serial = new SerializableSettingsImplantSet();
            serial.Intelligence = Export(ImplantSlots.Intelligence);
            serial.Perception = Export(ImplantSlots.Perception);
            serial.Willpower = Export(ImplantSlots.Willpower);
            serial.Charisma = Export(ImplantSlots.Charisma);
            serial.Memory = Export(ImplantSlots.Memory);
            serial.Slot6 = Export(ImplantSlots.Slot6);
            serial.Slot7 = Export(ImplantSlots.Slot7);
            serial.Slot8 = Export(ImplantSlots.Slot8);
            serial.Slot9 = Export(ImplantSlots.Slot9);
            serial.Slot10 = Export(ImplantSlots.Slot10);
            serial.Name = m_name;
            return serial;
        }

        /// <summary>
        /// Exports an implant as a serialization object.
        /// </summary>
        /// <param name="slot"></param>
        /// <returns></returns>
        private string Export(ImplantSlots slot)
        {
            return m_values[(int)slot].Name;
        }

        /// <summary>
        /// Imports data from a settings serialization object.
        /// </summary>
        /// <param name="serial"></param>
        internal void Import(SerializableSettingsImplantSet serial, bool importName)
        {
            Import(ImplantSlots.Intelligence, serial.Intelligence);
            Import(ImplantSlots.Perception, serial.Perception);
            Import(ImplantSlots.Willpower, serial.Willpower);
            Import(ImplantSlots.Charisma, serial.Charisma);
            Import(ImplantSlots.Memory, serial.Memory);
            Import(ImplantSlots.Slot6, serial.Slot6);
            Import(ImplantSlots.Slot7, serial.Slot7);
            Import(ImplantSlots.Slot8, serial.Slot8);
            Import(ImplantSlots.Slot9, serial.Slot9);
            Import(ImplantSlots.Slot10, serial.Slot10);

            if (importName)
                m_name = serial.Name;
        }

        /// <summary>
        /// Updates the given slot with the provided serialization object.
        /// </summary>
        /// <param name="slot"></param>
        /// <param name="serial"></param>
        private void Import(ImplantSlots slot, string name)
        {
            // Backwards compatibility for older versions
            name = name.Replace("<", String.Empty).Replace(">", String.Empty);
            
            m_values[(int)slot] = StaticItems.GetImplants(slot)[name];
        }

        /// <summary>
        /// Imports data from an API serialization object.
        /// </summary>
        /// <param name="serial"></param>
        /// <param name="overrideManual"></param>
        internal void Import(SerializableImplantSet serial)
        {
            Import(ImplantSlots.Intelligence, serial.Intelligence);
            Import(ImplantSlots.Perception, serial.Perception);
            Import(ImplantSlots.Willpower, serial.Willpower);
            Import(ImplantSlots.Charisma, serial.Charisma);
            Import(ImplantSlots.Memory, serial.Memory);
        }

        /// <summary>
        /// Updates the given slot with the provided implant's name.
        /// </summary>
        /// <param name="slot"></param>
        /// <param name="src"></param>
        private void Import(ImplantSlots slot, SerializableImplant src)
        {
            int index = (int)slot;
            m_values[index] = (src == null ? Implant.None : StaticItems.GetImplants(slot)[src.Name]);
        }

    }
}
