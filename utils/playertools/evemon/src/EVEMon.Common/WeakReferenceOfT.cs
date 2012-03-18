using WeakReference=System.WeakReference;
using System;

namespace EVEMon.Common
{
    public class WeakReference<T> : WeakReference
        where T : class
    {
        public WeakReference(T target)
            : base(target)
        {
        }

        public WeakReference(T target, bool trackResurrection)
            : base(target, trackResurrection)
        {
        }

        public new T Target
        {
            get { return base.Target as T; }
        }

        /// <summary>
        /// Try to perform an action over this reference's target.
        /// </summary>
        /// <param name="action"></param>
        /// <returns>True if the action was performed, false if the reference was no longer available.</returns>
        public bool TryDo(Action<T> action)
        {
            // Removes the reference at this index if it does not exist anymore
            if (!this.IsAlive) return false;

            // Try to notify the monitor
            try
            {
                // Retrieve the reference and removes it if null.
                var target = this.Target;
                if (target == null) return false;

                // Notify the monitor.
                action(target);
                return true;
            }
            // Occurs when the target has been garbage collected since the moment we performed the check.
            catch (InvalidOperationException)
            {
                return false;
            }
        }
    }
}