using System;
using EVEMon.Common.Serialization.Datafiles;

namespace EVEMon.Common.Data
{
    #region Implant
    /// <summary>
    /// Represents an implant.
    /// </summary>
    public sealed class Implant : Item
    {
        private readonly static Implant m_none = new Implant();

        private readonly int m_bonus;
        private readonly ImplantSlots m_implantSlot;

        /// <summary>
        /// Private constructor for the default.
        /// </summary>
        internal Implant()
            : base(-1, ImplantSlots.None.ToString())
        {
            m_implantSlot = ImplantSlots.None;
        }

        /// <summary>
        /// Deserialization constructor.
        /// </summary>
        /// <param name="group"></param>
        /// <param name="src"></param>
        internal Implant(MarketGroup group, SerializableItem src)
            : base(group, src)
        {
            // Gets the slot
            var slotProperty = this.Properties[DBConstants.ImplantSlotPropertyID];
            m_implantSlot = (slotProperty == null ? ImplantSlots.None : (ImplantSlots)(slotProperty.Value.IValue - 1));

            // Get the bonus
            switch (m_implantSlot)
            {
                case ImplantSlots.Charisma:
                    m_bonus = this.Properties[DBConstants.CharismaModifierPropertyID].Value.IValue;
                    break;
                case ImplantSlots.Intelligence:
                    m_bonus = this.Properties[DBConstants.IntelligenceModifierPropertyID].Value.IValue;
                    break;
                case ImplantSlots.Memory:
                    m_bonus = this.Properties[DBConstants.MemoryModifierPropertyID].Value.IValue;
                    break;
                case ImplantSlots.Perception:
                    m_bonus = this.Properties[DBConstants.PerceptionModifierPropertyID].Value.IValue;
                    break;
                case ImplantSlots.Willpower:
                    m_bonus = this.Properties[DBConstants.WillpowerModifierPropertyID].Value.IValue;
                    break;
                default:
                    break;
            }

            // Adds itself to the implants slot
            StaticItems.GetImplants(m_implantSlot).Add(this);
        }

        /// <summary>
        /// Gets the slot.
        /// </summary>
        public ImplantSlots Slot
        {
            get { return m_implantSlot; }
        }

        /// <summary>
        /// For attributes implants, gets the amount of bonus points it grants.
        /// </summary>
        public int Bonus
        {
            get { return m_bonus; }
        }

        /// <summary>
        /// Converts the provided slot to an attribute. Returns <see cref="Attribute.None"/> when the provided slot does not match any attribute.
        /// </summary>
        /// <param name="slot"></param>
        /// <returns></returns>
        public static EveAttribute SlotToAttrib(ImplantSlots slot)
        {
            switch (slot)
            {
                case ImplantSlots.Perception:
                    return EveAttribute.Perception;
                case ImplantSlots.Memory:
                    return EveAttribute.Memory;
                case ImplantSlots.Willpower:
                    return EveAttribute.Willpower;
                case ImplantSlots.Intelligence:
                    return EveAttribute.Intelligence;
                case ImplantSlots.Charisma:
                    return EveAttribute.Charisma;
                default:
                    return EveAttribute.None;
            }
        }

        /// <summary>
        /// Converts the provided slot to an attribute. Returns <see cref="Attribute.None"/> when the provided slot does not match any attribute.
        /// </summary>
        /// <param name="slot"></param>
        /// <returns></returns>
        public static ImplantSlots AttribToSlot(EveAttribute attr)
        {
            switch (attr)
            {
                case EveAttribute.Perception:
                    return ImplantSlots.Perception;
                case EveAttribute.Memory:
                    return ImplantSlots.Memory;
                case EveAttribute.Willpower:
                    return ImplantSlots.Willpower;
                case EveAttribute.Intelligence:
                    return ImplantSlots.Intelligence;
                case EveAttribute.Charisma:
                    return ImplantSlots.Charisma;
                default:
                    return ImplantSlots.None;
            }
        }

        /// <summary>
        /// Gets an implant for an empty set.
        /// </summary>
        public static Implant None
        {
            get { return m_none; }
        }
    }
    #endregion
}
