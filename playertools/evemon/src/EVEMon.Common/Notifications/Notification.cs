using System;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel;
using EVEMon.Common.Attributes;

namespace EVEMon.Common.Notifications
{
    /// <summary>
    /// Represents an argument for a notification invalidation.
    /// </summary>
    public class Notification : EventArgs
    {
        protected readonly NotificationCategory m_category;
        protected readonly Object m_sender;

        protected NotificationBehaviour m_behaviour;
        protected NotificationPriority m_priority;
        protected string m_description;

        protected bool m_userValidated;


        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="category"></param>
        /// <param name="sender"></param>
        public Notification(NotificationCategory category, Object sender)
        {
            m_category = category;
            m_sender = sender;
        }

        /// <summary>
        /// Gets this category's notification.
        /// </summary>
        public NotificationCategory Category
        {
            get { return m_category; }
        }

        /// <summary>
        /// Gets the sender of this notification.
        /// </summary>
        public Object Sender
        {
            get { return m_sender; }
        }

        /// <summary>
        /// Gets the character who sent this notification, or null if the sender was not a character.
        /// </summary>
        public Character SenderCharacter
        {
            get { return m_sender as Character; }
        }

        /// <summary>
        /// Gets the account which sent this notification, or null if the sender was not an account.
        /// </summary>
        public Account SenderAccount
        {
            get { return m_sender as Account; }
        }

        /// <summary>
        /// Gets or sets the description.
        /// </summary>
        public string Description
        {
            get { return m_description; }
            set { m_description = value; }
        }

        /// <summary>
        /// Gets or sets the priority for this notification.
        /// </summary>
        public NotificationPriority Priority
        {
            get { return m_priority; }
            set { m_priority = value; }
        }

        /// <summary>
        /// Gets or sets the behaviour of this notification regarding other notifications with the same validation key.
        /// </summary>
        public NotificationBehaviour Behaviour
        {
            get { return m_behaviour; }
            set { m_behaviour = value; }
        }

        /// <summary>
        /// Gets or sets true whether the user checked that notification.
        /// </summary>
        public bool UserValidated
        {
            get { return m_userValidated; }
            set { m_userValidated = value; }
        }

        /// <summary>
        /// Gets true if the notification has details.
        /// </summary>
        public virtual bool HasDetails
        {
            get { return false; }
        }

        /// <summary>
        /// Gets a key, identifying a category/sender pair, that will be used for invalidation.
        /// </summary>
        public virtual long InvalidationKey
        {
            get { return GetKey(m_sender, m_category); }
        }

        /// <summary>
        /// Gets the key for the given sender and category.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="category"></param>
        /// <returns></returns>
        public static long GetKey(Object sender, NotificationCategory category)
        {
            var left = ((long)category) << 32;
            var right = (sender == null ? 0 : sender.GetHashCode());
            return left | unchecked((uint)right);
        }

        /// <summary>
        /// Appends a given notification to this one.
        /// </summary>
        /// <param name="?"></param>
        public virtual void Append(Notification other)
        {
            // Must have to be implemented by inheritors.
            throw new NotImplementedException();
        }

        /// <summary>
        /// Gets the description for this 
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return m_description;
        }
    }
}
