using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using EVEMon.ApiErrorHandling;
using EVEMon.Common;
using EVEMon.Common.Notifications;
using EVEMon.Common.SettingsObjects;

using CommonProperties = EVEMon.Common.Properties;

namespace EVEMon
{
    /// <summary>
    /// Displays a list of notifications
    /// </summary>
    public partial class NotificationList : UserControl
    {
        private readonly Color WarningColor = Color.LightGoldenrodYellow;
        private readonly Color ErrorColor = Color.LavenderBlush;
        private readonly Color InfoColor = Color.AliceBlue;

        private const int TextLeft = 20;
        private const int LeftPadding = 2;
        private const int RightPadding = 2;
        private const int IconDeletePositionFromRight = 14;
        private const int IconMagnifierPositionFromRight = 34;

        private List<Notification> m_notifications = new List<Notification>();
        private int m_hoveredIndex = -1;

        private bool m_pendingUpdate;

        /// <summary>
        /// Constructor.
        /// </summary>
        public NotificationList()
        {
            InitializeComponent();

            listBox.DrawItem += listBox_DrawItem;
            listBox.MouseMove += listBox_MouseMove;
            listBox.MouseDown += listBox_MouseDown;
            listBox.MouseLeave += listBox_MouseLeave;

            listBox.Font = FontFactory.GetFont("Tahoma", 8.25F, FontStyle.Regular, GraphicsUnit.Point);
        }

        /// <summary>
        /// On load, fils up the list for the design mode.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLoad(EventArgs e)
        {
            if (DesignMode || this.IsDesignModeHosted())
            {
                var list = new List<Notification>();
                var notification = new Notification(NotificationCategory.AccountNotInTraining, null)
                {
                    Priority = NotificationPriority.Information,
                    Description = "Some information"
                };
                
                list.Add(notification);

                notification = new Notification(NotificationCategory.AccountNotInTraining, null);
                notification.Priority = NotificationPriority.Warning;
                notification.Description = "Some warning";
                list.Add(notification);

                notification = new Notification(NotificationCategory.AccountNotInTraining, null);
                notification.Priority = NotificationPriority.Error;
                notification.Description = "Some error";
                list.Add(notification);

                Notifications = list;
            }

            base.OnLoad(e);
        }

        /// <summary>
        /// Gets or sets the displayed notifications.
        /// </summary>
        public IEnumerable<Notification> Notifications
        {
            get 
            {
                foreach (var notification in m_notifications)
                {
                    yield return notification;
                }
            }
            set
            {
                m_notifications.Clear();
                if (value != null)
                {
                    var notificationsToAdd = value.Where(x => Settings.Notifications.Categories[x.Category].ShowOnMainWindow);
                    m_notifications.AddRange(notificationsToAdd.ToArray().OrderBy(x => (int)x.Priority));
                }
                UpdateContent();
            }
        }

        /// <summary>
        /// Calculates the maximum text length of the list with given font
        /// </summary>
        /// <param name="font"></param>
        /// <returns></returns>
        private int CalculateMaxTextLength(Font font)
        {
            int maxTextLength = 0;

            foreach (var item in listBox.Items)
            {
                string text = item.ToString();
                Size textSize = TextRenderer.MeasureText(text, font);
                if (textSize.Width > maxTextLength)
                    maxTextLength = (int)textSize.Width;
            }

            return maxTextLength;
        }

        /// <summary>
        /// Calculates the font according to the notifications to display
        /// </summary>
        private void CalculateFontSize()
        {
            if (Width == 0)
                return;

            Font font = Font;
            var fontSize = font.Size;
            int magnifierIconSize;

            // Check for magnifier icon
            var itemWithDetails = listBox.Items.OfType<Notification>().FirstOrDefault(x => x.HasDetails);
            if (itemWithDetails != null)
                magnifierIconSize = IconMagnifierPositionFromRight;
            else
                magnifierIconSize = 0;

            // Calculates the available text space
            var availableTextSpace = Width - LeftPadding - TextLeft - magnifierIconSize - IconDeletePositionFromRight - RightPadding;

            // If any text length exceeds our bounds we decrease the font size
            while ((CalculateMaxTextLength(font) > availableTextSpace) && (fontSize > 6.5f))
            {
                fontSize -= 0.05f;
                font = FontFactory.GetFont("Tahoma", fontSize);
            }

            // If any text length fits better in our bounds we increase the font size
            while ((CalculateMaxTextLength(font) < availableTextSpace) && (fontSize < 8.25f))
            {
                fontSize += 0.05f;
                font = FontFactory.GetFont("Tahoma", fontSize);
            }

            Font = font;
        }

