using System;
using System.Threading;

namespace EVEMon.Common.Threading
{
    /// <summary>
    /// Encapsulates an action, also providing synchronization services
    /// </summary>
    public struct Message
    {
        private readonly Action m_action;
        private readonly EventWaitHandle m_waitHandle;

        /// <summary>
        /// Constructor for an message without waiting mechanism
        /// </summary>
        /// <param name="action"></param>
        public Message(Action action)
        {
            this.m_action = action;
            this.m_waitHandle = null;
        }

        /// <summary>
        /// Constructor for a message with or without waiting mechanism
        /// </summary>
        /// <param name="action"></param>
        /// <param name="waitForInvocation">When true, the message creator requires a waiting mechanism</param>
        public Message(Action action, bool waitForInvocation)
        {
            this.m_action = action;

            if (waitForInvocation) this.m_waitHandle = new EventWaitHandle(false, EventResetMode.AutoReset);
            else this.m_waitHandle = null;
        }

        /// <summary>
        /// Invoke the action and, when appliable, signal the creator thread.
        /// </summary>
        public void Invoke()
        {
            this.m_action();
            if (this.m_waitHandle != null) m_waitHandle.Set();
        }

        /// <summary>
        /// Wait for the action completion by the owner of the messages pump
        /// </summary>
        public void Wait()
        {
            if (this.m_waitHandle != null) this.m_waitHandle.WaitOne();
        }

        /// <summary>
        /// Cancels the operation, just signals the waiting thread when any
        /// </summary>
        public void Cancel()
        {
            if (this.m_waitHandle != null) m_waitHandle.Set();
        }
    }
}
