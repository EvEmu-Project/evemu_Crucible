using System;
using System.Collections.Generic;
using System.Text;
using EVEMon.Common.Scheduling;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using EVEMon.Common;

namespace EVEMon.Schedule
{
    public class ScheduleCalendar : CalendarControl
    {
        protected Font smallFont = null;

        public Font EntryFont
        {
            get { return smallFont; }
            set { smallFont = value; }
        }

        protected Color text_color = Color.White;
        public Color TextColor
        {
            get { return text_color; }
            set { text_color = value; }
        }

        protected Color blocking_color = Color.Red;
        public Color BlockingColor
        {
            get { return blocking_color; }
            set { blocking_color = value; }
        }

        protected Color single_color = Color.Blue;
        protected Color single_color2 = Color.LightBlue;
        public Color SingleColor
        {
            get { return single_color; }
            set { single_color = value; }
        }
        public Color SingleColor2
        {
            set { single_color2 = value; }
            get { return single_color2; }
        }

        protected Color recurring_color = Color.Green;
        protected Color recurring_color2 = Color.LightGreen;

        public Color RecurringColor
        {
            get { return recurring_color; }
            set { recurring_color = value; }
        }
        public Color RecurringColor2
        {
            get { return recurring_color2; }
            set { recurring_color2 = value; }
        }

        protected List<ScheduleEntry> entries = new List<ScheduleEntry>();
        public List<ScheduleEntry> Entries
        {
            get { return entries; }
        }

        public ScheduleCalendar() : base()
        {
            SetupDefaultResources();
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            smallFont.Dispose(); smallFont = null;
        }

        protected void SetupDefaultResources()
        {
            smallFont = FontFactory.GetDefaultFont(7.0f);
        }

        protected override void PaintMonthEntriesForDay(Graphics g, DateTime datetime, Rectangle cellRect)
        {
            List<ScheduleEntry> todays = new List<ScheduleEntry>();
            foreach (ScheduleEntry entry in entries)
            {
                if (entry.IsToday(datetime))
                {
                    todays.Add(entry);
                }
            }

            // Sort Todays Entries Alphabetically
            todays.Sort(new ScheduleEntryTitleComparer());

            Rectangle rect = cellRect;
            rect.X++;
            rect.Y += 14;
            rect.Width -= 1;
            rect.Height = 11;


            List<ScheduleEntry>.Enumerator e = todays.GetEnumerator();
            if (e.MoveNext())
            {
                while (e.Current != null)
                {
                    ScheduleEntry entry = e.Current;
                    int q = entry.Title.Length;

                    if (entry is SimpleScheduleEntry)
                    {
                        // Setup a nice Brush
                        Brush fillBrush = null;
                        if ((entry.Options & ScheduleEntryOptions.Blocking) != 0)
                        {
                            fillBrush = new LinearGradientBrush(new Point(rect.X, rect.Y), new Point(rect.X + rect.Width, rect.Y + rect.Height), blocking_color, single_color2);
                        }
                        else
                        {
                            fillBrush = new LinearGradientBrush(new Point(rect.X, rect.Y), new Point(rect.X + rect.Width, rect.Y + rect.Height), single_color, single_color2);
                        }

                        using (fillBrush)
                        {
                            // Check if the text fits
                            Size textsize = TextRenderer.MeasureText(entry.Title, smallFont);
                            if (textsize.Width <= rect.Width)
                            {
                                g.FillRectangle(fillBrush, rect);
                                TextRenderer.DrawText(g, entry.Title, smallFont, new Point(rect.X + 1, rect.Y), text_color);
                            }
                            else
                            {
                                // Make sure the text fits
                                string shorttext = entry.Title + "..";
                                for (int i = entry.Title.Length - 1; i > 4; i--)
                                {
                                    shorttext = entry.Title.Substring(0, i) + "..";
                                    textsize = TextRenderer.MeasureText(shorttext, smallFont);
                                    if (textsize.Width <= rect.Width)
                                    {
                                        break;
                                    }
                                }
                                g.FillRectangle(fillBrush, rect);
                                TextRenderer.DrawText(g, shorttext, smallFont, new Point(rect.X + 1, rect.Y), text_color);
                            }
                        }
                    }
                    else if (entry is RecurringScheduleEntry)
                    {
                        // Setup a nice Brush
                        Brush fillBrush = null;
                        if ((entry.Options & ScheduleEntryOptions.Blocking) != 0)
                        {
                            fillBrush = new LinearGradientBrush(new Point(rect.X, rect.Y), new Point(rect.X + rect.Width, rect.Y + rect.Height), blocking_color, recurring_color2);
                        }
                        else
                        {
                            fillBrush = new LinearGradientBrush(new Point(rect.X, rect.Y), new Point(rect.X + rect.Width, rect.Y + rect.Height), recurring_color, recurring_color2);
                        }

                        using (fillBrush)
                        {

                            Size textsize = TextRenderer.MeasureText(entry.Title, smallFont);
                            if (textsize.Width <= rect.Width)
                            {
                                g.FillRectangle(fillBrush, rect);
                                TextRenderer.DrawText(g, entry.Title, smallFont, new Point(rect.X + 1, rect.Y), text_color);
                            }
                            else
                            {
                                // Make sure the text fits
                                string shorttext = entry.Title + "..";
                                for (int i = entry.Title.Length - 1; i > 4; i--)
                                {
                                    shorttext = entry.Title.Substring(0, i) + "..";
                                    textsize = TextRenderer.MeasureText(shorttext, smallFont);
                                    if (textsize.Width <= rect.Width)
                                    {
                                        break;
                                    }
                                }
                                g.FillRectangle(fillBrush, rect);
                                TextRenderer.DrawText(g, shorttext, smallFont, new Point(rect.X + 1, rect.Y), text_color);
                            }
                        }
                    }

                    rect.Y += rect.Height + 1;

                    // Check if we have room for one more entry?
                    if (rect.Y + rect.Height > cellRect.Y + cellRect.Height)
                    {
                        // No, are there more entries?
                        if (e.MoveNext())
                        {
                            // Yes, Draw something to let the user know
                            int toomuch = rect.Y + rect.Height - (cellRect.Y + cellRect.Height);
                            rect.Height -= toomuch;

                            // Make sure LinearGradientBrush doesn't get into any troubles (Out Of Memory Because both points are at the same position)
                            if (rect.Height == 0) rect.Height = 1;


                            using (Brush brush = new LinearGradientBrush(new Point(rect.X, rect.Y), new Point(rect.X, rect.Y + rect.Height), Color.Gray, Color.LightGray))
                            {
                                g.FillRectangle(brush, rect);
                            }
                        }
                        break;
                    }
                    else
                    {
                        // Yes, we have more room
                        e.MoveNext();
                    }
                }
            }
        }