        /// <summary>
        /// Updates the content of the listbox.
        /// </summary>
        private void UpdateContent()
        {
            if (!Visible)
            {
                m_pendingUpdate = true;
                return;
            }

            m_pendingUpdate = false;

            listBox.BeginUpdate();
            try
            {
                listBox.Items.Clear();
                foreach (var notification in m_notifications)
                {
                    listBox.Items.Add(notification);
                }
            }
            finally
            {
                listBox.EndUpdate();
            }

            UpdateSize();
        }

        /// <summary>
        /// Updates the size of the control.
        /// </summary>
        private void UpdateSize()
        {
            Height = listBox.Items.Count * listBox.ItemHeight;
            Width = listBox.Width;
            CalculateFontSize();
            Invalidate();
        }

        /// <summary>
        /// Draws an item.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void listBox_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index == -1)
                return;

            var g = e.Graphics;

            var notification = listBox.Items[e.Index] as Notification;
            if (notification == null)
                return;

            // Retrieves the icon and background color
            Image icon;
            Color color;
            switch (notification.Priority)
            {
                case NotificationPriority.Error:
                    icon = CommonProperties.Resources.Error16;
                    color = ErrorColor;
                    break;
                case NotificationPriority.Warning:
                    icon = CommonProperties.Resources.Warning16;
                    color = WarningColor;
                    break;
                case NotificationPriority.Information:
                    icon = CommonProperties.Resources.Information16;
                    color = InfoColor;
                    break;
                default:
                    throw new NotImplementedException();
            }

            // Background
            using (var brush = new SolidBrush(color))
            {
                g.FillRectangle(brush, e.Bounds);
            }

            // Left icon
            g.DrawImageUnscaled(icon, new Point(e.Bounds.Left + LeftPadding, e.Bounds.Top + (listBox.ItemHeight - icon.Height) / 2));

            // Delete icon
            icon = (m_hoveredIndex == e.Index ? CommonProperties.Resources.CrossBlack : CommonProperties.Resources.CrossGray);
            g.DrawImageUnscaled(icon, new Point(e.Bounds.Right - IconDeletePositionFromRight, e.Bounds.Top + (listBox.ItemHeight - icon.Height) / 2));

            // Magnifier icon
            if (notification.HasDetails)
            {
                icon = CommonProperties.Resources.Magnifier;
                g.DrawImageUnscaled(icon, new Point(e.Bounds.Right - IconMagnifierPositionFromRight, e.Bounds.Top + (listBox.ItemHeight - icon.Height) / 2));
            }

            // Text
            using (var foreBrush = new SolidBrush(ForeColor))
            {
                string text = notification.ToString();
                var size = g.MeasureString(text, Font);
                g.DrawString(text, Font, foreBrush, new Point(e.Bounds.Left + TextLeft, e.Bounds.Top + (int)(listBox.ItemHeight - size.Height) / 2));
            }

            // Draw line on top
            using (var lineBrush = new SolidBrush(Color.Gray))
            {
                using (var pen = new Pen(lineBrush, 1.0f))
                {
                    g.DrawLine(pen, new Point(e.Bounds.Left, e.Bounds.Bottom - 1), new Point(e.Bounds.Right, e.Bounds.Bottom - 1));
                }
            }
        }

