using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Resources;
using System.Windows.Forms;

using EVEMon.Common.Data;

namespace EVEMon.Common.Controls
{
    /// <summary>
    /// Displays an image for a given EveObject
    /// </summary>
    /// <remarks>
    /// Setting the PopUpEnabled property to true enables a pop-up
    /// window for EveObjects with a 256 x 256 image available, accessed
    /// via the user double-clicking the image. Image size must be
    /// set using the ImageSize property. The default Size property is
    /// overriden.
    /// </remarks>
    public partial class EveImage : UserControl
    {

        /// <summary>
        /// Holds configuration data for different image types
        /// </summary>
        private Dictionary<ImageType, ImageTypeData> m_ImageTypeAttributes;

        private bool m_popUpEnabled;
        private bool m_popUpActive;
        private EveImageSize m_imageSize;
        private EveImageSizeMode m_sizeMode;
        private Item m_item = null;


        #region Constructor

        /// <summary>
        /// Initialize the control
        /// </summary>
        /// <remarks>
        /// The default image size is 64 x 64, with the image pop-up enabled.
        /// </remarks>
        public EveImage()
        {
            InitializeComponent();
            SetImageTypeAttributes();
            ImageSize = EveImageSize.x64;
            PopUpEnabled = true;
            ShowBlankImage();
        }

        #endregion


        #region Public Properties

        /// <summary>
        /// Gets or sets the state of the pop-up ability.
        /// </summary>
        public bool PopUpEnabled
        {
            get
            {
                return m_popUpEnabled;
            }
            set
            {
                m_popUpEnabled = value;
            }
        }

        /// <summary>
        /// Gets or sets the item to display an image for.
        /// </summary>
        public Item EveItem
        {
            get
            {
                return m_item;
            }
            set
            {
                m_item = value;
                if (m_imageSize != EveImageSize.x0)
                    GetImage();
            }
        }

        /// <summary>
        /// Gets or sets the size of the image in eve parlance.
        /// </summary>
        public EveImageSize ImageSize
        {
            get
            {
                return m_imageSize;
            }
            set
            {
                m_imageSize = value;

                if (m_sizeMode == EveImageSizeMode.AutoSize)
                {
                    pbImage.Size = new Size((int)m_imageSize, (int)m_imageSize);
                    Size = pbImage.Size;
                }
            }
        }

        /// <summary>
        /// Gets or sets the size of the image in pixels.
        /// </summary>
        new public Size Size
        {
            get
            {
                return base.Size;
            }
            set
            {
                base.Size = (m_sizeMode == EveImageSizeMode.AutoSize ?
                    new Size((int)m_imageSize, (int)m_imageSize) : value);
            }
        }

        /// <summary>
        /// Gets or sets the size mode.
        /// </summary>
        /// <value>The size mode.</value>
        public  EveImageSizeMode SizeMode
        {
            get
            {
                return m_sizeMode;
            }
            set
            {
                m_sizeMode = value;

                switch (value)
                {
                    case EveImageSizeMode.Normal:
                        pbImage.SizeMode = PictureBoxSizeMode.Normal;
                        break;
                    case EveImageSizeMode.AutoSize:
                        pbImage.Size = new Size((int)m_imageSize, (int)m_imageSize);
                        Size = pbImage.Size;
                        pbImage.SizeMode = PictureBoxSizeMode.AutoSize;
                        break;
                    case EveImageSizeMode.StretchImage:
                        pbImage.SizeMode = PictureBoxSizeMode.StretchImage;
                        break;
                }
            }
        }

        #endregion


        #region Private Methods

