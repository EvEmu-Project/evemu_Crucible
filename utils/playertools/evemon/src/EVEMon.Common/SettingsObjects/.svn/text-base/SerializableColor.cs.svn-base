using System;
using System.Drawing;
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Represents a color in the settings
    /// </summary>
    public sealed class SerializableColor
    {
        public SerializableColor()
        {
            A = 255;
        }

        [XmlAttribute]
        public byte A
        {
            get;
            set;
        }

        [XmlAttribute]
        public byte R
        {
            get;
            set;
        }

        [XmlAttribute]
        public byte G
        {
            get;
            set;
        }

        [XmlAttribute]
        public byte B
        {
            get;
            set;
        }

        #region Implciit conversion operators with System.Drawing.Color
        // Do not make the operators implicit, there is a bug with XML serialization.
        public static explicit operator Color(SerializableColor src)
        {
            return Color.FromArgb(src.A, src.R, src.G, src.B);
        }

        public static explicit operator SerializableColor(Color src)
        {
            return new SerializableColor { A = src.A, R = src.R, G = src.G, B = src.B };
        }
        #endregion

        internal SerializableColor Clone()
        {
            return (SerializableColor)MemberwiseClone();
        }
    }
}