        /// <summary>
        /// When the mouse moves, detect the hovered index
        /// </summary>
        /// <param name="e"></param>
        void listBox_MouseMove(object sender, MouseEventArgs e)
        {
            int oldHoveredIndex = m_hoveredIndex;

            m_hoveredIndex = -1;
            for (int i = 0; i < listBox.Items.Count; i++)
            {
                var rect = GetDeleteIconRect(i);
                if (rect.Contains(e.Location))
                {
                    // Repaint the listbox if the previous index was different
                    m_hoveredIndex = i;
                    if (oldHoveredIndex != m_hoveredIndex)
                    {
                        listBox.Invalidate();
                        DisplayTooltip((Notification)listBox.Items[i]);
                    }
                    return;
                }
            }

            toolTip.Active = false;
        }

        /// <summary>
        /// When the user clicks, we need to detect whether it was on one of the buttons.
        /// </summary>
        /// <param name="e"></param>
        void listBox_MouseDown(object sender, MouseEventArgs e)
        {
            // First test whether the "delete" and "magnifier" icons have been clicked
            for (int i = 0; i < listBox.Items.Count; i++)
            {
                var rect = listBox.GetItemRectangle(i);                
                if (!rect.Contains(e.Location))
                    continue;

                var notification = listBox.Items[i] as Notification;

                // Did he click on the "magnifier" icon ?
                if (notification.HasDetails)
                {
                    rect = GetMagnifierIconRect(i);
                    if (rect.Contains(e.Location))
                    {
                        ShowDetails(notification);
                        return;
                    }
                }

                // Did he click on the "delete" icon or did a wheel-click?
                rect = GetDeleteIconRect(i);
                if (e.Button == MouseButtons.Middle || rect.Contains(e.Location))
                {
                    EveClient.Notifications.Invalidate(new NotificationInvalidationEventArgs(notification));
                    m_notifications.Remove(notification);
                    UpdateContent();
                    return;
                }
            }
        }

        /// <summary>
        /// Show the details for the given notification.
        /// </summary>
        /// <param name="notification"></param>
        private static void ShowDetails(Notification notification)
        {
            // API error ?
            if (notification is APIErrorNotification)
            {
                var window = WindowsFactory<APIErrorWindow>.ShowUnique();
                window.Notification = (APIErrorNotification)notification;
                return;
            }

            // Skills Completion ?
            if (notification is SkillCompletionNotification)
            {
                var window = WindowsFactory<SkillCompletionWindow>.ShowUnique();
                window.Notification = (SkillCompletionNotification)notification;
                return;
            }

            // Market orders ?
            if (notification is MarketOrdersNotification)
            {
                var ordersNotification = (MarketOrdersNotification)notification;
                var window = WindowsFactory<MarketOrdersWindow>.ShowUnique();
                window.Orders = ordersNotification.Orders;
                window.Columns = Settings.UI.MainWindow.MarketOrders.Columns;
                window.Grouping = MarketOrderGrouping.State;
                window.ShowIssuedFor = IssuedFor.All;
                return;
            }

            // Industry jobs ?
            if (notification is IndustryJobsNotification)
            {
                var jobsNotification = (IndustryJobsNotification)notification;
                var window = WindowsFactory<IndustryJobsWindow>.ShowUnique();
                window.Jobs = jobsNotification.Jobs;
                window.Columns = Settings.UI.MainWindow.IndustryJobs.Columns;
                window.Grouping = IndustryJobGrouping.State;
                window.ShowIssuedFor = IssuedFor.All;
                return;
            }
        }