        /// <summary>
        /// Builds the m_ImageTypeAttributes dictionary
        /// </summary>
        private void SetImageTypeAttributes()
        {
            ArrayList validSizes;
            m_ImageTypeAttributes = new Dictionary<ImageType, ImageTypeData>();
            
            // Ships
            validSizes = new ArrayList();
            validSizes.Add(EveImageSize.x32);
            validSizes.Add(EveImageSize.x64);
            validSizes.Add(EveImageSize.x128);
            validSizes.Add(EveImageSize.x256);
            m_ImageTypeAttributes.Add(ImageType.Ship,
                new ImageTypeData("Ships", "icons", ImageNameFrom.TypeID, validSizes));
            
            // Items
            validSizes = new ArrayList();
            validSizes.Add(EveImageSize.x16);
            validSizes.Add(EveImageSize.x32);
            validSizes.Add(EveImageSize.x64);
            validSizes.Add(EveImageSize.x128);
            m_ImageTypeAttributes.Add(ImageType.Item,
                new ImageTypeData("Items", "icons", ImageNameFrom.Icon, validSizes));
            
            // Drones
            validSizes = new ArrayList();
            validSizes.Add(EveImageSize.x32);
            validSizes.Add(EveImageSize.x64);
            validSizes.Add(EveImageSize.x128);
            validSizes.Add(EveImageSize.x256);
            m_ImageTypeAttributes.Add(ImageType.Drone,
                new ImageTypeData("Drones", "icons", ImageNameFrom.TypeID, validSizes));
            
            // Structures
            validSizes = new ArrayList();
            validSizes.Add(EveImageSize.x32);
            validSizes.Add(EveImageSize.x64);
            validSizes.Add(EveImageSize.x128);
            validSizes.Add(EveImageSize.x256);
            m_ImageTypeAttributes.Add(ImageType.Structure,
                new ImageTypeData("", "icons", ImageNameFrom.TypeID, validSizes));

            // Blueprints
            validSizes = new ArrayList();
            validSizes.Add(EveImageSize.x64);
            m_ImageTypeAttributes.Add(ImageType.Blueprint,
                new ImageTypeData("Blueprints", "icons", ImageNameFrom.TypeID, validSizes));
        }

        /// <summary>
        /// Gets the type of the image.
        /// </summary>
        /// <param name="item">The item.</param>
        /// <returns></returns>
        private ImageType GetImageType(Item item)
        {
            switch (item.Family)
            {
                case ItemFamily.Ship:
                    return ImageType.Ship;
                case ItemFamily.Drone:
                    return ImageType.Drone;
                case ItemFamily.StarbaseStructure:
                    return ImageType.Structure;
                case ItemFamily.Bpo:
                    return ImageType.Blueprint;
                default:
                    return ImageType.Item;
            }
        }

        /// <summary>
        /// Renders a BackColor square as a placeholder for the image
        /// </summary>
        private void ShowBlankImage()
        {
            Bitmap b = new Bitmap(pbImage.ClientSize.Width, pbImage.ClientSize.Height);
            using (Graphics g = Graphics.FromImage(b))
            {
                g.FillRectangle(new SolidBrush(BackColor), new Rectangle(0, 0, b.Width, b.Height));
            }
            pbImage.Image = b;
        }

        /// <summary>
        /// Retrieves image for the given EveObject
        /// </summary>
        private void GetImage()
        {
            // Reset flags and cursor
            m_popUpActive = false;
            toolTip.Active = false;
            pbImage.Cursor = Cursors.Default;

            if (m_item == null)
                return;

            ImageType imageType = GetImageType(m_item);
            ImageTypeData typeData = m_ImageTypeAttributes[imageType];

            // Only display an image if the correct size is available
            if (!typeData.validSizes.Contains(m_imageSize))
                return;

            // Enable pop up if required
            if (m_popUpEnabled && typeData.validSizes.Contains(EveImageSize.x256))
            {
                toolTip.Active = true;
                m_popUpActive = true;
                pbImage.Cursor = Cursors.Hand;
            }

            GetImageFromCCP(typeData);
        }

        /// <summary>
        /// Gets the image from CCP's image server (http://image.eveonline.com).
        /// </summary>
        /// <param name="typeData">The type data.</param>
        /// <returns></returns>
        private void GetImageFromCCP(ImageTypeData typeData)
        {
            string urlPath = "inventorytype";
            bool drawOverlayIcon = false;

            if ((int)m_imageSize > 64)
            {
                urlPath = "render";
                drawOverlayIcon = true;
            }

            string imageURL = String.Format(NetworkConstants.CCPIconsFromImageServer, urlPath, m_item.ID, (int)m_imageSize);

            ImageService.GetImageAsync(imageURL, true, (img) =>
                                        {
                                            GotImage(m_item.ID, img, drawOverlayIcon);
                                            
                                            if (img == null)
                                                GetImageFromAlternativeSource(typeData);
                                        });
        }

