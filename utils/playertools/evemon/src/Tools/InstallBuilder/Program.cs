using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Windows.Forms;
using ICSharpCode.SharpZipLib.Zip;

namespace InstallBuilder
{
    public class Program
    {
        private static string s_installerDir = Path.GetFullPath(@"..\..\..\..\..\EVEMon\bin\x86\Installbuilder\Installer");
        private static string s_binariesDir = Path.GetFullPath(@"..\..\..\..\..\EVEMon\bin\x86\Installbuilder\Binaries");
        private static string s_programFilesDir = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);

        private static string s_projectDir;
        private static string s_version;
        private static string s_sourceFilesDir;
        private static string s_nsisExe;

        public static int Main(string[] args)
        {
            CheckDebug();

            bool NSISExist = PopulateEnvironment(args);

            if (!NSISExist)
                return 0;

            // Create the installer folder if it doesn't exist
            if (!Directory.Exists(s_installerDir))
                Directory.CreateDirectory(s_installerDir);

            // Create the binaries folder if it doesn't exist
            if (!Directory.Exists(s_binariesDir))
                Directory.CreateDirectory(s_binariesDir);

            try
            {
                if (!String.IsNullOrEmpty(s_nsisExe))
                {
                    // Create an installer on the developers desktop
                    Console.WriteLine("Starting Installer creation.");
                    BuildInstaller();
                    Console.WriteLine("Installer creation finished.");
                    Console.WriteLine();
                }

                // Create a zip file on the developers desktop
                Console.WriteLine("Starting zip installer creation.");
                BuildZip();
                Console.WriteLine("Zip installer creation finished.");
                Console.WriteLine("Done");
                if (Debugger.IsAttached)
                    Console.ReadLine();
                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine("An error occurred: {0} in {1}", ex.Message, ex.Source);
                Console.WriteLine();
                Console.WriteLine(ex.StackTrace);
                if (Debugger.IsAttached)
                    Console.ReadLine();
                return 1;
            }
        }

        [Conditional("DEBUG")]
        private static void CheckDebug()
        {
            Application.Exit();
        }

        private static string FindMakeNsisExe()
        {
            string[] locations = new string[3];

            locations[0] = s_programFilesDir + @"\NSIS\makensis.exe";
            locations[1] = @"D:\Program Files\NSIS\makensis.exe"; // Possible location in TeamCity server
            locations[2] = @"D:\Program Files (x86)\NSIS\makensis.exe"; // Possible location in TeamCity server
            
            foreach (string path in locations)
            {
                if (File.Exists(path))
                    return path;
            }

            return String.Empty;
        }

        private static bool PopulateEnvironment(string[] args)
        {
            if (args.Length == 0)
            {
                s_projectDir = Path.GetFullPath(@"..\..\..");
            }
            else
            {
                s_projectDir = String.Join(" ", args);
            }

            try
            {
                Assembly exeAsm = Assembly.LoadFrom(@"..\..\..\..\..\EVEMon\bin\x86\Release\EVEMon.exe");
                s_version = exeAsm.GetName().Version.ToString();
            }
            catch (Exception)
            {
                Console.WriteLine("A \"Release\" has to be compiled first.");
                Console.WriteLine("Install Builder will now close.");
                Console.ReadLine();
                return false;
            }

            Console.WriteLine("Project directory : {0}", s_projectDir);

            s_sourceFilesDir = Path.GetFullPath(@"..\..\..\..\..\EVEMon\bin\x86\Release");
            Console.WriteLine("Source directory : {0}", s_sourceFilesDir);
            Console.WriteLine("Installer directory : {0}", s_installerDir);
            Console.WriteLine("Binaries directory : {0}", s_binariesDir);
            Console.WriteLine();

            s_nsisExe = FindMakeNsisExe();
            Console.WriteLine("NSIS : {0}", String.IsNullOrEmpty(s_nsisExe) ? "Not Found - Installer will not be created." : s_nsisExe);
            Console.WriteLine();

            return true;
        }

        private static void BuildZip()
        {
            string formattedDate = DateTime.Now.ToString("yyyy-MM-dd");
            string svnRevision = s_version.Substring(s_version.LastIndexOf('.') + 1, s_version.Length - (s_version.LastIndexOf('.') + 1));
            string zipFileName = String.Format("EVEMon-binaries-{0}.zip", s_version);
            zipFileName = Path.Combine(s_binariesDir, zipFileName);

            string[] filenames = Directory.GetFiles(s_sourceFilesDir, "*", SearchOption.AllDirectories);

            FileInfo zipFile = new FileInfo(zipFileName);
            if (zipFile.Exists)
                zipFile.Delete();

            using (ZipOutputStream zipStream = new ZipOutputStream(File.Create(zipFileName)))
            {
                zipStream.SetLevel(9);
                zipStream.UseZip64 = UseZip64.Off;

                byte[] buffer = new byte[4096];

                foreach (string file in filenames)
                {
                    if (file.Contains("vshost") || file.Contains(".config"))
                        continue;

                    string entryName = String.Format("EVEMon{0}", file.Remove(0, s_sourceFilesDir.Length));
                    Console.WriteLine("Zipping {0}", entryName);
                    ZipEntry entry = new ZipEntry(entryName);

                    entry.DateTime = DateTime.Now;
                    zipStream.PutNextEntry(entry);

                    using (FileStream fs = File.OpenRead(file))
                    {
                        int sourceBytes;
                        do
                        {
                            sourceBytes = fs.Read(buffer, 0, buffer.Length);
                            zipStream.Write(buffer, 0, sourceBytes);
                        } while (sourceBytes > 0);
                    }
                }
                zipStream.Finish();
                zipStream.Close();
            }
        }

        private static void BuildInstaller()
        {
            try
            {
#if DEBUG
                string nsisScript = Path.Combine(s_projectDir, "bin\\x86\\Debug\\EVEMon Installer Script.nsi");
#else
                string nsisScript = Path.Combine(s_projectDir, "bin\\x86\\Release\\EVEMon Installer Script.nsi");
#endif

                string param =
                    String.Format("/DVERSION={0} \"/DOUTDIR={1}\" \"{2}\"", s_version, s_installerDir, nsisScript);

                Console.WriteLine("NSIS script : {0}", nsisScript);
                Console.WriteLine("Output directory : {0}", s_installerDir);

                ProcessStartInfo psi = new ProcessStartInfo(s_nsisExe, param);
                psi.WorkingDirectory = s_projectDir;
                psi.UseShellExecute = false;
                psi.RedirectStandardOutput = true;
                Process makensisProcess = Process.Start(psi);
                Console.WriteLine(makensisProcess.StandardOutput.ReadToEnd());
                makensisProcess.WaitForExit();
                int exitCode = makensisProcess.ExitCode;
                makensisProcess.Dispose();

                if (exitCode == 1)
                    MessageBox.Show("MakeNSIS exited with Errors");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}
