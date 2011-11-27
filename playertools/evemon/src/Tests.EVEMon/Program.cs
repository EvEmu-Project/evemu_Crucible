using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;
using System.Reflection;

namespace Tests.EVEMon
{
    internal static class Program
    {
        // Some constants we are going to use
        private static readonly string[] m_programsPath = new string[2] { @"C:\Program Files\", @"C:\Program Files (x86)\" };
        private static readonly string m_executableName = "nunit-x86.exe"; // must be the x86 version of nUnit because this is a x86 Project
        private static readonly string m_searchPattern = "nunit *";
        private static readonly string m_binFolder = "bin/net-2.0";
        private static readonly string m_arguments = "/run \"{0}\"";

        [STAThread]
        public static void Main()
        {
            string executable = GetNUnitExecutable();

            if (String.IsNullOrEmpty(executable) || !File.Exists(executable))
            {
                // Could not any version of nUnit
                Console.WriteLine("****");
                Console.WriteLine("Did not find {0}, check it is installed.", m_executableName);
                Console.ReadKey();
                return;
            }

            // Great we found nUnit now lets start it
            ProcessStartInfo psi = new ProcessStartInfo(executable);
            psi.Arguments = String.Format(m_arguments, Assembly.GetEntryAssembly().Location);
            Process proc = new Process();
            proc.StartInfo = psi;
            proc.Start();
        }

        /// <summary>
        /// Gets the full path of the NUnit executable.
        /// </summary>
        /// <returns></returns>
        private static string GetNUnitExecutable()
        {
            string executable;

            List<PathVersion> versions = GetAllNUnitInstalls();

            if (versions.IsEmpty())
                return String.Empty;

            Version verMax = versions.Select(x => x.Version).Max();

            string newestInstall = versions.Where(x => x.Version == verMax).First().Path;

            string binPath = Path.Combine(newestInstall, m_binFolder);
            executable = Path.Combine(binPath, m_executableName);
            return executable;
        }

        /// <summary>
        /// Gets a list of all installs of NUnit.
        /// </summary>
        /// <returns></returns>
        private static List<PathVersion> GetAllNUnitInstalls()
        {
            List<PathVersion> versions = new List<PathVersion>();

            foreach (var path in m_programsPath)
            {
                if (!Directory.Exists(path))
                    continue;

                var matchingFolders = Directory.GetDirectories(path, m_searchPattern);

                if (matchingFolders.Length == 0)
                    continue;

                foreach (var folder in matchingFolders)
                {
                    string versionName = Path.GetFileName(folder).Remove(0, 6);

                    Version version = new Version(versionName);

                    versions.Add(new PathVersion(folder, version));
                }
            }

            return versions;
        }
    }

    /// <summary>
    /// Relationship class for Paths and Versions.
    /// </summary>
    internal class PathVersion
    {
        internal PathVersion(String path, Version version)
        {
            Path = path;
            Version = version;
        }

        internal String Path
        {
            get;
            private set;
        }

        internal Version Version
        {
            get;
            private set;
        }
    }
}
