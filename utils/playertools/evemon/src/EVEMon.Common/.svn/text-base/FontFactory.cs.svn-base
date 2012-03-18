using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Globalization;

namespace EVEMon.Common
{
    /// <summary>
    /// A simple font helper factory.
    /// </summary>
    public static class FontFactory
    {
        /// <summary>
        /// Variable to store default font in
        /// </summary>
        /// <remarks>
        /// DefaultFont is cached for the following reasons:
        /// 1) improve the display time of the traybar popup.
        /// 2) remove the 50+ TrueType exceptions which SystemFonts.DefaultFont generates (at least on WinXP).
        /// </remarks>
        private static Font s_CachedDefaultFont = null;
        
        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <value>The default font.</value>
        private static Font DefaultFont
        {
            get
            {
                if (s_CachedDefaultFont == null)
                    s_CachedDefaultFont = SystemFonts.DefaultFont;
                return s_CachedDefaultFont;
            }
        }

        #region Helpers for default font
        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <returns></returns>
        public static Font GetDefaultFont()
        {
            return FontFactory.DefaultFont;
        }

        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <param name="emSize">Size of the em.</param>
        /// <returns></returns>
        public static Font GetDefaultFont(float emSize)
        {
            return FontFactory.GetDefaultFont(emSize, FontStyle.Regular);
        }

        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <param name="style">The style.</param>
        /// <returns></returns>
        public static Font GetDefaultFont(FontStyle style)
        {
            return FontFactory.GetDefaultFont(SystemFonts.DefaultFont.Size, style);
        }

        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <param name="emSize">Size of the em, in points.</param>
        /// <param name="style">The style.</param>
        /// <param name="unit">Units for the size : pixels, points, etc. Default should be point.</param>
        /// <returns></returns>
        public static Font GetDefaultFont(float emSize, FontStyle style)
        {
            return FontFactory.GetDefaultFont(emSize, style, GraphicsUnit.Point);
        }

        /// <summary>
        /// Gets the default font.
        /// </summary>
        /// <param name="emSize">Size of the em, in the given unit.</param>
        /// <param name="style">The style.</param>
        /// <param name="unit">Units for the size : pixels, points, etc. Default should be point.</param>
        /// <returns></returns>
        public static Font GetDefaultFont(float emSize, FontStyle style, GraphicsUnit unit)
        {
            return FontFactory.GetFont(FontFactory.DefaultFont.FontFamily.Name, emSize, style, unit);
        }
        #endregion


        #region Helpers for non-default font
        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="fontName">The font's name</param>
        /// <param name="emSize">The font's size, in points</param>
        /// <returns></returns>
        public static Font GetFont(string fontName, float emSize)
        {
            return FontFactory.GetFont(fontName, emSize, FontStyle.Regular, GraphicsUnit.Point);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="fontName">The font's name</param>
        /// <param name="style">The font's style</param>
        /// <returns></returns>
        public static Font GetFont(string fontName, FontStyle style)
        {
            return FontFactory.GetFont(fontName, FontFactory.DefaultFont.Size, style, FontFactory.DefaultFont.Unit);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="fontName">The font's name</param>
        /// <param name="emSize">The font's size, in the given unit</param>
        /// <param name="unit">The unit to use for the given size (points, pixels, etc)</param>
        /// <returns></returns>
        public static Font GetFont(string fontName, float emSize, GraphicsUnit unit)
        {
            return FontFactory.GetFont(fontName, emSize, FontStyle.Regular, unit);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="fontName">The font's name</param>
        /// <param name="emSize">The font's size, in points</param>
        /// <param name="style">The overriden style.</param>
        /// <returns></returns>
        public static Font GetFont(string fontName, float emSize, FontStyle style)
        {
            return FontFactory.GetFont(fontName, emSize, style, GraphicsUnit.Point);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="prototype">The font's prototype this font will be based on</param>
        /// <param name="style">The overriden style.</param>
        /// <returns></returns>
        public static Font GetFont(Font prototype, FontStyle style)
        {
            return FontFactory.GetFont(prototype.FontFamily.Name, prototype.Size, style, prototype.Unit);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="family">The font's family</param>
        /// <param name="emSize">Size of the font, in points.</param>
        /// <param name="style">The font's style.</param>
        /// <returns></returns>
        public static Font GetFont(FontFamily family, float emSize, FontStyle style)
        {
            return FontFactory.GetFont(family.Name, emSize, style, GraphicsUnit.Point);
        }

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="family">The font's family</param>
        /// <param name="emSize">Size of the font, in the provided unit.</param>
        /// <param name="style">The font's style.</param>
        /// <param name="unit">The unit to use for the given size (points, pixels, etc)</param>
        /// <returns></returns>
        public static Font GetFont(FontFamily family, float emSize, FontStyle style, GraphicsUnit unit)
        {
            return FontFactory.GetFont(family.Name, emSize, style, unit);
        }
        #endregion

        /// <summary>
        /// Gets the specified font.
        /// </summary>
        /// <param name="family">The font's family</param>
        /// <param name="emSize">Size of the font, in the given unit.</param>
        /// <param name="style">The font's style.</param>
        /// <param name="unit">Units for the size : pixels, points, etc. Default is point.</param>
        /// <returns></returns>
        public static Font GetFont(string familyName, float emSize, FontStyle style, GraphicsUnit unit)
        {
            FontFamily family = null;
            try
            {
                try
                {
                    try
                    {
                        // Inital try
                        family = new FontFamily(familyName); // Will accept anything and won't throw an error
                        return new Font(familyName, emSize, style, unit);
                    }
                    catch (ArgumentException e)
                    {
                        // First fallback : default family
                        ExceptionHandler.LogException(e, true);
                        family = FontFactory.DefaultFont.FontFamily;
                        return new Font(family, emSize, style, unit);
                    }
                }
                catch (ArgumentException e)
                {
                    // Second fallback : default family and style
                    ExceptionHandler.LogException(e, true);
                    style = FontFactory.DefaultFont.Style;
                    return new Font(family, emSize, style, unit);
                }
            }
            catch (ArgumentException e)
            {
                // Fourth fallback : all to default
                ExceptionHandler.LogException(e, true);
                return FontFactory.DefaultFont;
            }
        }
    }
}