        /// <summary>
        /// Gets the image from an alternative source [local or (http://eve.no-ip.de)].
        /// </summary>
        /// <param name="typeData">The type data.</param>
        private void GetImageFromAlternativeSource(ImageTypeData typeData)
        {
            // Set file & pathname variables
            string eveSize = String.Format(CultureConstants.DefaultCulture, "{0}_{0}", (int)m_imageSize);

            string imageWebName;
            string imageResourceName;

            if (typeData.imageNameFrom == ImageNameFrom.TypeID)
            {
                imageWebName = m_item.ID.ToString();
                imageResourceName = String.Format("_{0}", imageWebName);
            }
            else
            {
                imageWebName = String.Format("icon{0}", m_item.Icon);
                imageResourceName = imageWebName;
            }

            // Try and get image from a local optional resources file (probably don't used anymore, not sure)
            string localResources = String.Format("{1}Resources{0}Optional{0}{2}{3}.resources",
                Path.DirectorySeparatorChar, AppDomain.CurrentDomain.BaseDirectory, typeData.localComponent, eveSize);

            // Try to get image from web (or local cache located in %APPDATA%\EVEMon if not found yet)
            if (!FetchImageResource(imageResourceName, localResources))
            {
                // Result should be like :
                // http://eve.no-ip.de/icons/32_32/icon22_08.png
                // http://eve.no-ip.de/icons/32_32/7538.png
                string imageURL = String.Format(NetworkConstants.CCPIcons, typeData.urlPath, eveSize, imageWebName);

                ImageService.GetImageAsync(imageURL, true, (img) => GotImage(m_item.ID, img, true));
            }
        }

        /// <summary>
        /// Find an image resource from local resource files
        /// </summary>
        /// <param name="imageResourceName">Resource name for the image</param>
        /// <param name="localResources">Local resource</param>
        /// <returns></returns>
        private bool FetchImageResource(string imageResourceName, string localResources)
        {
            if (!File.Exists(localResources))
                return false;

            try
            {
                IResourceReader basic = new ResourceReader(localResources);
                IDictionaryEnumerator basicx = basic.GetEnumerator();
                while (basicx.MoveNext())
                {
                    if (basicx.Key.ToString() == imageResourceName)
                    {
                        pbImage.Image = (Image)basicx.Value;
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                ExceptionHandler.LogException(ex, true);
            }
            return false;
        }

        /// <summary>
        /// Callback method for asynchronous web requests
        /// </summary>
        /// <param name="id">EveObject id for retrieved image</param>
        /// <param name="image">Image object retrieved</param>
        private void GotImage(long id, Image image, bool drawOverlayIcon)
        {
            // Only display the image if the id matches the current EveObject
            if (image != null && m_item.ID == id)
            {
                pbImage.Image = image;

                // Draw the overlay icon
                if (drawOverlayIcon)
                    DrawOverlayIcon();
            }
            else 
            {
                ShowBlankImage();
            }
        }

        /// <summary>
        /// Draws the overlay icon.
        /// </summary>
        private void DrawOverlayIcon()
        {
            var overlayIcon = new Bitmap(16, 16);
            switch (m_item.MetaGroup)
            {
                case ItemMetaGroup.T2:
                    overlayIcon = Properties.Resources.T2;
                    break;
                case ItemMetaGroup.T3:
                    overlayIcon = Properties.Resources.T3;
                    break;
                case ItemMetaGroup.Storyline:
                    overlayIcon = Properties.Resources.Storyline;
                    break;
                case ItemMetaGroup.Deadspace:
                    overlayIcon = Properties.Resources.Deadspace;
                    break;
                case ItemMetaGroup.Officer:
                    overlayIcon = Properties.Resources.Officer;
                    break;
                case ItemMetaGroup.Faction:
                    overlayIcon = Properties.Resources.Faction;
                    break;
            }

            using (Graphics graph = Graphics.FromImage(pbImage.Image))
            {
                graph.DrawImage(overlayIcon, 0, 0, (int)m_imageSize / 4, (int)m_imageSize / 4);
            }
        }

        #endregion


        #region Public Event Handlers

        /// <summary>
        /// Event handler for image double click
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void pbImage_DoubleClick(object sender, EventArgs e)
        {
            // Only display the pop up form if pop-ups are enabled and a suitable image can be retrieved
            if (!m_popUpActive)
                return;

            WindowsFactory<EveImagePopUp>.ShowUnique(()=> new EveImagePopUp(m_item));
        }
        #endregion


        #region Private Enumerations and Structs

        public enum EveImageSizeMode { Normal, AutoSize, StretchImage };

        /// <summary>
        /// Identifies the image type being handled
        /// </summary>
        private enum ImageType { Ship, Drone, Structure, Item, Blueprint, None }

        /// <summary>
        /// Indicates the source of the .png image name
        /// </summary>
        private enum ImageNameFrom { TypeID, Icon };

        /// <summary>
        /// Defines configuration data for a specific ImageType
        /// </summary>
        private struct ImageTypeData
        {
            public string localComponent;
            public string urlPath;
            public ImageNameFrom imageNameFrom;
            public ArrayList validSizes;

            public ImageTypeData(string local, string url, ImageNameFrom name, ArrayList sizes)
            {
                localComponent = local;
                urlPath = url;
                imageNameFrom = name;
                validSizes = sizes;
            }
        }

        #endregion
    }
}