        private const int LEGEND_X = 5;
        private const int LEGEND_Y = 5;
        private const int LEGEND_WIDTH = 200;
        private const int LEGEND_HEIGHT = 40;
        private const int LEGEND_PADDING = 5;
        private const int LEGEND_BOX = 10;
        private const int LEGEND_SPACING_X = 110;
        private const int LEGEND_SPACING_Y = 18;

        protected override void OnPaint(PaintEventArgs args)
        {
            // Paint the Calendar
            base.OnPaint(args);

            // Paint some kind of Legend
            Graphics g = args.Graphics;

            Rectangle r = new Rectangle();
            r.X = LEGEND_X;
            r.Y = LEGEND_Y;
            r.Height = LEGEND_HEIGHT;
            r.Width = LEGEND_WIDTH;
            g.FillRectangle(Brushes.White, r);
            g.DrawRectangle(Pens.Black, r);

            r.X = LEGEND_X + LEGEND_PADDING;
            r.Y = LEGEND_Y + LEGEND_PADDING;
            r.Width = LEGEND_BOX * 2;
            r.Height = LEGEND_BOX;
            using (Brush b = new LinearGradientBrush(new Point(r.X, r.Y), new Point(r.X + r.Width, r.Y), single_color, single_color2))
            {
                g.FillRectangle(b, r);
                g.DrawRectangle(Pens.Black, r);
                TextRenderer.DrawText(g, "Single Entry", this.Font, new Point(r.X + r.Width + 2, r.Y), Color.Black);
            }

            r.Y += LEGEND_SPACING_Y;
            using (Brush b = new LinearGradientBrush(new Point(r.X, r.Y), new Point(r.X + r.Width, r.Y), recurring_color, recurring_color2))
            {
                g.FillRectangle(b, r);
                g.DrawRectangle(Pens.Black, r);
                TextRenderer.DrawText(g, "Recurring Entry", this.Font, new Point(r.X + r.Width + 2, r.Y), Color.Black);
            }

            r.Y = LEGEND_Y + LEGEND_PADDING;
            r.X += LEGEND_SPACING_X;
            using (Brush b = new SolidBrush(blocking_color))
            {
                g.FillRectangle(b, r);
                g.DrawRectangle(Pens.Black, r);
                TextRenderer.DrawText(g, "Blocked", this.Font, new Point(r.X + r.Width + 2, r.Y), Color.Black);
            }
        }
    }
}