        /// <summary>
        /// Displays the tooltip for the hovered item
        /// </summary>
        private void DisplayTooltip(Notification notification)
        {
            // No details ?
            if (!notification.HasDetails)
            {
                toolTip.Active = false;
                return;
            }

            // API error ?
            if (notification is APIErrorNotification)
            {
                var errorNotification = (APIErrorNotification)notification;
                toolTip.SetToolTip(listBox, errorNotification.Result.ErrorMessage);
                toolTip.Active = true;
                return;
            }

            // Skills Completion ?
            if (notification is SkillCompletionNotification)
            {
                var skillNotifications = (SkillCompletionNotification)notification;
                StringBuilder builder = new StringBuilder();
                foreach (var skill in skillNotifications.Skills.Reverse())
                {
                    builder.AppendFormat(CultureConstants.DefaultCulture,
                        "{0} {1} completed.", skill.SkillName, Skill.GetRomanForInt(skill.Level)).AppendLine();
                }
                toolTip.SetToolTip(listBox, builder.ToString());    
                toolTip.Active = true;
                return;
            }

            // Market orders ?
            if (notification is MarketOrdersNotification)
            {
                var ordersNotification = (MarketOrdersNotification)notification;

                StringBuilder builder = new StringBuilder();
                foreach (var orderGroup in ordersNotification.Orders.GroupBy(x => x.State))
                {
                    if (builder.Length != 0)
                        builder.AppendLine();
                    builder.AppendLine(orderGroup.Key.GetHeader());

                    foreach (var order in orderGroup)
                    {
                        if (order.Item == null)
                            continue;

                        var format = AbbreviationFormat.AbbreviationSymbols;

                        // Expired :    12k/15k invulnerability fields at Pator V - Tech School
                        // Fulfilled :  15k invulnerability fields at Pator V - Tech School
                        if (order.State == OrderState.Expired)
                            builder.Append(MarketOrder.Format(order.RemainingVolume, format)).Append("/");
                        
                        builder.Append(MarketOrder.Format(order.InitialVolume, format)).Append(" ");
                        builder.Append(order.Item.Name).Append(" at ");
                        builder.AppendLine(order.Station.Name);
                    }
                }
                toolTip.SetToolTip(listBox, builder.ToString());
                toolTip.Active = true;
                return;
            }

            // Industry jobs ?
            if (notification is IndustryJobsNotification)
            {
                var jobsNotification = (IndustryJobsNotification)notification;

                StringBuilder builder = new StringBuilder();
                foreach (var job in jobsNotification.Jobs)
                {
                    if (job.InstalledItem == null)
                        continue;

                    builder.Append(job.InstalledItem.Name).Append(" at ");
                    builder.AppendFormat(CultureConstants.DefaultCulture, "{0} > {1}", job.SolarSystem.Name, job.Installation).AppendLine();
                }
                toolTip.SetToolTip(listBox, builder.ToString());
                toolTip.Active = true;
                return;
            }
        }

        /// <summary>
        /// When the mouse leaves the control, we "unhover" the delete icon.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void listBox_MouseLeave(object sender, EventArgs e)
        {
            if (m_hoveredIndex != -1)
            {
                m_hoveredIndex = -1;
                listBox.Invalidate();
            }
        }

        /// <summary>
        /// Gets the rectangle of the "magnifier" icon for the listbox item at the given index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        private Rectangle GetMagnifierIconRect(int index)
        {
            var rect = listBox.GetItemRectangle(index);
            var icon = CommonProperties.Resources.Magnifier;
            var yOffset = (rect.Height - icon.Height) / 2;
            var magnifierIconRect = new Rectangle(rect.Right - IconMagnifierPositionFromRight, rect.Top + yOffset, icon.Width, icon.Height);
            magnifierIconRect.Inflate(2, 8);
            return magnifierIconRect;
        }

        /// <summary>
        /// Gets the rectangle of the "delete" icon for the listbox item at the given index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        private Rectangle GetDeleteIconRect(int index)
        {
            var rect = listBox.GetItemRectangle(index);
            var icon = CommonProperties.Resources.CrossBlack;
            var yOffset = (rect.Height - icon.Height) / 2;
            var deleteIconRect = new Rectangle(rect.Right - IconDeletePositionFromRight, rect.Top + yOffset, icon.Width, icon.Height);
            deleteIconRect.Inflate(2, 8);
            return deleteIconRect;
        }

        /// <summary>
        /// When the control becomes visible again, we check whether there was an update pending.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnVisibleChanged(EventArgs e)
        {
            if (Visible && m_pendingUpdate)
                UpdateContent();

            base.OnVisibleChanged(e);
        }
    }
}
