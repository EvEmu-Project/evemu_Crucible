using System;
using System.Windows.Forms;
using System.Diagnostics;
using System.Reflection;
using System.IO;

namespace EVEMon.Watchdog
{
    /// <summary>
    /// Window that monitors the EVEMon process, restarting it when it has closed.
    /// </summary>
    public partial class WatchdogWindow : Form
    {
        private string[] m_args;
        private bool m_executableLaunched = false;

        /// <summary>
        /// Creates the Watchdog Window.
        /// </summary>
        /// <param name="args"></param>
        public WatchdogWindow(string[] args)
        {
            InitializeComponent();
            m_args = args;
        }

        /// <summary>
        /// Load event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void WatchdogWindow_Load(object sender, EventArgs e)
        {
            WaitTimer.Enabled = true;
            StatusLabel.Text = "Waiting for EVEMon to close.";
        }

        /// <summary>
        /// Returns true EVEMon process currently executing.
        /// </summary>
        private bool IsEvemonRunning
        {
            get
            {
                Process[] processes = Process.GetProcessesByName("EVEMon");
                return processes.Length != 0;
            }
        }

        /// <summary>
        /// Timer triggered every 1000msec (1 second)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void WaitTimer_Tick(object sender, EventArgs e)
        {
            // First time through after EVEMon has closed
            if (!m_executableLaunched && !IsEvemonRunning)
            {
                m_executableLaunched = true;
                StartEvemonProcess();
                StatusLabel.Text = "Restarting EVEMon.";
                return;
            }

            // EVEMon has been restarted and is running
            if (m_executableLaunched && IsEvemonRunning)
                Application.Exit();
        }

        /// <summary>
        /// Starts the new EVEMon process.
        /// </summary>
        private void StartEvemonProcess()
        {
            // Find the expected path for EVEMon.exe
            Assembly assembly = Assembly.GetEntryAssembly();
            string path = Path.GetDirectoryName(assembly.Location);
            string executable = Path.Combine(path, "EVEMon.exe");

            // If EVEMon.exe doesn't exist we don't have anything to do
            if (!File.Exists(executable))
                Application.Exit();

            StartProcess(executable, m_args);
        }

        /// <summary>
        /// Starts a process with arguments.
        /// </summary>
        /// <param name="executable">Executable to start (i.e. EVEMon.exe).</param>
        /// <param name="arguments">Arguments to pass to the executable.</param>
        private void StartProcess(string executable, string[] arguments)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo()
            {
                FileName = executable,
                Arguments = String.Join(" ", arguments),
                UseShellExecute = false
            };

            Process evemonProc = new Process()
            {
                StartInfo = startInfo
            };

            evemonProc.Start();
        }
    }
}
