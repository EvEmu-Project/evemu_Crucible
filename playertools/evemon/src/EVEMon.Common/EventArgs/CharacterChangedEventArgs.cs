using System;

namespace EVEMon.Common
{
    public sealed class CharacterChangedEventArgs : EventArgs
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="character"></param>
        public CharacterChangedEventArgs(Character character)
        {
            Character = character;
        }

        /// <summary>
        /// Gets or sets the character related to this event.
        /// </summary>
        public Character Character { get; private set; }
    }
}
