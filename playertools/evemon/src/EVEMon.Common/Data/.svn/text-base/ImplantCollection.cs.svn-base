using EVEMon.Common.Collections;

namespace EVEMon.Common.Data
{
    #region ImplantSlot
    /// <summary>
    /// Represents a collection of all the implants bound to a given group.
    /// </summary>
    public sealed class ImplantCollection : ReadonlyCollection<Implant>
    {
        private readonly ImplantSlots m_slot;

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="src"></param>
        internal ImplantCollection(ImplantSlots slot)
            : base()
        {
            m_slot = slot;
        }

        /// <summary>
        /// Gets the slot represented by this group.
        /// </summary>
        public ImplantSlots Slot
        {
            get { return m_slot; }
        }


        /// <summary>
        /// Gets an implant by its name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        public Implant this[string name]
        {
            get
            {
                foreach (var implant in m_items)
                {
                    if (implant.Name == name) return implant;
                }
                return null;
            }
        }

        /// <summary>
        /// Add an implant to this slot.
        /// </summary>
        /// <param name="implant"></param>
        internal void Add(Implant implant)
        {
            m_items.Add(implant);
        }
    }
    #endregion
}
