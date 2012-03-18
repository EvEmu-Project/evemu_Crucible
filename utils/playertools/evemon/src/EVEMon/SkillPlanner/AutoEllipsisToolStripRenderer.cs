using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon.SkillPlanner
{
    /// <summary>
    /// Renders the text of a ToolStripLabel with an Ellipsis if it
    /// overflows the available space.
    /// </summary>
    /// <remarks>
    /// For this render to work correctly the ToolStripStatusLabel
    /// Spring property should be set to True, set the TextAlign
    /// property to MiddleLeft to approximate the functionality of the
    /// standard ToolStripStatusLabel.
    /// 
    /// This class was adapted from the code on Joel on Software:
    ///   http://discuss.joelonsoftware.com/default.asp?dotnet.12.597246.5
    /// </remarks>
    public class AutoEllipsisToolStripRenderer : ToolStripSystemRenderer
    {
        protected override void OnRenderItemText(ToolStripItemTextRenderEventArgs e)
        {
            ToolStripStatusLabel label = e.Item as ToolStripStatusLabel;

            // It's not a label, panic, wait no; just let the SystemRenderer do it's thing
            if (label == null)
            {
                base.OnRenderItemText(e);
                return;
            }

            // If we are rendering a link we don't support AutoEllipsie
            if (label.IsLink)
            {
                base.OnRenderItemText(e);
                return;
            }

            TextRenderer.DrawText(e.Graphics,
                label.Text,
                label.Font,
                e.TextRectangle,
                label.ForeColor,
                TextFormatFlags.EndEllipsis);
        }
    }
}
