using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;
using EVEMon.Common;
using System.Collections;
using System.Drawing;
using EVEMon.Common.Controls;

namespace EVEMon
{
    /// <summary>
    /// Displays the About Window (Help -&gt; About) containing contrib,
    /// legal and version information about the application.
    /// </summary>
    public partial class AboutWindow : EVEMonForm
    {
        SortedList slPriority;
        SortedList slDevelopers;
        SortedList slOutput;
        public Font myFont;
        public Font myFontbold;

        /// <summary>
        /// Setup the list of developers and the standard font
        /// </summary>
        public AboutWindow()
        {
            InitializeComponent();
            devsList.SelectedIndexChanged += devsList_SelectedIndexChanged;
            myFont = FontFactory.GetDefaultFont(8.25f);
            myFontbold = FontFactory.GetDefaultFont(8.25f, FontStyle.Bold);
            EVEMonLabel.Font = FontFactory.GetDefaultFont(8.25F, FontStyle.Bold);
            DevContribLabel.Font = FontFactory.GetDefaultFont(8.25F, FontStyle.Bold);
            CredentialsLabels.Font = FontFactory.GetDefaultFont(8.25F, FontStyle.Bold);

            // list of headings
            slPriority = new SortedList();
            slPriority.Add("01", "Guru");
            slPriority.Add("02", "Guru (Retired)");
            slPriority.Add("03", "Developers");
            slPriority.Add("04", "Developers (Retired)");
            slPriority.Add("05", "Contributors");
            
            // list of developers by heading
            slDevelopers = new SortedList();
            
            // EVEMon Guru
            slDevelopers.Add("Jimi", "01");

            // Guru (Retired)
            slDevelopers.Add("Araan Sunn", "02");
            slDevelopers.Add("Six Anari", "02");
            slDevelopers.Add("Anders Chydenius", "02");
            slDevelopers.Add("Brad Stone", "02");
            slDevelopers.Add("Eewec Ourbyni", "02");
            slDevelopers.Add("Richard Slater", "02");
            slDevelopers.Add("Vehlin", "02");

            // Developers
            slDevelopers.Add("MrCue", "03");
            slDevelopers.Add("Tonto Auri", "03");

            // Developers (Retired)
            slDevelopers.Add("Collin Grady", "04");
            slDevelopers.Add("DCShadow", "04");
            slDevelopers.Add("DonQuiche", "04");
            slDevelopers.Add("Grauw", "04");
            slDevelopers.Add("Jalon Mevek", "04");
            slDevelopers.Add("Labogh", "04");
            slDevelopers.Add("romanl", "04");
            slDevelopers.Add("Safrax", "04");
            slDevelopers.Add("Stevil Knevil", "04");
            slDevelopers.Add("TheBelgarion", "04");

            // Contributors
            slDevelopers.Add("Abomb", "05");
            slDevelopers.Add("Adam Butt", "05");
            slDevelopers.Add("Aevum Decessus", "05");
            slDevelopers.Add("aliceturing", "05");
            slDevelopers.Add("aMUSiC", "05");
            slDevelopers.Add("Arengor", "05");
            slDevelopers.Add("ATGardner", "05");
            slDevelopers.Add("Barend", "05");
            slDevelopers.Add("bugusnot", "05");
            slDevelopers.Add("Candle","05");
            slDevelopers.Add("coeus", "05");
            slDevelopers.Add("CrazyMahone", "05");
            slDevelopers.Add("CyberTech", "05");
            slDevelopers.Add("Dariana", "05");
            slDevelopers.Add("Eviro", "05");
            slDevelopers.Add("exi", "05");
            slDevelopers.Add("FangVV", "05");
            slDevelopers.Add("Femaref", "05");
            slDevelopers.Add("Flash", "05");
            slDevelopers.Add("Galideeth", "05");
            slDevelopers.Add("gareth", "05");
            slDevelopers.Add("gavinl", "05");
            slDevelopers.Add("GoneWacko", "05");
            slDevelopers.Add("happyslinky", "05");
            slDevelopers.Add("jdread", "05");
            slDevelopers.Add("Jeff Zellner", "05");
            slDevelopers.Add("jthiesen", "05");            
            slDevelopers.Add("justinian", "05");
            slDevelopers.Add("Kingdud", "05");
            slDevelopers.Add("Kw4h", "05");
            slDevelopers.Add("Lexiica", "05");
            slDevelopers.Add("Master of Dice", "05");
            slDevelopers.Add("Maximilian Kernbach", "05");
            slDevelopers.Add("MaZ", "05");
            slDevelopers.Add("mexx24", "05");
            slDevelopers.Add("Michayel Lyon", "05");
            slDevelopers.Add("mintoko", "05");
            slDevelopers.Add("misterilla", "05");
            slDevelopers.Add("Moq", "05");
            slDevelopers.Add("morgangreenacre", "05");
            slDevelopers.Add("Namistai", "05");
            slDevelopers.Add("Nascent Nimbus", "05");
            slDevelopers.Add("NetMage", "05");
            slDevelopers.Add("Nagapito", "05");
            slDevelopers.Add("Nilyen", "05");
            slDevelopers.Add("Nimrel", "05");
            slDevelopers.Add("Niom", "05");
            slDevelopers.Add("Pharazon", "05");
            slDevelopers.Add("Phoenix Flames", "05");
            slDevelopers.Add("phorge", "05");
            slDevelopers.Add("Optica", "05");
            slDevelopers.Add("Risako", "05");
            slDevelopers.Add("Ruldar", "05");
            slDevelopers.Add("Safarian Lanar", "05");
            slDevelopers.Add("scoobyrich", "05");
            slDevelopers.Add("Sertan Deras", "05");
            slDevelopers.Add("shaver", "05");
            slDevelopers.Add("Shocky", "05");
            slDevelopers.Add("Shwehan Juanis", "05");
            slDevelopers.Add("skolima", "05");
            slDevelopers.Add("Spiff Nutter", "05");
            slDevelopers.Add("Subkahnshus", "05");
            slDevelopers.Add("The_Assimilator", "05");
            slDevelopers.Add("TheConstructor", "05");
            slDevelopers.Add("Trin", "05");
            slDevelopers.Add("vardoj", "05");
            slDevelopers.Add("Waste Land", "05");
            slDevelopers.Add("wrok", "05");
            slDevelopers.Add("xNomeda", "05");
            slDevelopers.Add("ykoehler", "05");
            slDevelopers.Add("Zarra Kri", "05");
            slDevelopers.Add("Zofu", "05");
    
            slOutput = new SortedList();
        }

