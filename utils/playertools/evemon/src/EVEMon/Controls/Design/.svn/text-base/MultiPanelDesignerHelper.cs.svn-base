using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;
using System.ComponentModel.Design;
using System.Windows.Forms;

namespace EVEMon.Controls.Design
{
    public static class MultiPanelDesignerHelper
    {
        /// <summary>
        /// Gets the collection of verbs displayed in the top right menu of the designer.
        /// </summary>
        /// <param name="panel"></param>
        /// <returns></returns>
        public static DesignerVerbCollection GetDesignerVerbs(IDesignerHost host, MultiPanel panel)
        {
            var verbs = new DesignerVerbCollection();
            verbs.Add(new DesignerVerb("Add Page", (sender, args) => AddPage(host, panel)));
            verbs.Add(new DesignerVerb("Remove Page", (sender, args) => RemovePage(host, panel)));

            foreach (MultiPanelPage page in panel.Controls)
            {
                var pageCopy = page;
                verbs.Add(new DesignerVerb("Select \"" + page.Text + "\"", (sender, args) => SelectPage(host, panel, pageCopy)));
            }

            return verbs;
        }

        /// <summary>
        /// Event handler for the "Add Page" verb.
        /// </summary>
        /// <param name="panel"></param>
        private static void AddPage(IDesignerHost dh, MultiPanel panel)
        {
            DesignerTransaction dt = dh.CreateTransaction("Added new page");
            MultiPanelPage oldSelectedPage = panel.SelectedPage;

            // Gets a free name
            int i = 1;
            while (panel.Controls.Cast<Control>().Any(x => x.Name == "Page" + i)) i++;
            var name = "Page" + i;

            // Creates the page
            MultiPanelPage newPage = dh.CreateComponent(typeof(MultiPanelPage), name) as MultiPanelPage;
            newPage.Text = name;
            panel.Controls.Add(newPage);

            // Update selection
            panel.SelectedPage = newPage;

            dt.Commit();
        }

        /// <summary>
        /// Event handler for the "Remove Tab" verb.
        /// </summary>
        /// <param name="panel"></param>
        private static void RemovePage(IDesignerHost dh, MultiPanel panel)
        {
            MultiPanelPage page = panel.SelectedPage;
            if (page == null) return;

            DesignerTransaction dt = dh.CreateTransaction("Removed page");

            panel.Controls.Remove(page);
            dh.DestroyComponent(page);

            if (panel.Controls.Count > 0) panel.SelectedPage = (MultiPanelPage)panel.Controls[0];
            else panel.SelectedPage = null;

            dt.Commit();
        }

        /// <summary>
        /// Event handler for the "Select X page" handler.
        /// </summary>
        /// <param name="panel"></param>
        /// <param name="page"></param>
        /// <returns></returns>
        private static void SelectPage(IDesignerHost dh, MultiPanel panel, MultiPanelPage page)
        {
            DesignerTransaction dt = dh.CreateTransaction("Selected page");
            MultiPanelPage oldSelectedPage = panel.SelectedPage;

            panel.SelectedPage = page;
            dt.Commit();
        }

    }
}
