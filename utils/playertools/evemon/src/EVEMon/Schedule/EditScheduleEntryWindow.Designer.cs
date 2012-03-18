namespace EVEMon.Schedule
{
    partial class EditScheduleEntryWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.pnlRecurring = new System.Windows.Forms.Panel();
            this.pnlRecurWeekly = new System.Windows.Forms.Panel();
            this.label13 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.nudWeeklyFrequency = new System.Windows.Forms.NumericUpDown();
            this.btnRecurringNoStartDate = new System.Windows.Forms.Button();
            this.btnRecurringNoEndDate = new System.Windows.Forms.Button();
            this.tbRecurringTimeTo = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.tbRecurringTimeFrom = new System.Windows.Forms.TextBox();
            this.pnlRecurMonthly = new System.Windows.Forms.Panel();
            this.label8 = new System.Windows.Forms.Label();
            this.cbRecurOnOverflow = new System.Windows.Forms.ComboBox();
            this.label7 = new System.Windows.Forms.Label();
            this.nudRecurDayOfMonth = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.cbRecurringFrequency = new System.Windows.Forms.ComboBox();
            this.btnRecurringEndDateChoose = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.tbRecurringEndDate = new System.Windows.Forms.TextBox();
            this.btnRecurringStartDateChoose = new System.Windows.Forms.Button();
            this.label4 = new System.Windows.Forms.Label();
            this.tbRecurringStartDate = new System.Windows.Forms.TextBox();
            this.rbRecurring = new System.Windows.Forms.RadioButton();
            this.pnlOneTime = new System.Windows.Forms.Panel();
            this.label12 = new System.Windows.Forms.Label();
            this.tbOneTimeEndDate = new System.Windows.Forms.TextBox();
            this.btnOneTimeEndDateChoose = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.tbOneTimeEndTime = new System.Windows.Forms.TextBox();
            this.tbOneTimeStartDate = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.btnOneTimeStartDateChoose = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.tbOneTimeStartTime = new System.Windows.Forms.TextBox();
            this.rbOneTime = new System.Windows.Forms.RadioButton();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.cbUseEVETime = new System.Windows.Forms.CheckBox();
            this.cbSilent = new System.Windows.Forms.CheckBox();
            this.cbBlocking = new System.Windows.Forms.CheckBox();
            this.tbTitle = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.pnlRecurring.SuspendLayout();
            this.pnlRecurWeekly.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nudWeeklyFrequency)).BeginInit();
            this.pnlRecurMonthly.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nudRecurDayOfMonth)).BeginInit();
            this.pnlOneTime.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.pnlRecurring);
            this.groupBox1.Controls.Add(this.rbRecurring);
            this.groupBox1.Controls.Add(this.pnlOneTime);
            this.groupBox1.Controls.Add(this.rbOneTime);
            this.groupBox1.Location = new System.Drawing.Point(12, 135);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(377, 370);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Date/Time Setting";
            // 
            // pnlRecurring
            // 
            this.pnlRecurring.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.pnlRecurring.Controls.Add(this.pnlRecurWeekly);
            this.pnlRecurring.Controls.Add(this.btnRecurringNoStartDate);
            this.pnlRecurring.Controls.Add(this.btnRecurringNoEndDate);
            this.pnlRecurring.Controls.Add(this.tbRecurringTimeTo);
            this.pnlRecurring.Controls.Add(this.label9);
            this.pnlRecurring.Controls.Add(this.label10);
            this.pnlRecurring.Controls.Add(this.tbRecurringTimeFrom);
            this.pnlRecurring.Controls.Add(this.pnlRecurMonthly);
            this.pnlRecurring.Controls.Add(this.label6);
            this.pnlRecurring.Controls.Add(this.cbRecurringFrequency);
            this.pnlRecurring.Controls.Add(this.btnRecurringEndDateChoose);
            this.pnlRecurring.Controls.Add(this.label5);
            this.pnlRecurring.Controls.Add(this.tbRecurringEndDate);
            this.pnlRecurring.Controls.Add(this.btnRecurringStartDateChoose);
            this.pnlRecurring.Controls.Add(this.label4);
            this.pnlRecurring.Controls.Add(this.tbRecurringStartDate);
            this.pnlRecurring.Enabled = false;
            this.pnlRecurring.Location = new System.Drawing.Point(37, 192);
            this.pnlRecurring.Name = "pnlRecurring";
            this.pnlRecurring.Size = new System.Drawing.Size(334, 168);
            this.pnlRecurring.TabIndex = 10;
            // 
            // pnlRecurWeekly
            // 
            this.pnlRecurWeekly.Controls.Add(this.label13);
            this.pnlRecurWeekly.Controls.Add(this.label14);
            this.pnlRecurWeekly.Controls.Add(this.nudWeeklyFrequency);
            this.pnlRecurWeekly.Location = new System.Drawing.Point(180, 51);
            this.pnlRecurWeekly.Margin = new System.Windows.Forms.Padding(0);
            this.pnlRecurWeekly.Name = "pnlRecurWeekly";
            this.pnlRecurWeekly.Size = new System.Drawing.Size(139, 27);
            this.pnlRecurWeekly.TabIndex = 21;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(3, 5);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(39, 13);
            this.label13.TabIndex = 18;
            this.label13.Text = "every:";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(89, 6);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(37, 13);
            this.label14.TabIndex = 20;
            this.label14.Text = "weeks";
            // 
            // nUDweeklyfrequency
            // 
            this.nudWeeklyFrequency.Location = new System.Drawing.Point(48, 3);
            this.nudWeeklyFrequency.Maximum = new decimal(new int[] {
            52,
            0,
            0,
            0});
            this.nudWeeklyFrequency.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nudWeeklyFrequency.Name = "nUDweeklyfrequency";
            this.nudWeeklyFrequency.Size = new System.Drawing.Size(35, 21);
            this.nudWeeklyFrequency.TabIndex = 19;
            this.nudWeeklyFrequency.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // btnRecurringNoStartDate
            // 
            this.btnRecurringNoStartDate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRecurringNoStartDate.Location = new System.Drawing.Point(286, 0);
            this.btnRecurringNoStartDate.Name = "btnRecurringNoStartDate";
            this.btnRecurringNoStartDate.Size = new System.Drawing.Size(48, 23);
            this.btnRecurringNoStartDate.TabIndex = 2;
            this.btnRecurringNoStartDate.Text = "None";
            this.btnRecurringNoStartDate.UseVisualStyleBackColor = true;
            this.btnRecurringNoStartDate.Click += new System.EventHandler(this.btnRecurringNoStartDate_Click);
            // 
            // btnRecurringNoEndDate
            // 
            this.btnRecurringNoEndDate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRecurringNoEndDate.Location = new System.Drawing.Point(286, 27);
            this.btnRecurringNoEndDate.Name = "btnRecurringNoEndDate";
            this.btnRecurringNoEndDate.Size = new System.Drawing.Size(48, 23);
            this.btnRecurringNoEndDate.TabIndex = 5;
            this.btnRecurringNoEndDate.Text = "None";
            this.btnRecurringNoEndDate.UseVisualStyleBackColor = true;
            this.btnRecurringNoEndDate.Click += new System.EventHandler(this.btnRecurringNoEndDate_Click);
            // 
            // tbRecurringTimeTo
            // 
            this.tbRecurringTimeTo.Location = new System.Drawing.Point(162, 144);
            this.tbRecurringTimeTo.Name = "tbRecurringTimeTo";
            this.tbRecurringTimeTo.Size = new System.Drawing.Size(64, 21);
            this.tbRecurringTimeTo.TabIndex = 8;
            this.tbRecurringTimeTo.TextChanged += new System.EventHandler(this.tbRecurringTimeTo_TextChanged);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(133, 147);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(23, 13);
            this.label9.TabIndex = 17;
            this.label9.Text = "To:";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(-3, 147);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(60, 13);
            this.label10.TabIndex = 16;
            this.label10.Text = "Time From:";
            // 
            // tbRecurringTimeFrom
            // 
            this.tbRecurringTimeFrom.Location = new System.Drawing.Point(63, 144);
            this.tbRecurringTimeFrom.Name = "tbRecurringTimeFrom";
            this.tbRecurringTimeFrom.Size = new System.Drawing.Size(64, 21);
            this.tbRecurringTimeFrom.TabIndex = 7;
            this.tbRecurringTimeFrom.TextChanged += new System.EventHandler(this.tbRecurringTimeFrom_TextChanged);
            // 
            // pnlRecurMonthly
            // 
            this.pnlRecurMonthly.AutoSize = true;
            this.pnlRecurMonthly.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.pnlRecurMonthly.Controls.Add(this.label8);
            this.pnlRecurMonthly.Controls.Add(this.cbRecurOnOverflow);
            this.pnlRecurMonthly.Controls.Add(this.label7);
            this.pnlRecurMonthly.Controls.Add(this.nudRecurDayOfMonth);
            this.pnlRecurMonthly.Enabled = false;
            this.pnlRecurMonthly.Location = new System.Drawing.Point(64, 81);
            this.pnlRecurMonthly.Name = "pnlRecurMonthly";
            this.pnlRecurMonthly.Size = new System.Drawing.Size(188, 51);
            this.pnlRecurMonthly.TabIndex = 14;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(-3, 30);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(70, 13);
            this.label8.TabIndex = 3;
            this.label8.Text = "On overflow:";
            // 
            // cbRecurOnOverflow
            // 
            this.cbRecurOnOverflow.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.cbRecurOnOverflow.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbRecurOnOverflow.FormattingEnabled = true;
            this.cbRecurOnOverflow.Items.AddRange(new object[] {
            "Skip",
            "Overlap Forward",
            "Clip Backward"});
            this.cbRecurOnOverflow.Location = new System.Drawing.Point(79, 27);
            this.cbRecurOnOverflow.Name = "cbRecurOnOverflow";
            this.cbRecurOnOverflow.Size = new System.Drawing.Size(109, 21);
            this.cbRecurOnOverflow.TabIndex = 1;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(-3, 2);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(76, 13);
            this.label7.TabIndex = 1;
            this.label7.Text = "Day of Month:";
            // 
            // nudRecurDayOfMonth
            // 
            this.nudRecurDayOfMonth.Location = new System.Drawing.Point(79, 0);
            this.nudRecurDayOfMonth.Maximum = new decimal(new int[] {
            31,
            0,
            0,
            0});
            this.nudRecurDayOfMonth.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.nudRecurDayOfMonth.Name = "nudRecurDayOfMonth";
            this.nudRecurDayOfMonth.Size = new System.Drawing.Size(49, 21);
            this.nudRecurDayOfMonth.TabIndex = 0;
            this.nudRecurDayOfMonth.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(-3, 57);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(62, 13);
            this.label6.TabIndex = 13;
            this.label6.Text = "Frequency:";
            // 
            // cbRecurringFrequency
            // 
            this.cbRecurringFrequency.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbRecurringFrequency.FormattingEnabled = true;
            this.cbRecurringFrequency.Items.AddRange(new object[] {
            "Every Day",
            "Weekdays",
            "Weekends",
            "Every Monday",
            "Every Tuesday",
            "Every Wednesday",
            "Every Thursday",
            "Every Friday",
            "Every Saturday",
            "Every Sunday",
            "Monthly"});
            this.cbRecurringFrequency.Location = new System.Drawing.Point(63, 54);
            this.cbRecurringFrequency.Name = "cbRecurringFrequency";
            this.cbRecurringFrequency.Size = new System.Drawing.Size(114, 21);
            this.cbRecurringFrequency.TabIndex = 6;
            this.cbRecurringFrequency.SelectedIndexChanged += new System.EventHandler(this.cbRecurringFrequency_SelectedIndexChanged);
            // 
            // btnRecurringEndDateChoose
            // 
            this.btnRecurringEndDateChoose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRecurringEndDateChoose.Location = new System.Drawing.Point(252, 27);
            this.btnRecurringEndDateChoose.Name = "btnRecurringEndDateChoose";
            this.btnRecurringEndDateChoose.Size = new System.Drawing.Size(28, 23);
            this.btnRecurringEndDateChoose.TabIndex = 4;
            this.btnRecurringEndDateChoose.Text = "...";
            this.btnRecurringEndDateChoose.UseVisualStyleBackColor = true;
            this.btnRecurringEndDateChoose.Click += new System.EventHandler(this.btnRecurringEndDateChoose_Click);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(-3, 30);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(55, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "End Date:";
            // 
            // tbRecurringEndDate
            // 
            this.tbRecurringEndDate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbRecurringEndDate.Location = new System.Drawing.Point(63, 27);
            this.tbRecurringEndDate.Name = "tbRecurringEndDate";
            this.tbRecurringEndDate.ReadOnly = true;
            this.tbRecurringEndDate.Size = new System.Drawing.Size(183, 21);
            this.tbRecurringEndDate.TabIndex = 3;
            // 
            // btnRecurringStartDateChoose
            // 
            this.btnRecurringStartDateChoose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnRecurringStartDateChoose.Location = new System.Drawing.Point(252, 0);
            this.btnRecurringStartDateChoose.Name = "btnRecurringStartDateChoose";
            this.btnRecurringStartDateChoose.Size = new System.Drawing.Size(28, 23);
            this.btnRecurringStartDateChoose.TabIndex = 1;
            this.btnRecurringStartDateChoose.Text = "...";
            this.btnRecurringStartDateChoose.UseVisualStyleBackColor = true;
            this.btnRecurringStartDateChoose.Click += new System.EventHandler(this.btnRecurringStartDateChoose_Click);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(-3, 3);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(61, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Start Date:";
            // 
            // tbRecurringStartDate
            // 
            this.tbRecurringStartDate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbRecurringStartDate.Location = new System.Drawing.Point(63, 0);
            this.tbRecurringStartDate.Name = "tbRecurringStartDate";
            this.tbRecurringStartDate.ReadOnly = true;
            this.tbRecurringStartDate.Size = new System.Drawing.Size(183, 21);
            this.tbRecurringStartDate.TabIndex = 0;
            // 
            // rbRecurring
            // 
            this.rbRecurring.AutoSize = true;
            this.rbRecurring.Location = new System.Drawing.Point(17, 169);
            this.rbRecurring.Name = "rbRecurring";
            this.rbRecurring.Size = new System.Drawing.Size(100, 17);
            this.rbRecurring.TabIndex = 1;
            this.rbRecurring.TabStop = true;
            this.rbRecurring.Text = "Recurring entry";
            this.rbRecurring.UseVisualStyleBackColor = true;
            this.rbRecurring.CheckedChanged += new System.EventHandler(this.rbRecurring_CheckedChanged);
            // 
            // pnlOneTime
            // 
            this.pnlOneTime.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.pnlOneTime.Controls.Add(this.label12);
            this.pnlOneTime.Controls.Add(this.tbOneTimeEndDate);
            this.pnlOneTime.Controls.Add(this.btnOneTimeEndDateChoose);
            this.pnlOneTime.Controls.Add(this.label1);
            this.pnlOneTime.Controls.Add(this.tbOneTimeEndTime);
            this.pnlOneTime.Controls.Add(this.tbOneTimeStartDate);
            this.pnlOneTime.Controls.Add(this.label3);
            this.pnlOneTime.Controls.Add(this.btnOneTimeStartDateChoose);
            this.pnlOneTime.Controls.Add(this.label2);
            this.pnlOneTime.Controls.Add(this.tbOneTimeStartTime);
            this.pnlOneTime.Location = new System.Drawing.Point(37, 53);
            this.pnlOneTime.Name = "pnlOneTime";
            this.pnlOneTime.Size = new System.Drawing.Size(334, 105);
            this.pnlOneTime.TabIndex = 8;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(-2, 57);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(55, 13);
            this.label12.TabIndex = 9;
            this.label12.Text = "End Date:";
            // 
            // tbOneTimeEndDate
            // 
            this.tbOneTimeEndDate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbOneTimeEndDate.Location = new System.Drawing.Point(64, 54);
            this.tbOneTimeEndDate.Name = "tbOneTimeEndDate";
            this.tbOneTimeEndDate.ReadOnly = true;
            this.tbOneTimeEndDate.Size = new System.Drawing.Size(189, 21);
            this.tbOneTimeEndDate.TabIndex = 3;
            // 
            // btnOneTimeEndDateChoose
            // 
            this.btnOneTimeEndDateChoose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOneTimeEndDateChoose.Location = new System.Drawing.Point(259, 54);
            this.btnOneTimeEndDateChoose.Name = "btnOneTimeEndDateChoose";
            this.btnOneTimeEndDateChoose.Size = new System.Drawing.Size(75, 23);
            this.btnOneTimeEndDateChoose.TabIndex = 4;
            this.btnOneTimeEndDateChoose.Text = "Choose...";
            this.btnOneTimeEndDateChoose.UseVisualStyleBackColor = true;
            this.btnOneTimeEndDateChoose.Click += new System.EventHandler(this.btnOneTimeEndDateChoose_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(-3, 3);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Start Date:";
            // 
            // tbOneTimeEndTime
            // 
            this.tbOneTimeEndTime.Location = new System.Drawing.Point(63, 81);
            this.tbOneTimeEndTime.Name = "tbOneTimeEndTime";
            this.tbOneTimeEndTime.Size = new System.Drawing.Size(64, 21);
            this.tbOneTimeEndTime.TabIndex = 5;
            this.tbOneTimeEndTime.TextChanged += new System.EventHandler(this.tbOneTimeEndTime_TextChanged);
            // 
            // tbOneTimeStartDate
            // 
            this.tbOneTimeStartDate.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbOneTimeStartDate.Location = new System.Drawing.Point(63, 0);
            this.tbOneTimeStartDate.Name = "tbOneTimeStartDate";
            this.tbOneTimeStartDate.ReadOnly = true;
            this.tbOneTimeStartDate.Size = new System.Drawing.Size(190, 21);
            this.tbOneTimeStartDate.TabIndex = 0;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(-2, 84);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(54, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "End Time:";
            // 
            // btnOneTimeStartDateChoose
            // 
            this.btnOneTimeStartDateChoose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOneTimeStartDateChoose.Location = new System.Drawing.Point(259, 0);
            this.btnOneTimeStartDateChoose.Name = "btnOneTimeStartDateChoose";
            this.btnOneTimeStartDateChoose.Size = new System.Drawing.Size(75, 23);
            this.btnOneTimeStartDateChoose.TabIndex = 1;
            this.btnOneTimeStartDateChoose.Text = "Choose...";
            this.btnOneTimeStartDateChoose.UseVisualStyleBackColor = true;
            this.btnOneTimeStartDateChoose.Click += new System.EventHandler(this.btnOneTimeStartDateChoose_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(-3, 30);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(60, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Start Time:";
            // 
            // tbOneTimeStartTime
            // 
            this.tbOneTimeStartTime.Location = new System.Drawing.Point(63, 27);
            this.tbOneTimeStartTime.Name = "tbOneTimeStartTime";
            this.tbOneTimeStartTime.Size = new System.Drawing.Size(64, 21);
            this.tbOneTimeStartTime.TabIndex = 2;
            this.tbOneTimeStartTime.TextChanged += new System.EventHandler(this.tbOneTimeStartTime_TextChanged);
            // 
            // rbOneTime
            // 
            this.rbOneTime.AutoSize = true;
            this.rbOneTime.Checked = true;
            this.rbOneTime.Location = new System.Drawing.Point(17, 30);
            this.rbOneTime.Name = "rbOneTime";
            this.rbOneTime.Size = new System.Drawing.Size(97, 17);
            this.rbOneTime.TabIndex = 0;
            this.rbOneTime.TabStop = true;
            this.rbOneTime.Text = "One time entry";
            this.rbOneTime.UseVisualStyleBackColor = true;
            this.rbOneTime.CheckedChanged += new System.EventHandler(this.rbOneTime_CheckedChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.cbUseEVETime);
            this.groupBox2.Controls.Add(this.cbSilent);
            this.groupBox2.Controls.Add(this.cbBlocking);
            this.groupBox2.Location = new System.Drawing.Point(12, 36);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(377, 93);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Schedule Type";
            // 
            // cbUseEVETime
            // 
            this.cbUseEVETime.AutoSize = true;
            this.cbUseEVETime.Location = new System.Drawing.Point(17, 66);
            this.cbUseEVETime.Name = "cbUseEVETime";
            this.cbUseEVETime.Size = new System.Drawing.Size(172, 17);
            this.cbUseEVETime.TabIndex = 2;
            this.cbUseEVETime.Text = "Entry defined in EVE/UTC Time";
            this.cbUseEVETime.UseVisualStyleBackColor = true;
            // 
            // cbSilent
            // 
            this.cbSilent.AutoSize = true;
            this.cbSilent.Location = new System.Drawing.Point(17, 43);
            this.cbSilent.Name = "cbSilent";
            this.cbSilent.Size = new System.Drawing.Size(303, 17);
            this.cbSilent.TabIndex = 1;
            this.cbSilent.Text = "Silent mode during this entry (no tray tooltips nor sounds)";
            this.cbSilent.UseVisualStyleBackColor = true;
            // 
            // cbBlocking
            // 
            this.cbBlocking.AutoSize = true;
            this.cbBlocking.Location = new System.Drawing.Point(17, 20);
            this.cbBlocking.Name = "cbBlocking";
            this.cbBlocking.Size = new System.Drawing.Size(283, 17);
            this.cbBlocking.TabIndex = 0;
            this.cbBlocking.Text = "Unavailable to start new skill training during this entry";
            this.cbBlocking.UseVisualStyleBackColor = true;
            // 
            // tbTitle
            // 
            this.tbTitle.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tbTitle.Location = new System.Drawing.Point(49, 9);
            this.tbTitle.Name = "tbTitle";
            this.tbTitle.Size = new System.Drawing.Size(226, 21);
            this.tbTitle.TabIndex = 0;
            this.tbTitle.TextChanged += new System.EventHandler(this.tbTitle_TextChanged);
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(12, 12);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(31, 13);
            this.label11.TabIndex = 1;
            this.label11.Text = "Title:";
            // 
            // btnOk
            // 
            this.btnOk.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnOk.Enabled = false;
            this.btnOk.Location = new System.Drawing.Point(233, 515);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 3;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(314, 515);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 4;
            this.btnCancel.Text = "Cancel";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button1.Location = new System.Drawing.Point(12, 514);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(114, 23);
            this.button1.TabIndex = 5;
            this.button1.Text = "debug: roundtrip";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // EditScheduleEntryWindow
            // 
            this.AcceptButton = this.btnOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(401, 550);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label11);
            this.Controls.Add(this.tbTitle);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "EditScheduleEntryWindow";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Schedule Entry";
            this.Load += new System.EventHandler(this.EditScheduleEntryWindow_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.pnlRecurring.ResumeLayout(false);
            this.pnlRecurring.PerformLayout();
            this.pnlRecurWeekly.ResumeLayout(false);
            this.pnlRecurWeekly.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nudWeeklyFrequency)).EndInit();
            this.pnlRecurMonthly.ResumeLayout(false);
            this.pnlRecurMonthly.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.nudRecurDayOfMonth)).EndInit();
            this.pnlOneTime.ResumeLayout(false);
            this.pnlOneTime.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton rbOneTime;
        private System.Windows.Forms.Panel pnlOneTime;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbOneTimeEndTime;
        private System.Windows.Forms.TextBox tbOneTimeStartDate;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button btnOneTimeStartDateChoose;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbOneTimeStartTime;
        private System.Windows.Forms.RadioButton rbRecurring;
        private System.Windows.Forms.Panel pnlRecurring;
        private System.Windows.Forms.Panel pnlRecurMonthly;
        private System.Windows.Forms.NumericUpDown nudRecurDayOfMonth;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox cbRecurringFrequency;
        private System.Windows.Forms.Button btnRecurringEndDateChoose;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox tbRecurringEndDate;
        private System.Windows.Forms.Button btnRecurringStartDateChoose;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox tbRecurringStartDate;
        private System.Windows.Forms.Button btnRecurringNoEndDate;
        private System.Windows.Forms.TextBox tbRecurringTimeTo;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox tbRecurringTimeFrom;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ComboBox cbRecurOnOverflow;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btnRecurringNoStartDate;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox cbSilent;
        private System.Windows.Forms.CheckBox cbBlocking;
        private System.Windows.Forms.TextBox tbTitle;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox tbOneTimeEndDate;
        private System.Windows.Forms.Button btnOneTimeEndDateChoose;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.CheckBox cbUseEVETime;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.NumericUpDown nudWeeklyFrequency;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Panel pnlRecurWeekly;
    }
}
