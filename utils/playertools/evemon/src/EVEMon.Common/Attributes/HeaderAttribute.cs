using System;
using System.Collections.Generic;
using System.Text;

namespace EVEMon.Common.Attributes
{
    public sealed class HeaderAttribute : Attribute
    {
        public HeaderAttribute(string header)
        {
            this.Header = header;
        }

        public string Header
        {
            get;
            set;
        }
    }
}
