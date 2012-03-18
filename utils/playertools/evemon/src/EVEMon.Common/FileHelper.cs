using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;

namespace EVEMon.Common
{
    public static class FileHelper
    {
        private static Nullable<bool> s_removeReadOnlyAttributes;
        private static object s_removeReadOnlyAttributesLock = new object();    // Nullable<T> assignment is not atomic

        /// <summary>
        /// Opens a file, offering the user to retry if an exception occurs
        /// </summary>
        /// <param name="filename"></param>
        /// <returns></returns>
        public static Stream OpenRead(string filename, bool allowIgnore)
        {
            string normalizedFilename = filename;
            if (filename.StartsWith("file:///"))
            {
                normalizedFilename = filename.Remove(0, 8);
            }
            if (!File.Exists(normalizedFilename))
                return null;

            // While problems happen and the user ask to retry...
            while (true)
            {
                try
                {
                    return new MemoryStream(File.ReadAllBytes(normalizedFilename));
                }
                catch (UnauthorizedAccessException exc)
                {
                    ExceptionHandler.LogException(exc, true);

                    string message = exc.Message;
                    message += "\r\n\r\nEVEMon failed to read a file. You may have insufficient rights or a synchronization may be occuring. Choosing to " + (allowIgnore ? "abort" : "cancel") + " will make EVEMon quit.";
                    var result = MessageBox.Show(message, "Failed to read a file", 
                        (allowIgnore ? MessageBoxButtons.AbortRetryIgnore : MessageBoxButtons.RetryCancel), 
                        MessageBoxIcon.Error);

                    // On abort, we quit the application
                    if (result == DialogResult.Abort || result == DialogResult.Cancel)
                    {
                        Application.Exit();
                        return null;
                    }

                    // The loop will begin again if the users asked to retry
                    if (result == DialogResult.Ignore)
                        return null;
                }
            }
        }

        /// <summary>
        /// Overwrites a destination file with the temporary file populated with content using the provided callback.
        /// This method will take care of the readonly attributes, prompting the user to EVEMon removing them.
        /// Finally, should a <see cref="UnauthorizedAccessException"/> occurs, EVEMon will display a readable message for the end-user.
        /// </summary>
        /// <param name="destFileName">The destination file name, it does not have to already exist</param>
        /// <param name="writeContentFunc"></param>
        /// <returns>
        /// False if the provided callback returned false or 
        /// if the user denied to remove the read-only attribute or 
        /// if he didn't have the permissions to write the file;
        /// true otherwise.</returns>
        public static bool OverwriteOrWarnTheUser(string destFileName, Func<Stream, bool> writeContentFunc)
        {
            string tempFileName = Path.GetTempFileName();
            try
            {
                using (FileStream fs = new FileStream(tempFileName, FileMode.Open))
                {
                    if (!writeContentFunc(fs))
                        return false;
                }

                return OverwriteOrWarnTheUser(tempFileName, destFileName);
            }
            finally
            {
                File.Delete(tempFileName);
            }
        }

