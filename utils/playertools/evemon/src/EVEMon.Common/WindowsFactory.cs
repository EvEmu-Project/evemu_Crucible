using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace EVEMon.Common
{
    /// <summary>
    /// This factory allows us keep unique instances of 
    /// </summary>
    /// <typeparam name="TForm">The type of windows ro create</typeparam>
    public static class WindowsFactory<TForm>
        where TForm : Form
    {
        private static readonly Object m_syncLock = new object();
        private static readonly List<TForm> m_taggedWindows = new List<TForm>();
        private static TForm m_uniqueWindow;

        /// <summary>
        /// Close the unique window.
        /// </summary>
        public static void CloseUnique()
        {
            lock (m_syncLock)
            {
                try
                {
                    // Does it already exist ?
                    if (m_uniqueWindow != null && !m_uniqueWindow.IsDisposed)
                        m_uniqueWindow.Close();
                }
                // Catch exception when the window is being disposed
                catch (ObjectDisposedException ex)
                {
                    ExceptionHandler.LogException(ex, true);
                }
            }
        }

        /// <summary>
        /// Gets the window displayed as unique if it exists, null otherwise.
        /// </summary>
        /// <returns></returns>
        public static TForm GetUnique()
        {
            lock (m_syncLock)
            {
                try
                {
                    // Does it already exist ?
                    if (m_uniqueWindow != null && !m_uniqueWindow.IsDisposed)
                    {
                        // Bring to front or show
                        if (m_uniqueWindow.Visible)
                        {
                            m_uniqueWindow.BringToFront();
                        }
                        else
                        {
                            m_uniqueWindow.Show();
                        }

                        // Give focus and return
                        m_uniqueWindow.Activate();
                        return m_uniqueWindow;
                    }
                }
                // Catch exception when the window is being disposed
                catch (ObjectDisposedException ex)
                {
                    ExceptionHandler.LogException(ex, true);
                }

                return null;
            }
        }

        /// <summary>
        /// Show the unique window. 
        /// When none exist, it is created using the default constructor
        /// When it already exists, it is bringed to front, or show when hidden.
        /// </summary>
        /// <param name="creation"></param>
        /// <returns></returns>
        public static TForm ShowUnique()
        {
            return ShowUnique(() => Create());
        }

        /// <summary>
        /// Show the unique window. 
        /// When none exist, it is created using the provided callback.
        /// When it already exists, it is bringed to front, or show when hidden.
        /// </summary>
        /// <param name="creation"></param>
        /// <returns></returns>
        public static TForm ShowUnique(Func<TForm> creation)
        {
            lock (m_syncLock)
            {
                try
                {
                    // Does it already exist ?
                    if (m_uniqueWindow != null && !m_uniqueWindow.IsDisposed)
                    {
                        // Bring to front or show
                        if (m_uniqueWindow.Visible)
                        {
                            m_uniqueWindow.BringToFront();
                        }
                        else
                        {
                            m_uniqueWindow.Show();
                        }

                        // Give focus and return
                        m_uniqueWindow.Activate();
                        return m_uniqueWindow;
                    }
                }
                // Catch exception when the window is being disposed
                catch (ObjectDisposedException ex)
                {
                    ExceptionHandler.LogException(ex, true);
                }

                // Create the window and subscribe to its closing for cleanup
                m_uniqueWindow = creation();
                m_uniqueWindow.FormClosing += (FormClosingEventHandler)((sender, args) => {
                    lock(m_syncLock)
                    { 
                        m_uniqueWindow = null;
                    }});

                // Show and return
                m_uniqueWindow.Show();
                return m_uniqueWindow;
            }
        }

        /// <summary>
        /// Gets the existing form associated with the given tag.
        /// </summary>
        /// <typeparam name="TTag"></typeparam>
        /// <param name="tag"></param>
        /// <returns></returns>
        public static TForm GetByTag<TTag>(TTag tag)
            where TTag : class
        {
            Object otag = (Object)tag;

            lock (m_syncLock)
            {
                // Does it already exist ?
                foreach (var existingWindow in m_taggedWindows)
                {
                    try
                    {
                        if (existingWindow.Tag == otag)
                            return existingWindow;
                    }
                    // Catch exception when the window was disposed
                    catch (ObjectDisposedException ex)
                    {
                        ExceptionHandler.LogException(ex, true);
                    }
                }

                // Not found, we return null
                return null;
            }
        }


        /// <summary>
        /// Show the window with the given tag.
        /// When none exist, it is created using the public constructor accepting an argument of type <see cref="TArg"/>, or the default constructor if the previous one does not exist.
        /// When it already exists, it is bringed to front, or show when hidden.
        /// </summary>
        /// <param name="tag"></param>
        /// <returns></returns>
        public static TForm ShowByTag<TTag>(TTag tag)
            where TTag : class
        {
            return ShowByTag(tag, (data) => Create<TTag>(data));
        }

        /// <summary>
        /// Show the window with the given tag.
        /// When none exist, it is created using the provided callback, and the provided tag is then associated with it.
        /// When it already exists, it is bringed to front, or show when hidden.
        /// </summary>
        /// <param name="tag"></param>
        /// <param name="creation"></param>
        /// <returns></returns>
        public static TForm ShowByTag<TTag>(TTag tag, Func<TTag, TForm> creation)
            where TTag : class
        {
            Object otag = (Object)tag;

            lock (m_syncLock)
            {
                // Does it already exist ?
                foreach(var existingWindow in m_taggedWindows)
                {
                    try
                    {
                        if (existingWindow.Tag != otag)
                            continue;

                        // Bring to front or show
                        if (existingWindow.Visible)
                        {
                            existingWindow.BringToFront();
                        }
                        else
                        {
                            existingWindow.Show();
                        }

                        // Give focus and return
                        existingWindow.Activate();
                        return existingWindow;
                    }
                    // Catch exception when the window was disposed
                    catch (ObjectDisposedException ex)
                    {
                        ExceptionHandler.LogException(ex, true);
                    }
                }

                // Create the window and attach the tag
                TForm window = creation(tag);
                window.Tag = otag;

                // Store it and subscribe to closing for clean up
                m_taggedWindows.Add(window);
                window.FormClosing += (FormClosingEventHandler)((sender, args) => {
                    lock(m_syncLock) 
                    {
                        m_taggedWindows.Remove((TForm)sender);
                    }});

                // Show and return
                window.Show();
                return window;
            }
        }

        /// <summary>
        /// Call the default constructor.
        /// </summary>
        /// <returns></returns>
        private static TForm Create()
        {
            return (TForm)(typeof(TForm).GetConstructor(Type.EmptyTypes).Invoke(null));
        }

        /// <summary>
        /// Call the public constructor with the provided argument type.
        /// </summary>
        /// <returns></returns>
        private static TForm Create<TArg>(TArg data)
        {
            // Search a public instance constructor with a single argument of type TArg
            var ctor = typeof(TForm).GetConstructor(new Type[] { typeof(TArg) });
            if (ctor != null) 
                return (TForm)(ctor.Invoke(new Object[] { (object)data }));

            // Failed, use the default constructor
            return Create();
        }

        /// <summary>
        /// Close the window with the given tag.
        /// </summary>
        /// <typeparam name="TTag"></typeparam>
        /// <param name="tag"></param>
        public static void CloseByTag<TTag>(TTag tag)
        {
            Object otag = (Object)tag;

            lock (m_syncLock)
            {
                // While we find windows to close...
                while (true)
                {
                    // Search all the disposed windows or windows with the same tag
                    bool isDisposed = false;
                    TForm formToRemove = null;
                    foreach (var existingWindow in m_taggedWindows)
                    {
                        try
                        {
                            if (existingWindow.Tag == otag)
                            {
                                formToRemove = existingWindow;
                                break;
                            }
                        }
                        // Catch exception when the window was disposed - we will remove it also by the way
                        catch (ObjectDisposedException ex)
                        {
                            ExceptionHandler.LogException(ex, true);
                            formToRemove = existingWindow;
                            isDisposed = true;
                        }
                    }

                    // Returns if nothing found on this cycle
                    if (formToRemove == null)
                        return;

                    if (isDisposed)
                    {
                        m_taggedWindows.Remove(formToRemove);
                    }
                    else
                    {
                        formToRemove.Close();
                    }
                }

            }
        }
    }
}
