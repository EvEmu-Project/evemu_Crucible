using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Xml.Serialization;

namespace EVEMon.Common.SettingsObjects
{
    /// <summary>
    /// Represents a serializable version of a rectangle
    /// </summary>
    public sealed class SerializableRectangle
    {
        [XmlAttribute("left")]
        public int Left
        {
            get;
            set;
        }

        [XmlAttribute("top")]
        public int Top
        {
            get;
            set;
        }

        [XmlAttribute("width")]
        public int Width
        {
            get;
            set;
        }

        [XmlAttribute("height")]
        public int Height
        {
            get;
            set;
        }

        // Do not make the conversion operators explicit, there is a bug with XML serialization
        public static explicit operator Rectangle(SerializableRectangle rect)
        {
            return new Rectangle(rect.Left, rect.Top, rect.Width, rect.Height);
        }

        public static explicit operator SerializableRectangle(Rectangle rect)
        {
            return new SerializableRectangle { Left = rect.Left, Top = rect.Top, Width = rect.Width, Height = rect.Height };
        }

        internal SerializableRectangle Clone()
        {
            return (SerializableRectangle)MemberwiseClone();
        }
    }
}
