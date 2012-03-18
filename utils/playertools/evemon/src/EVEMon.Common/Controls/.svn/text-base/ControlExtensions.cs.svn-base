using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace System.Windows.Forms
{
    public static class ControlExtensions
    {
        /// <summary>
        /// Recursively checks among the container anecestry line whether one of the container is in design mode.
        /// This is done because <see cref="Control.DesignMode"/> is false when it is not the control directly edited in the designer, 
        /// like when it is a component of a form or another control.
        /// </summary>
        /// <param name="control"></param>
        /// <returns></returns>
        public static bool IsDesignModeHosted(this Control control)
        {
            while (control != null)
            {
                if (control.Site != null && control.Site.DesignMode) return true;
                control = control.Parent;
            }
            return false;
        }
    }
}
