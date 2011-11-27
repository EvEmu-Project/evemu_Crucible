using System;
using System.Windows.Forms;
using System.Threading;

#pragma warning disable 420

namespace EVEMon.Common.Threading
{
    /// <summary>
    /// Implements a pseudo-actor pattern bound to the UI thread, see also <see cref="IActor"/>.
    /// </summary>
    public sealed class UIActor : IActor
    {
        private Form m_applicationForm;
        private volatile int m_shutdown;

        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <param name="mainApplicationForm">The application's main form</param>
        public UIActor(Form mainApplicationForm)
        {
            m_applicationForm = mainApplicationForm;

            m_applicationForm.Invoke((Action)(() =>
            {
                if (Actor.CurrentActor != null) Actor.CurrentActor = this;
            }));
        }

        /// <summary>
        /// Gets the actor the instances of this type have an affinity with. 
        /// </summary>
        public Form MainForm
        {
            get { return m_applicationForm; }
        }

        /// <summary>
        /// Invoke the provided delegate on the bound thread and wait for completion
        /// </summary>
        /// <param name="action">The action to invoke</param>
        /// <exception cref="NullReferenceException">The main form has not been specified through <see cref="Initialize()"/> yet.</exception>
        /// <returns>True when succesful, false otherwise (the thread has been shutdown).</returns>
        public bool Invoke(Action action)
        {
            try
            {
                m_applicationForm.Invoke(action);
                return true;
            }
            catch (ObjectDisposedException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return false;
            }
            catch (AccessViolationException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return false;
            }
        }

        /// <summary>
        /// Invoke the provided delegate on the bound thread and immediately returns without waiting for the completion. Note that, when the calling thread and the bound thread are the same, we execute the action immediately without waiting.
        /// </summary>
        /// <param name="action">The action to invoke</param>
        /// <exception cref="NullReferenceException">The main form has not been specified through <see cref="Initialize()"/> yet.</exception>
        /// <returns>True when succesful, false otherwise (the thread has been shutdown).</returns>
        public bool BeginInvoke(Action action)
        {
            try
            {
                m_applicationForm.BeginInvoke(action);
                return true;
            }
            catch (ObjectDisposedException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return false;
            }
            catch (AccessViolationException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return false;
            }
        }

        /// <summary>
        /// Asserts the calling thread is this actor's underlying thread or throws an exception
        /// </summary>
        /// <exception cref="AccessViolationException">The calling thread is different from the underlying thread</exception>
        public void AssertAccess()
        {
            try
            {
                if (!m_applicationForm.InvokeRequired) return;
            }
            catch (ObjectDisposedException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return;
            }
            catch (AccessViolationException exc)
            {
                ExceptionHandler.LogException(exc, true);
                Interlocked.Increment(ref m_shutdown);
                return;
            }

            throw new AccessViolationException("The calling thread is different from the underlying thread");
        }

        /// <summary>
        /// Forces the bound thread to shutdown in a clean way (no thread abortion)
        /// </summary>
        /// <exception cref="NullReferenceException">The main form has not been specified through <see cref="Initialize()"/> yet.</exception>
        public void Shutdown()
        {
            if (Interlocked.Increment(ref m_shutdown) == 1)
            {
                try
                {
                    m_applicationForm.Close();
                }
                catch (ObjectDisposedException exc)
                {
                    ExceptionHandler.LogException(exc, true);
                    Interlocked.Increment(ref m_shutdown);
                    return;
                }
                catch (AccessViolationException exc)
                {
                    ExceptionHandler.LogException(exc, true);
                    Interlocked.Increment(ref m_shutdown);
                    return;
                }
            }
        }

        /// <summary>
        /// Gets true if the actor has been shutdown
        /// </summary>
        /// <exception cref="NullReferenceException">The main form has not been specified through <see cref="Initialize()"/> yet.</exception>
        public bool IsShutdown
        {
            get 
            {
                return m_shutdown != 0 || m_applicationForm.IsDisposed;
            }
        }

        /// <summary>
        /// Gets true if the calling thread is the underlying thread and is living; false otherwise.
        /// When false, operations on this actor have to be done through <see cref="Invoke"/> or <see cref="BeginInvoke"/>.
        /// </summary>
        /// <exception cref="NullReferenceException">The main form has not been specified through <see cref="Initialize()"/> yet.</exception>
        public bool HasAccess
        {
            get 
            {
                try
                {
                    return !m_applicationForm.InvokeRequired;
                }
                catch (ObjectDisposedException exc)
                {
                    ExceptionHandler.LogException(exc, true);
                    Interlocked.Increment(ref m_shutdown);
                    return false;
                }
                catch (AccessViolationException exc)
                {
                    ExceptionHandler.LogException(exc, true);
                    Interlocked.Increment(ref m_shutdown);
                    return false;
                }
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
            return new UIActorTimer(m_applicationForm, callback, period, start);
        }
    }
}