        /// <summary>
        /// Overwrites a destination file with the provided source file by copying this one.
        /// This method will take care of the readonly attributes, prompting the user to EVEMon removing them.
        /// Finally, should a <see cref="UnauthorizedAccessException"/> occurs, EVEMon will display a readable message for the end-user.
        /// </summary>
        /// <param name="srcFileName">The source file name.</param>
        /// <param name="destFileName">The destination file name, it does not have to already exist</param>
        /// <returns>False if the user denied to remove the read-only attribute or if he didn't have the permissions to write the file; true otherwise.</returns>
        public static bool OverwriteOrWarnTheUser(string srcFileName, string destFileName)
        {
            // While problems happen and the user ask to retry...
            while (true)
            {
                try
                {
                    var destFile = new FileInfo(destFileName);

                    // We need to make sure this file is not read-only
                    // If it is, this method will request the user the permission to automatically remove the readonly attributes
                    if (!EnsureWritable(destFile))
                        return false;

                    // Overwrite the file
                    File.Copy(srcFileName, destFileName, true);

                    // Ensures we didn't copied a read-only attribute, no permission required since the file has been overwritten
                    destFile.Refresh();
                    if (destFile.IsReadOnly)
                        destFile.IsReadOnly = false;

                    // Quit the loop
                    return true;
                }
                catch (UnauthorizedAccessException exc)
                {
                    ExceptionHandler.LogException(exc, true);

                    string message = exc.Message;
                    message += "\r\n\r\nEVEMon failed to save to a file. You may have insufficient rights or a synchronization may be occuring. Choosing to abort will make EVEMon quit.";
                    var result = MessageBox.Show(message, "Failed to write over a file", MessageBoxButtons.AbortRetryIgnore, MessageBoxIcon.Error);

                    // On abort, we quit the application
                    if (result == DialogResult.Abort)
                    {
                        Application.Exit();
                        return false;
                    }

                    // The loop will begin again if the users asked to retry
                    if (result == DialogResult.Ignore)
                        return false;
                }
            }
        }

        /// <summary>
        /// Ensures the given file is writable.
        /// </summary>
        /// <param name="file">The file to make writable.</param>
        /// <returns>False if file exists, is readonly and the user denied permission to make it writable; true otherwise.</returns>
        public static bool EnsureWritable(string filename)
        {
            var file = new FileInfo(filename);
            return EnsureWritable(file);
        }

        /// <summary>
        /// Ensures the given file is writable.
        /// </summary>
        /// <param name="file">The file to make writable.</param>
        /// <returns>False if file exists, is readonly and the user denied permission to make it writable; true otherwise.</returns>
        private static bool EnsureWritable(FileInfo file)
        {
            if (!file.Exists)
                return true;

            return TryMakeWritable(file);
        }

        /// <summary>
        /// Try to make a file writable, requesting the user the right to remove the readonly attributes the first time
        /// </summary>
        /// <param name="file">The file to make writable. It must exists but does not have to be read-only</param>
        /// <returns>False if the file was readonly and the user denied permission to make it writable; true otherwise</returns>
        private static bool TryMakeWritable(FileInfo file)
        {
            // Return true if the file can be written to.
            if (!file.IsReadOnly)
                return true;

            // Return false if we're not allowed to remove the read-only attribute
            if (!HasPermissionToRemoveReadOnlyAttributes)
                return false;

            // Remove the attribute
            file.IsReadOnly = false;

            return true;
        }

        /// <summary>
        /// Request from the use the permission for EVEMon to remove the read-only attributes on its own files
        /// </summary>
        /// <returns>True if the user granted the rights to make its files writable, false otherwise</returns>
        private static bool HasPermissionToRemoveReadOnlyAttributes
        {
            get
            {
                lock (s_removeReadOnlyAttributesLock)
                {
                    // We request the user to allow us to remove the attributes
                    if (!s_removeReadOnlyAttributes.HasValue)
                    {
                        // Prepare caption and text
                        string message = "EVEMon detected that some of its files are read-only, preventing it to save its datas.\r\n\r\n";
                        message += "Choosing YES will allow EVEMon to remove the read-only attributes on its own files (only).\r\n";
                        message += "Choosing NO will force EVEMon to continue without writing its files. This can cause unexpected behaviours.\r\n\r\n";
                        message += "Note : if you restart EVEMon and it still encounters read-only files, you will be prompted again.";

                        // Display the message box
                        var result = MessageBox.Show(message, "Allow EVEMon to make its files writable", MessageBoxButtons.YesNo, MessageBoxIcon.Error);

                        // User denied us the permission to make files writeable
                        if (result == DialogResult.No) s_removeReadOnlyAttributes = false;
                        else s_removeReadOnlyAttributes = true;
                    }

                    // Returns the permission granted by the user
                    return s_removeReadOnlyAttributes.Value;
                }
            }
        }
    }
}