        /// <summary>
        /// Prevents the user to select an item in the list.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void devsList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (devsList.SelectedItems.Count != 0)
            {
                devsList.SelectedItems.Clear();
            }
        }

        /// <summary>
        /// Populates and adds links to the various labels and list
        /// boxes on the form.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void AboutWindow_Load(object sender, EventArgs e)
        {
            Version currentVersion = Assembly.GetExecutingAssembly().GetName().Version;
            VersionLabel.Text = String.Format(CultureConstants.DefaultCulture, VersionLabel.Text, currentVersion.ToString());

            AddDevelopersToListView();
            AddDebugTag();

            AddLinkToLabel(ccpGamesLinkLabel, "CCP Games", "http://www.ccpgames.com/");
            AddLinkToLabel(battleclinicLinkLabel, "BattleClinic", "http://www.battleclinic.com/");
            AddLinkToLabel(eveCentralLinkLabel, "Eve-central", "http://www.eve-central.com/");
            AddLinkToLabel(eveDevLinkLabel, "EVEDev", "http://www.eve-dev.net/");
            AddLinkToLabel(googleDataLinkLabel, "Google.Data", "http://code.google.com/apis/gdata/client-cs.html");
            AddLinkToLabel(lironLeviLinkLabel, "Liron Levi", "http://www.codeproject.com/KB/cs/multipanelcontrol.aspx");
            AddLinkToLabel(stackOverflowLinkLabel, "Stack Overflow", "http://stackoverflow.com");
        }

        /// <summary>
        /// Little function to allow us to add links to a link label
        /// after the contents has been set, purely by the contained
        /// text
        /// </summary>
        /// <remarks>
        /// At present this function only works on the first instance
        /// of a string within the text property of the link label,
        /// further insances will be ignored.
        /// </remarks>
        /// <param name="label">LinkLabel to act upon</param>
        /// <param name="linkText">text to make a link</param>
        /// <param name="url">URL for the link to point to</param>
        private static void AddLinkToLabel(LinkLabel label, String linkText, String url)
        {
            int start = label.Text.IndexOf(linkText, StringComparison.Ordinal);
            int length = linkText.Length;

            label.Links.Add(start, length, url);
        }

        /// <summary>
        /// Adds " (Debug)" to the verison number if the build has
        /// the DEBUG conditional set.
        /// </summary>
        [Conditional("DEBUG")]
        private void AddDebugTag()
        {
            VersionLabel.Text = String.Format(CultureConstants.DefaultCulture, "{0} (Debug)", VersionLabel.Text);
        }

        /// <summary>
        /// Loops through the list of headings and developers and adds
        /// them to the list box.
        /// </summary>
        private void AddDevelopersToListView()
        {
            devsList.Columns.Add(new ColumnHeader());

            // Set up the list of developers.
            for (int i = 0; i < slPriority.Count; i++)
            {
                var group = new ListViewGroup(slPriority.GetByIndex(i).ToString());
                devsList.Groups.Add(group);

                for (int j = 0; j < slDevelopers.Count; j++)
                {
                    if (slPriority.GetKey(i) == slDevelopers.GetByIndex(j))
                    {
                        var item = new ListViewItem(slDevelopers.GetKey(j).ToString(), group);
                        devsList.Items.Add(item);
                    }
                }
            }

            devsList.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
        }


        /// <summary>
        /// Handles the Click event of the btnOk control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.EventArgs"/> instance containing the event data.</param>
        private void btnOk_Click(object sender, EventArgs e)
        {
            Close();
        }

        /// <summary>
        /// Handles the LinkClicked event of the llHomePage LinkLabel.
        /// Navigates to the EVEMon website in a browser.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.LinkLabelLinkClickedEventArgs"/> instance containing the event data.</param>
        private void llHomePage_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Util.OpenURL(NetworkConstants.EVEMonMainPage);
        }

        /// <summary>
        /// Handles the LinkClicked event of the IconSourceLinkLabel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="System.Windows.Forms.LinkLabelLinkClickedEventArgs"/> instance containing the event data.</param>
        private void LinkLabel_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            if (e.Link.LinkData.GetType() != typeof(String))
            {
                return;
            }

            try
            {
                Uri linkUri = new Uri(e.Link.LinkData.ToString());
                Util.OpenURL(linkUri.AbsoluteUri);
            }
            catch (UriFormatException ex)
            {
                // uri is malformed, never mind just ignore it
                ExceptionHandler.LogException(ex, true);
            }
        }
    }
}
