//#define DEBUG_SINGLETHREAD
using System;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

using EVEMon.Common;
using EVEMon.Common.Serialization.BattleClinic;
using EVEMon.WindowsApi;

namespace EVEMon
{
    internal static class Program
    {
        private static bool s_exitRequested;
        private static bool s_showWindowOnError = true;
        private static MainWindow s_mainWindow;
        private static bool s_isDebugBuild = false;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main()
        {
            // Sets isDebugBuild variable to true if this is a debug build
            CheckIsDebug();

            // Quits non-debug builds if another instance already exists
            if (!s_isDebugBuild && !IsInstanceUnique)
                return;

            // Subscribe application's events (especially the unhandled exceptions management for the crash box)
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
            Application.ThreadException += Application_ThreadException;
            Application.ApplicationExit += ApplicationExitCallback;
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);

            // Find our files
            EveClient.InitializeFileSystemPaths();

            // Creates a trace file
            EveClient.StartTraceLogging();
            EveClient.Trace("Starting up");

            // Make our windows nice
            MakeWindowsJuicy();

            // Check arguments
            bool startMinimized = Environment.GetCommandLineArgs().Contains("-startMinimized");

            // Initialization
            EveClient.Initialize();
            Settings.InitializeFromFile();

            // Did something requested an exit before we entered Run() ?
            if (s_exitRequested)
                return;

            // Fires the main window
            try
            {
                EveClient.Trace("Main loop - start"); 
                Application.Run(new MainWindow(startMinimized));
                EveClient.Trace("Main loop - done");
            }
            // Save before we quit
            finally
            {
                Settings.SaveImmediate();
                EveIDtoName.Save();
                BCAPI.UploadSettingsFile();
                EveClient.Trace("Closed");
                EveClient.StopTraceLogging();
            }
        }

        #region Properties

        /// <summary>
        /// The main window of the application
        /// </summary>
        public static MainWindow MainWindow
        {
            get { return s_mainWindow; }
            set { s_mainWindow = value; }
        }

        /// <summary>
        /// Ensures that only one instance of EVEMon is ran at once
        /// </summary>
        private static bool IsInstanceUnique
        {
            get
            {
                InstanceManager im = InstanceManager.GetInstance();
                if (!im.CreatedNew)
                {
                    im.Signal();
                    return false;
                }

                return true;
            }
        }

        #endregion


        #region Helpers
        /// <summary>
        /// Makes the windows nice.
        /// </summary>
        private static void MakeWindowsJuicy()
        {
            try
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                String appId = "EVEMon";
                if (s_isDebugBuild)
                    appId = String.Format(CultureConstants.DefaultCulture, "{0}-DEBUG", appId);

                Windows7.SetProcessAppID(appId);
            }
            catch (Exception ex)
            {
                // On some systems, a crash may occur here because of some skinning programs or others.
                ExceptionHandler.LogException(ex, true);
            }
        }

        /// <summary>
        /// Will only execute if DEBUG is set, thus lets us avoid #IFDEF
        /// </summary>
        [Conditional("DEBUG")]
        private static void CheckIsDebug()
        {
            s_isDebugBuild = true;
        }

        #endregion


        #region Callbacks
        /// <summary>
        /// If <see cref="Application.Exit()"/> is called before the <see cref="Application.Run()"/> method, then it won't occur. 
        /// So, here, we set up a boolean to prevent that.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private static void ApplicationExitCallback(object sender, EventArgs e)
        {
            s_exitRequested = true;
        }

        /// <summary>
        /// Occurs when an exception reach the entry point of the
        /// application. We then display our custom crash box.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            HandleUnhandledException(e.ExceptionObject as Exception);
        }

        /// <summary>
        /// Handles exceptions in WinForms threads, such exceptions
        /// would never reach the entry point of the application, 
        /// generally causing a CTD or trigger WER. We display our
        /// custom crash box
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private static void Application_ThreadException(object sender, ThreadExceptionEventArgs e)
        {
            HandleUnhandledException(e.Exception as Exception);
        }

        /// <summary>
        /// Handles an exception through the Unhandled Exception window
        /// </summary>
        /// <param name="ex">Exception to display</param>
        private static void HandleUnhandledException(Exception ex)
        {
            if (!Debugger.IsAttached)
            {
                if (s_showWindowOnError)
                {
                    s_showWindowOnError = false;

                    // Shutdown EveClient timer incase that was causing the crash
                    // so we don't get multiple crashes
                    try
                    {
                        EveClient.Shutdown();
                        using (UnhandledExceptionWindow f = new UnhandledExceptionWindow(ex))
                        {
                            f.ShowDialog(s_mainWindow);
                        }
                    }
                    catch
                    {
                        StringBuilder MessageBuilder = new StringBuilder();
                        MessageBuilder.AppendLine("An error occurred and EVEMon was unable to handle the error message gracefully");
                        MessageBuilder.AppendLine();
                        MessageBuilder.AppendFormat(CultureConstants.DefaultCulture, "The exception encountered was '{0}'.", ex.Message);
                        MessageBuilder.AppendLine();
                        MessageBuilder.AppendLine();
                        MessageBuilder.AppendLine("Please report this on the EVEMon forums.");
                        MessageBox.Show(MessageBuilder.ToString(), "EVEMon Error Occurred", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }

                    Environment.Exit(1);
                }
            }
        }
        #endregion
    }
}
