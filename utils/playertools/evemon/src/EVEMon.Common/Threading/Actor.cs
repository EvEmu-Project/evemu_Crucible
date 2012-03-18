using System;
using System.Collections.Generic;
using System.Threading;

#pragma warning disable 420

namespace EVEMon.Common.Threading
{
    /// <summary>
    /// Implements an actor pattern, see <see cref="IActor"/>.
    /// </summary>
    public sealed class Actor : IActor
    {
        /// <summary>
        /// Holds a reference to the actor currently associated with this thread
        /// </summary>
        [ThreadStatic] private static IActor m_currentActor;

        private Thread m_thread;
        private Queue<Message> m_messageQueue;
        private AutoResetEvent m_waitHandle;
        private volatile int m_shutdown;


        /// <summary>
        /// Default constructor with a normal priority
        /// </summary>
        public Actor()
            : this(ThreadPriority.Normal)
        {
        }

        /// <summary>
        /// Constructor with a custom thread priority
        /// </summary>
        /// <param name="priority">The underlying thread's priority</param>
        public Actor(ThreadPriority priority)
        {
            m_messageQueue = new Queue<Message>();
            m_waitHandle = new AutoResetEvent(false);
            m_thread = new Thread(MessagePump);
            m_thread.Priority = priority;
            m_thread.Start();
        }

        /// <summary>
        /// Gets the actor currently asssociated with this thread. May be null if this thread has no actor. 
        /// References regular Actors but also the UIActor singletong when called from the UI thread.
        /// </summary>
        public static IActor CurrentActor
        {
            get { return m_currentActor; }
            internal set { m_currentActor = value; }
        }

        /// <summary>
        /// The thread's main method
        /// </summary>
        private void DoWork()
        {
            // Sets the current actor for this thread
            m_currentActor = this;

            // Pumps the messages until the abortion is fired
            MessagePump();

            // Clean up, signal the waiting threads
            lock (m_messageQueue)
            {
                while (m_messageQueue.Count != 0)
                {
                    m_messageQueue.Dequeue().Cancel();
                }
            }
        }

        /// <summary>
        /// The main loop running on the underlying thread
        /// </summary>
        private void MessagePump()
        {
            // Run indefinitely until aborted
            while (true)
            {
                // Waits to be woke up, or continue if it has been signaled since the last reset/wait. 
                // After 5s, it automatically wakes up to check whether it must abort (should not be needed but, well, just to be sure...)
                try
                {
                    m_waitHandle.WaitOne(5000, false);
                }
                // Should never occur since this thread should be the one waiting for handle
                catch (AbandonedMutexException)
                {
                    m_shutdown = 1;
                    return;
                }

                // Pumps the queue while not empty
                while (true)
                {
                    // Returns if aborted
                    if (m_shutdown != 0) return;

                    // Pumps the next message in queue - require locking
                    Nullable<Message> message = null;
                    lock (m_messageQueue)
                    {
                        // Dequeues the message
                        if (m_messageQueue.Count != 0) message = m_messageQueue.Dequeue();

                        // If a thread has signaled the thread since the last time, we need to reset now
                        m_waitHandle.Reset();
                    }

                    // Quit if the pumpin and go to sleep if the queue was empty
                    if (message == null) break;

                    // Returns if aborted
                    if (m_shutdown != 0) return;

                    // Invoke the next action in queue
                    message.Value.Invoke();
                }
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the bound thread and wait for completion
        /// </summary>
        /// <param name="action">The action to invoke</param>
        /// <returns>True when succesful, false otherwise (the thread has been shutdown).</returns>
        public bool Invoke(Action action)
        {
            return Invoke(action, true);
        }

        /// <summary>
        /// Invoke the provided delegate on the bound thread and immediately returns without waiting for the completion. Note that, when the calling thread and the bound thread are the same, we execute the action immediately without waiting.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        /// <returns>True when succesful, false otherwise (the thread has been shutdown).</returns>
        public bool BeginInvoke(Action action)
        {
            return Invoke(action, false);
        }

        /// <summary>
        /// Performs the invocation, waiting or not for completion
        /// </summary>
        /// <param name="action"></param>
        /// <param name="waitForCompletion">When true, will wait for the completion</param>
        /// <returns>True when succesful, false otherwise (the thread has been shutdown).</returns>
        private bool Invoke(Action action, bool waitForCompletion)
        {
            // Already on the bound thread, execute immediately
            if (Thread.CurrentThread == m_thread)
            {
                if (IsShutdown) return false;
                action();
            }
            // Different thread, push to the invocation queue
            else
            {
                Message msg = new Message(action, waitForCompletion);

                // Push the action on the queue and wake up the bound thread
                lock (m_messageQueue)
                {
                    m_messageQueue.Enqueue(msg);
                    m_waitHandle.Set();
                    if (IsShutdown) return false;
                }

                // Wait for the action to be executed
                if (waitForCompletion) msg.Wait();
                if (IsShutdown) return false;
            }

            return true;
        }

        /// <summary>
        /// Asserts the calling thread is this actor's underlying thread or throws an exception
        /// </summary>
        /// <exception cref="AccessViolationException">The calling thread is different from the underlying thread</exception>
        public void AssertAccess()
        {
            if (Thread.CurrentThread != m_thread)
            {
                throw new AccessViolationException("The calling thread is different from the underlying thread");
            }
        }

        /// <summary>
        /// Forces the bound thread to shutdown in a clean way (no thread abortion)
        /// </summary>
        public void Shutdown()
        {
            // If currently 0, set to 1 and shutdown
            if (Interlocked.Increment(ref m_shutdown) == 1)
            {
                lock (m_messageQueue)
                {
                    // Wakes up the thread to force it to quit
                    m_waitHandle.Set();
                }
            }
        }

        /// <summary>
        /// Gets true if the actor has been shut down
        /// </summary>
        public bool IsShutdown
        {
            get { return m_shutdown != 0; }
        }

        /// <summary>
        /// Gets true if the calling thread is the underlying thread; false otherwise.
        /// When false, operations on this actor have to be done through <see cref="Invoke"/> or <see cref="BeginInvoke"/>.
        /// </summary>
        public bool HasAccess
        {
            get
            {
                if (IsShutdown) return false;
                return Thread.CurrentThread == m_thread;
            }
        }

        /// <summary>
        /// Gets a timer.
        /// </summary>
        /// <param name="callback">The callback invoked on every tick.</param>
        /// <param name="period">The ticking period.</param>
        /// <param name="start">When true, the timer starts on creation.</param>
        /// <returns>The created timer.</returns>
        public IActorTimer GetTimer(Action callback, int period, bool start)
        {
            return new ActorTimer(callback, period, start);
        }
    }
}
