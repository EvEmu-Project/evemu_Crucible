using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace EVEMon.Common.Controls
{
    public partial class CharacterPortrait : UserControl
    {
        private long m_id = -1;
        private Character m_character;
        private bool m_updatingPortrait;
        private bool m_pendingUpdate;

        /// <summary>
        /// Constructor
        /// </summary>
        public CharacterPortrait()
        {
            InitializeComponent();
            pictureBox.Image = pictureBox.InitialImage;

            EveClient.CharacterPortraitChanged += EveClient_CharacterPortraitChanged;
            Disposed += OnDisposed;
        }

        /// <summary>
        /// Unsubscribe events on disposing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDisposed(object sender, EventArgs e)
        {
            EveClient.CharacterPortraitChanged -= EveClient_CharacterPortraitChanged;
            Disposed -= OnDisposed;
        }

        /// <summary>
        /// When the control is made visible, we check for a pending update.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            // If we previously delayed an update, we do it now.
            if (Visible && m_pendingUpdate)
                UpdateContent();

            base.OnVisibleChanged(e);
        }

        #region Properties
        /// <summary>
        /// Gets or sets the character ID. Also sets the <see cref="Character"/> property to <c>null</c>.
        /// </summary>
        public long CharacterID
        {
            get { return m_id; }
            set
            {
                if (m_id != value)
                {
                    m_id = value;
                    m_character = null;
                    UpdateContent();
                }
            }
        }

        /// <summary>
        /// Gets or sets the character. Also updates the <see cref="CharacterID"/> property.
        /// </summary>
        public Character Character
        {
            get { return m_character; }
            set
            {
                if (m_character != value)
                {
                    m_id = value.CharacterID;
                    m_character = value;
                    UpdateContent();
                }
            }
        }

        /// <summary>
        /// Gets the local portraits cache path for our character's GUID.
        /// </summary>
        /// <remarks>
        /// We're talking about the cache in %APPDATA%\cache. 
        /// This is different from the ImageService's hit cache (%APPDATA%\cache\image) or the game's portrait cache (in EVE Online folder)
        /// </remarks>
        public string EVEMonCachePortraitsPath
        {
            get
            {
                if (m_character == null)
                    return String.Empty;

                var cacheDir = String.Format(CultureConstants.DefaultCulture, "{1}{0}cache{0}portraits", Path.DirectorySeparatorChar, EveClient.EVEMonDataDir);
                if (!Directory.Exists(cacheDir))
                    Directory.CreateDirectory(cacheDir);

                return Path.Combine(cacheDir, String.Format("{0}.png", m_character.Guid));
            }
        }

        /// <summary>
        /// Gets or sets true when control or the character it is bound to is updating.
        /// </summary>
        private bool IsUpdating
        {
            get
            {
                if (m_updatingPortrait)
                    return true;

                if (m_character != null)
                    return m_character.IsUpdatingPortrait;
                
                return false;
            }
            set
            {
                m_updatingPortrait = value;

                if (m_character != null)
                    m_character.IsUpdatingPortrait = value;
            }
        }
        #endregion


        #region Default mechanism on character id change (portraits cache, then ImageService for CCP url)
        /// <summary>
        /// When the character ID changed... 
        /// <list type="bullet">
        /// <item>We check for a cached portrait in %APPDATA%\Cache.</item>
        /// <item>It if failed, we assemble the url for a CCP download and give it to ImageService.</item>
        /// <item>ImageService will first check its cache (%APPDATA%\Cache\Images), then download the url if no image was found in cache.</item>
        /// </list>
        /// </summary>
        /// <remarks>Note this method will first check the ImageService cache before to resort to download.</remarks>
        private void UpdateContent()
        {
            if (!Visible)
            {
                m_pendingUpdate = true;
                return;
            }
            m_pendingUpdate = false;

            // In safe mode, doesn't bother with the character portrait
            if (Settings.UI.SafeForWork)
            {
                pictureBox.Image = pictureBox.InitialImage;
                return;
            }

            // Try to retrieve the portrait from our portrait cache (%APPDATA%\Cache)
            var image = GetPortraitFromCache();
            if (image != null)
            {
                pictureBox.Image = image;
                return;
            }

            // The image does not exist in cache, we try to retrieve it from CCP
            pictureBox.Image = pictureBox.InitialImage;
            UpdateCharacterImageFromCCP();
        }

        /// <summary>
        /// Open the character portrait from the EVEMon cache
        /// </summary>
        /// <returns>The character portrait as an Image object</returns>
        private Image GetPortraitFromCache()
        {
            if (m_id <= 0) return null;

            string cacheFileName = EVEMonCachePortraitsPath;
            if (!File.Exists(cacheFileName))
                return null;

            try
            {
                // We need to load the data into a MemoryStream before
                // returning the image or GDI+ will lock the file for 
                // the lifetime of the image
                Image image;

                byte[] imageBytes = File.ReadAllBytes(cacheFileName);

                using (MemoryStream stream = new MemoryStream())
                {
                    stream.Write(imageBytes, 0, imageBytes.Length);
                    stream.Position = 0;

                    image = Image.FromStream(stream);
                }
                return image;
            }
            catch (Exception e)
            {
                ExceptionHandler.LogException(e, false);
                return null;
            }
        }

        /// <summary>
        /// Download the image from CCP...
        /// <list type="bullet">
        /// <item>We assemble the url for a CCP download and give it to ImageService.</item>
        /// <item>ImageService will first check its cache (%APPDATA%\Cache\Images), then download the url if no image was found in cache.</item>
        /// </list>
        /// </summary>
        private void UpdateCharacterImageFromCCP()
        {
            Cursor.Current = Cursors.WaitCursor;
            if (IsUpdating)
                return;

            IsUpdating = true;

            ImageService.GetCharacterImageAsync(m_id, new GetImageCallback(OnGotCharacterImageFromCCP));
        }

        /// <summary>
        /// We retrieve a portrait from the ImageService's cache or from the CCP url. We then save it to the portraits' cache (%APPDATA%\Cache).
        /// </summary>
        /// <param name="i">The retrieved image.</param>
        private void OnGotCharacterImageFromCCP(Image newImage)
        {
            // Restore cursor, then quit if download failed
            Cursor.Current = Cursors.Default;
            if (newImage == null)
            {
                IsUpdating = false;
                return;
            }

            // The image was retrieved, we save it to the cache
            SavePortraitToCache(newImage);
        }

        /// <summary>
        /// Save the specified image to the EVEMon cache as this character's portrait
        /// </summary>
        /// <param name="newImage">The new portrait image.</param>
        private void SavePortraitToCache(Image newImage)
        {
            // If control only has a character ID, we just update the picture box right now
            if (m_character == null)
            {
                pictureBox.Image = newImage;
                IsUpdating = false;
                return;
            }

            // Save to the portraits cache and notify we changed this character's portrait
            try
            {
                // Save the image to the portrait cache file
                FileHelper.OverwriteOrWarnTheUser(EVEMonCachePortraitsPath, fs =>
                {
                    newImage.Save(fs, ImageFormat.Png);
                    fs.Flush();
                    fs.Close();
                    return true;
                });

                // Notify the other controls we updated this portrait
                EveClient.OnCharacterPortraitChanged(m_character);
            }
            catch (Exception e)
            {
                // TODO : Add a tooltip here
                ExceptionHandler.LogException(e, false);
            }
            finally
            {
                // Release the updating flag
                IsUpdating = false;
            }
        }
        #endregion


        #region Mechanisms related to the game folder
        /// <summary>
        /// Download the image from the EVE cache (in EVE Online client installation folder)
        /// </summary>
        private void UpdateCharacterFromEVECache()
        {
            IsUpdating = true;
            try
            {
                // If we don't have the game's portraits cache already, prompt the user
                if (EveClient.EvePortraitCacheFolders == null || EveClient.EvePortraitCacheFolders.Length == 0)
                {
                    // Return if the user canceled
                    if (!ChangeEVEPortraitCache())
                        return;
                }

                // Now, search in the game folder all matching files 
                // (different resolutions are available for every character)
                try
                {
                    // Retrieve all files in the EVE cache directory which matches "<characterId>*"
                    List<FileInfo> filesInEveCache = new List<FileInfo>();
                    List<FileInfo> imageFilesInEveCache = new List<FileInfo>();
                    foreach (var evePortraitCacheFolder in EveClient.EvePortraitCacheFolders)
                    {
                        DirectoryInfo di = new DirectoryInfo(evePortraitCacheFolder);
                        filesInEveCache.AddRange(di.GetFiles(String.Format("{0}*", m_id)));
                        
                        // Look up for an image file and add it to the list
                        // (CCP changed image format in Incursion 1.1.0
                        // as part of new character portraits creator,
                        // so I added an image file check method to provide compatibility
                        // with all image formats (Jimi))
                        foreach (var file in filesInEveCache)
                        {
                            if (IsImageFile(file))
                                imageFilesInEveCache.Add(file);
                        }
                    }

                    // Displays an error message if none found.
                    if (imageFilesInEveCache.Count == 0)
                    {
                        StringBuilder message = new StringBuilder();

                        message.AppendFormat("No portraits for your character were found in the folder you selected.{0}{0}", Environment.NewLine);
                        message.AppendFormat("Ensure that you have checked the following:{0}", Environment.NewLine);
                        message.AppendFormat(" - You have logged into EVE with that characters' account.{0}", Environment.NewLine);
                        message.AppendFormat(" - You have selected a folder that contains EVE Portraits.{0}", Environment.NewLine);
                        message.AppendFormat("Your default EVE Portrait directory is:{1}{0}{1}", EveClient.EVEApplicationDataDir, Environment.NewLine);

                        MessageBox.Show(message.ToString(), "Portrait Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        
                        IsUpdating = false;
                        return;
                    }

                    // Search for the biggest portrait
                    int bestSize = 0;
                    string bestFile = String.Empty;
                    int charIDLength = m_id.ToString().Length;
                    foreach (FileInfo file in imageFilesInEveCache)
                    {
                        int sizeLength = (file.Name.Length - (file.Extension.Length + 1)) - charIDLength;
                        int imageSize = int.Parse(file.Name.Substring(charIDLength + 1, sizeLength));

                        if (imageSize > bestSize)
                        {
                            bestFile = file.FullName;
                            bestSize = imageSize;
                        }
                    }

                    // Open the largest image and save it
                    var image = Image.FromFile(bestFile);
                    SavePortraitToCache(image);
                }
                catch (Exception e)
                {
                    // TODO : Add a tooltip here
                    ExceptionHandler.LogException(e, false);
                }
            }
            finally
            {
                IsUpdating = false;
            }
        }

        /// <summary>
        /// Determines whether the specified file is an image file.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <returns>
        /// 	<c>true</c> if the specified file is an image file; otherwise, <c>false</c>.
        /// </returns>
        private static bool IsImageFile(FileInfo file)
        {
            try
            {
                var image = Image.FromFile(file.FullName);
            }
            catch (OutOfMemoryException)
            {
                return false;
            }
            return true;
        }

        /// <summary>
        /// Pops up a window for the user to select their EVE portrait cache folder.
        /// </summary>
        private static bool ChangeEVEPortraitCache()
        {
            using (EveFolderWindow f = new EveFolderWindow())
            {
                if (f.ShowDialog() == DialogResult.OK)
                {
                    EveClient.SetEvePortraitCacheFolder(f.EVEPortraitCacheFolder);
                    return true;
                }
            }

            return false;
        }
        #endregion


        #region Controls and global events handler
        /// <summary>
        /// Handles the CharacterPortraitChanged event of the EveClient control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="EVEMon.Common.CharacterChangedEventArgs"/> instance containing the event data.</param>
        private void EveClient_CharacterPortraitChanged(object sender, CharacterChangedEventArgs e)
        {
            if (!Visible)
            {
                m_pendingUpdate = true;
                return;
            }

            var image = GetPortraitFromCache();
            pictureBox.Image = (image != null ? image : pictureBox.InitialImage);
        }

        /// <summary>
        /// Handles the Click event of the miUpdatePicture control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void miUpdatePicture_Click(object sender, EventArgs e)
        {
            UpdateCharacterImageFromCCP();
        }

        /// <summary>
        /// Handles the Click event of the miUpdatePictureFromEVECache control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void miUpdatePictureFromEVECache_Click(object sender, EventArgs e)
        {
            UpdateCharacterFromEVECache();
        }

        /// <summary>
        /// Handles the Click event of the miSetEVEFolder control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void miSetEVEFolder_Click(object sender, EventArgs e)
        {
            ChangeEVEPortraitCache();
        }

        /// <summary>
        /// Handles the Click event of the pictureBox control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void pictureBox_Click(object sender, EventArgs e)
        {
            if (IsUpdating)
                return;

            cmsPictureOptions.Show(MousePosition);
        }
        #endregion
    }
}
