using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Reflection;

namespace EVEMon.Common.Threading
{
    /// <summary>
    /// This class provides the tools we need to apply the threading model in use in this assembly.
    /// It relies on an internal actor, which should be the UI actor. Most objects will have affinity with this thread and most actions will occur on this actor.
    /// <para>Besides, should the actor be null (for testing purposes), then a single-threaded mode will be enforced, all asynchronous calls being replaced by synchroneous ones.</para>
    /// <para>Finally, note that, in normal mode (actor not null), very few operations are performed on other threads. They are mainly network operations.</para>
    /// </summary>
    public static class Dispatcher
    {
        private static readonly Object m_syncLock = new Object();

        private static IActor m_actor;
        private static IActorTimer m_oneSecondTimer;
        private static readonly SortedList<DateTime, Action> m_delayedOperations = new SortedList<DateTime,Action>();

        /// <summary>
        /// Gets true whether the threading model uses many threads, false otherwise.
        /// </summary>
        public static bool IsMultiThreaded
        {
            get { return m_actor != null; }
        }

        /// <summary>
        /// Gets true if the calling thread is the underlying thread; false otherwise.
        /// When false, operations on the underlying actor have to be done through <see cref="Invoke"/> or <see cref="BeginInvoke"/>.
        /// </summary>
        public static bool HasAccess
        {
            get 
            {
                var actor = m_actor;
                if (actor == null)
                    return true;

                return actor.HasAccess;
            }
        }

        /// <summary>
        /// Gets the underlying actor of the dispatcher.
        /// When null, the dispatcher will run in single-threaded mode.
        /// </summary>
        public static IActor Actor
        {
            get { return m_actor; }
        }

        /// <summary>
        /// Starts the dispatcher with the given actor.
        /// </summary>
        /// <remarks>If the method has already been called previously, this new call will silently fail.</remarks>
        /// <param name="actor">The actor to run on.</param>
        /// <exception cref="ArgumentException">The specified actor is null.</exception>
        internal static void Run(IActor actor)
        {
            Enforce.ArgumentNotNull(actor, "actor");

            // Double-check pattern (1)
            if (m_actor != null)
                return;

            lock (m_syncLock)
            {
                // Double-check pattern (2)
                if (m_actor != null)
                    return;

                m_actor = actor;
                m_oneSecondTimer = actor.GetTimer(OnOneSecondTimerTick, 1000, true);
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the underlying actor and wait for completion.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        public static void Invoke(Action action)
        {
            var actor = m_actor;
            if (HasAccess || (actor == null))
            {
                action();
            }
            else
            {
                actor.Invoke(action);
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the underlying actor and immediately returns without waiting for the completion. 
        /// Note that, when the calling thread and the bound thread are the same, we execute the action immediately without waiting.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        public static void BeginInvoke(Action action)
        {
            var actor = m_actor;
            if (HasAccess || (actor == null))
            {
                action();
            }
            else
            {
                actor.BeginInvoke(action);
            }
        }

        /// <summary>
        /// Schedule an action to invoke on the actor, by specifying a delay from now on (one second resolution).
        /// </summary>
        /// <remarks>
        /// Note the scheduler only has a one second resolution.
        /// <para>If the time is already elasped, the execution will occur now.</para></remarks>
        /// <param name="delay">The delay before the action is executed.</param>
        /// <param name="action">The callback to execute.</param>
        public static void Schedule(TimeSpan delay, Action action)
        {
            var dateTime = DateTime.UtcNow.Add(delay);
            Schedule(dateTime, action);
        }

        /// <summary>
        /// Schedule an action to invoke on the actor, by specifying the time it will be executed  (one second resolution).
        /// </summary>
        /// <remarks>
        /// Note the scheduler only has a one second resolution.
        /// <para>If the time is already elasped, the execution will occur now.</para></remarks>
        /// <param name="action">The callback to execute.</param>
        /// <param name="time">The time at which the delay will be executed.</param>
        public static void Schedule(DateTime time, Action action)
        {
            time = time.ToUniversalTime();

            // Is it already elapsed ?
            if (time < DateTime.UtcNow)
            {
                BeginInvoke(action);
                return;
            }

            // Plan
            lock (m_syncLock)
            {
                // Add one tick until we get a unique key
                while (m_delayedOperations.ContainsKey(time))
                {
                    time = time.AddTicks(1);
                }

                // Stores it in a sorted list.
                m_delayedOperations.Add(time, action);
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the thread pool and immediately returns without waiting for the completion. 
        /// Note that, when the dispatcher runs in single-threaded mode (for testing purposes), the action is actually performed on the calling thread.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        public static void BackgroundInvoke(Action action)
        {
            if (IsMultiThreaded)
            {
                action.BeginInvoke(null, null);
            }
            else
            {
                action.Invoke();
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the thread pool and immediately returns without waiting for the completion. 
        /// Note that, when the dispatcher runs in single-threaded mode (for testing purposes), the action is actually performed on the calling thread.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        /// <param name="callback"></param>
        /// <param name="object"></param>
        public static void BackgroundInvoke(Action action, AsyncCallback callback, object @object)
        {
            if (IsMultiThreaded)
            {
                action.BeginInvoke(callback, @object);
            }
            else
            {
                action.Invoke();
            }
        }

        /// <summary>
        /// Asserts the calling thread is this actor's underlying thread or throws an exception.
        /// </summary>
        public static void AssertAccess()
        {
            var actor = m_actor;
            if (actor != null)
                actor.AssertAccess();
        }

        /// <summary>
        /// Occurs on every second, when the timer ticks.
        /// </summary>
        private static void OnOneSecondTimerTick()
        {
            // Updates the eve client
            EveClient.UpdateOnOneSecondTick();

            // Check for scheduled operations before now
            List<Action> actionsToInvoke = new List<Action>();
            lock (m_syncLock)
            {
                // Collect all the actions scheduled before now
                var now = DateTime.UtcNow;
                foreach(var pair in m_delayedOperations)
                {
                    if (pair.Key > now)
                        break;

                    actionsToInvoke.Add(pair.Value);
                }

                // Remove those actions
                for(int i=0; i<actionsToInvoke.Count; i++)
                {
                    m_delayedOperations.RemoveAt(0);
                }
            }

            // Execute the entries (we're already on the proper thread)
            foreach (var action in actionsToInvoke)
            {
                action.Invoke();
            }
        }

        /// <summary>
        /// Shutdowns the dispatcher.
        /// </summary>
        internal static void Shutdown()
        {

            if (m_oneSecondTimer == null)
                return;

            m_oneSecondTimer.Stop();
            m_oneSecondTimer = null;
        }
    }
}
