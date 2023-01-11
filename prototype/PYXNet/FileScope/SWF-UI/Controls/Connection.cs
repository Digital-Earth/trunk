// Connection.cs
// Copyright (C) 2002 Matt Zyzik (www.FileScope.com)
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Data;
using System.Windows.Forms;
using System.IO;
using FileScopeCore;

namespace FileScope
{
	/// <summary>
	/// Connection tab.
	/// </summary>
	public class Connection : System.Windows.Forms.UserControl
	{
		private FileScope.ElTabControl tabControl1;

		public delegate void updateUltrapeerStatus();
		public delegate void g2Query(string query);
		public delegate void g2NewConnection(string addr, int sockNum);
		public delegate void g2JustConnected(int sockNum);
		public delegate void g2Update(int sockNum, string bwInOut);
        public delegate void g2JustDisconnected(int sockNum);
        private System.Windows.Forms.ImageList imageList1;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.ToolBarButton toolBarButton8;
		private System.Windows.Forms.ToolBarButton toolBarButton9;
		private System.Windows.Forms.ToolBarButton toolBarButton10;
		private System.Windows.Forms.ToolBarButton toolBarButton11;
		private System.Windows.Forms.ToolBarButton toolBarButton12;
		private System.Windows.Forms.ToolBarButton toolBarButton13;
		private System.Windows.Forms.ToolBarButton toolBarButton14;
		private System.Windows.Forms.ColumnHeader columnHeader14;
		private System.Windows.Forms.ColumnHeader columnHeader15;
		private System.Windows.Forms.ColumnHeader columnHeader16;
		private System.Windows.Forms.ColumnHeader columnHeader17;
		private System.Windows.Forms.ColumnHeader columnHeader18;
        private System.Windows.Forms.ColumnHeader columnHeader19;
		private FileScope.ElMenuItem elMenuItem1;
		private FileScope.ElMenuItem elMenuItem2;
		private System.Windows.Forms.ContextMenu contextMenu3;
		private FileScope.ElMenuItem menuItem10;
		private FileScope.ElMenuItem menuItem11;
		private FileScope.ElMenuItem menuItem12;
		private FileScope.ElMenuItem menuItem13;
		private FileScope.ElMenuItem menuItem14;
		private FileScope.ElMenuItem menuItem15;
		private FileScope.ElMenuItem menuItem16;
		private System.Windows.Forms.ColumnHeader columnHeader24;
		private System.Windows.Forms.ColumnHeader columnHeader28;
		private System.Windows.Forms.ColumnHeader columnHeader29;
		private System.Windows.Forms.ColumnHeader columnHeader30;
		private System.Windows.Forms.ColumnHeader columnHeader31;
		private System.Windows.Forms.ColumnHeader columnHeader32;
		private System.Windows.Forms.ColumnHeader columnHeader33;
		public System.Windows.Forms.ListView listView4;
		private System.Windows.Forms.ColumnHeader columnHeader34;
		private System.Windows.Forms.ColumnHeader columnHeader35;
		private System.Windows.Forms.ColumnHeader columnHeader36;
		private System.Windows.Forms.ColumnHeader columnHeader37;
		private System.Windows.Forms.ColumnHeader columnHeader38;
		private System.Windows.Forms.ColumnHeader columnHeader39;
		private System.Windows.Forms.ColumnHeader columnHeader40;
		private System.Windows.Forms.ColumnHeader columnHeader41;
        private System.Windows.Forms.ColumnHeader columnHeader42;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.NumericUpDown numericUpDown1;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.ContextMenu contextMenu4;
		private FileScope.ElMenuItem menuItem19;
		private FileScope.ElMenuItem menuItem20;
		private FileScope.ElMenuItem menuItem21;
		private FileScope.ElMenuItem menuItem22;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.ListBox listQueries2;
		private FileScope.ElMenuItem menuItem23;
		private System.Windows.Forms.Label lblg2qka;
		private System.Windows.Forms.Label lblg2qkr;
		private System.Windows.Forms.Label lblg2qht;
		private System.Windows.Forms.Label lblg2khl;
		private System.Windows.Forms.Label lblg2lni;
		private System.Windows.Forms.Label lblg2qa;
		private System.Windows.Forms.Label lblg2push;
		private System.Windows.Forms.Label lblg2qh2;
		private System.Windows.Forms.Label lblg2q2;
		private System.Windows.Forms.Label lblg2po;
		private System.Windows.Forms.Label lblg2pi;
		public ContextMenu[] cmArray;

		public Connection()
		{
			InitializeComponent();
			cmArray = new ContextMenu[]{ this.contextMenu3, this.contextMenu4};
			UpdateUltrapeerStatus();
            this.numericUpDown1.Value = Configuration.settings.gConnectionsToKeep;
			Control c = (Control)this;
			Themes.SetupTheme(c);
			tabControl1.SelectedIndex = 1;
		}

		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Connection));
            this.tabControl1 = new FileScope.ElTabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.listQueries2 = new System.Windows.Forms.ListBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.lblg2qka = new System.Windows.Forms.Label();
            this.lblg2qkr = new System.Windows.Forms.Label();
            this.lblg2qht = new System.Windows.Forms.Label();
            this.lblg2khl = new System.Windows.Forms.Label();
            this.lblg2lni = new System.Windows.Forms.Label();
            this.lblg2qa = new System.Windows.Forms.Label();
            this.lblg2push = new System.Windows.Forms.Label();
            this.lblg2qh2 = new System.Windows.Forms.Label();
            this.lblg2q2 = new System.Windows.Forms.Label();
            this.lblg2po = new System.Windows.Forms.Label();
            this.lblg2pi = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.listView4 = new System.Windows.Forms.ListView();
            this.columnHeader34 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader35 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader36 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader37 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader41 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader38 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader42 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader39 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader40 = new System.Windows.Forms.ColumnHeader();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.toolBarButton8 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton9 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton10 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton11 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton12 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton13 = new System.Windows.Forms.ToolBarButton();
            this.toolBarButton14 = new System.Windows.Forms.ToolBarButton();
            this.columnHeader14 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader15 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader16 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader17 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader18 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader19 = new System.Windows.Forms.ColumnHeader();
            this.elMenuItem1 = new FileScope.ElMenuItem();
            this.elMenuItem2 = new FileScope.ElMenuItem();
            this.contextMenu3 = new System.Windows.Forms.ContextMenu();
            this.menuItem10 = new FileScope.ElMenuItem();
            this.menuItem11 = new FileScope.ElMenuItem();
            this.menuItem12 = new FileScope.ElMenuItem();
            this.menuItem13 = new FileScope.ElMenuItem();
            this.menuItem14 = new FileScope.ElMenuItem();
            this.menuItem15 = new FileScope.ElMenuItem();
            this.menuItem16 = new FileScope.ElMenuItem();
            this.columnHeader24 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader28 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader29 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader30 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader31 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader32 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader33 = new System.Windows.Forms.ColumnHeader();
            this.contextMenu4 = new System.Windows.Forms.ContextMenu();
            this.menuItem23 = new FileScope.ElMenuItem();
            this.menuItem19 = new FileScope.ElMenuItem();
            this.menuItem20 = new FileScope.ElMenuItem();
            this.menuItem21 = new FileScope.ElMenuItem();
            this.menuItem22 = new FileScope.ElMenuItem();
            this.tabControl1.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tabControl1.HotTrack = true;
            this.tabControl1.ImageList = this.imageList1;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Multiline = true;
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(880, 440);
            this.tabControl1.TabIndex = 0;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.groupBox4);
            this.tabPage3.Controls.Add(this.groupBox3);
            this.tabPage3.Controls.Add(this.listView4);
            this.tabPage3.ImageIndex = 2;
            this.tabPage3.Location = new System.Drawing.Point(4, 39);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(872, 397);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Gnutella2";
            this.tabPage3.Resize += new System.EventHandler(this.tabPage3_Resize);
            this.tabPage3.Paint += new System.Windows.Forms.PaintEventHandler(this.tabPage3_Paint);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.listQueries2);
            this.groupBox4.Font = new System.Drawing.Font("Arial", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox4.Location = new System.Drawing.Point(432, 216);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(276, 160);
            this.groupBox4.TabIndex = 8;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Incoming Queries";
            // 
            // listQueries2
            // 
            this.listQueries2.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.listQueries2.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.listQueries2.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.listQueries2.ItemHeight = 14;
            this.listQueries2.Location = new System.Drawing.Point(6, 28);
            this.listQueries2.Name = "listQueries2";
            this.listQueries2.SelectionMode = System.Windows.Forms.SelectionMode.None;
            this.listQueries2.Size = new System.Drawing.Size(264, 126);
            this.listQueries2.TabIndex = 0;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.lblg2qka);
            this.groupBox3.Controls.Add(this.lblg2qkr);
            this.groupBox3.Controls.Add(this.lblg2qht);
            this.groupBox3.Controls.Add(this.lblg2khl);
            this.groupBox3.Controls.Add(this.lblg2lni);
            this.groupBox3.Controls.Add(this.lblg2qa);
            this.groupBox3.Controls.Add(this.lblg2push);
            this.groupBox3.Controls.Add(this.lblg2qh2);
            this.groupBox3.Controls.Add(this.lblg2q2);
            this.groupBox3.Controls.Add(this.lblg2po);
            this.groupBox3.Controls.Add(this.lblg2pi);
            this.groupBox3.Controls.Add(this.label7);
            this.groupBox3.Controls.Add(this.numericUpDown1);
            this.groupBox3.Controls.Add(this.label8);
            this.groupBox3.Font = new System.Drawing.Font("Arial", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox3.Location = new System.Drawing.Point(120, 216);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(276, 160);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Stats";
            // 
            // lblg2qka
            // 
            this.lblg2qka.AutoSize = true;
            this.lblg2qka.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2qka.Location = new System.Drawing.Point(176, 136);
            this.lblg2qka.Name = "lblg2qka";
            this.lblg2qka.Size = new System.Drawing.Size(63, 15);
            this.lblg2qka.TabIndex = 19;
            this.lblg2qka.Text = "QKA / sec:";
            // 
            // lblg2qkr
            // 
            this.lblg2qkr.AutoSize = true;
            this.lblg2qkr.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2qkr.Location = new System.Drawing.Point(176, 120);
            this.lblg2qkr.Name = "lblg2qkr";
            this.lblg2qkr.Size = new System.Drawing.Size(65, 15);
            this.lblg2qkr.TabIndex = 18;
            this.lblg2qkr.Text = "QKR / sec:";
            // 
            // lblg2qht
            // 
            this.lblg2qht.AutoSize = true;
            this.lblg2qht.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2qht.Location = new System.Drawing.Point(176, 104);
            this.lblg2qht.Name = "lblg2qht";
            this.lblg2qht.Size = new System.Drawing.Size(64, 15);
            this.lblg2qht.TabIndex = 17;
            this.lblg2qht.Text = "QHT / sec:";
            // 
            // lblg2khl
            // 
            this.lblg2khl.AutoSize = true;
            this.lblg2khl.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2khl.Location = new System.Drawing.Point(88, 120);
            this.lblg2khl.Name = "lblg2khl";
            this.lblg2khl.Size = new System.Drawing.Size(63, 15);
            this.lblg2khl.TabIndex = 16;
            this.lblg2khl.Text = "KHL / sec:";
            // 
            // lblg2lni
            // 
            this.lblg2lni.AutoSize = true;
            this.lblg2lni.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2lni.Location = new System.Drawing.Point(88, 104);
            this.lblg2lni.Name = "lblg2lni";
            this.lblg2lni.Size = new System.Drawing.Size(58, 15);
            this.lblg2lni.TabIndex = 15;
            this.lblg2lni.Text = "LNI / sec:";
            // 
            // lblg2qa
            // 
            this.lblg2qa.AutoSize = true;
            this.lblg2qa.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2qa.Location = new System.Drawing.Point(8, 136);
            this.lblg2qa.Name = "lblg2qa";
            this.lblg2qa.Size = new System.Drawing.Size(55, 15);
            this.lblg2qa.TabIndex = 14;
            this.lblg2qa.Text = "QA / sec:";
            // 
            // lblg2push
            // 
            this.lblg2push.AutoSize = true;
            this.lblg2push.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2push.Location = new System.Drawing.Point(88, 88);
            this.lblg2push.Name = "lblg2push";
            this.lblg2push.Size = new System.Drawing.Size(73, 15);
            this.lblg2push.TabIndex = 13;
            this.lblg2push.Text = "PUSH / sec:";
            // 
            // lblg2qh2
            // 
            this.lblg2qh2.AutoSize = true;
            this.lblg2qh2.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2qh2.Location = new System.Drawing.Point(88, 136);
            this.lblg2qh2.Name = "lblg2qh2";
            this.lblg2qh2.Size = new System.Drawing.Size(64, 15);
            this.lblg2qh2.TabIndex = 12;
            this.lblg2qh2.Text = "QH2 / sec:";
            // 
            // lblg2q2
            // 
            this.lblg2q2.AutoSize = true;
            this.lblg2q2.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2q2.Location = new System.Drawing.Point(8, 120);
            this.lblg2q2.Name = "lblg2q2";
            this.lblg2q2.Size = new System.Drawing.Size(55, 15);
            this.lblg2q2.TabIndex = 11;
            this.lblg2q2.Text = "Q2 / sec:";
            // 
            // lblg2po
            // 
            this.lblg2po.AutoSize = true;
            this.lblg2po.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2po.Location = new System.Drawing.Point(8, 104);
            this.lblg2po.Name = "lblg2po";
            this.lblg2po.Size = new System.Drawing.Size(56, 15);
            this.lblg2po.TabIndex = 10;
            this.lblg2po.Text = "PO / sec:";
            // 
            // lblg2pi
            // 
            this.lblg2pi.AutoSize = true;
            this.lblg2pi.Font = new System.Drawing.Font("Arial", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblg2pi.Location = new System.Drawing.Point(8, 88);
            this.lblg2pi.Name = "lblg2pi";
            this.lblg2pi.Size = new System.Drawing.Size(50, 15);
            this.lblg2pi.TabIndex = 9;
            this.lblg2pi.Text = "PI / sec:";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("Arial", 14.5F);
            this.label7.Location = new System.Drawing.Point(82, 21);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(113, 23);
            this.label7.TabIndex = 6;
            this.label7.Text = "[Leaf Node]";
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Enabled = false;
            this.numericUpDown1.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.numericUpDown1.Location = new System.Drawing.Point(72, 56);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            800,
            0,
            0,
            0});
            this.numericUpDown1.Minimum = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(48, 26);
            this.numericUpDown1.TabIndex = 5;
            this.numericUpDown1.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDown1.ValueChanged += new System.EventHandler(this.numericUpDown1_ValueChanged);
            this.numericUpDown1.Leave += new System.EventHandler(this.numericUpDown1_Leave);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Enabled = false;
            this.label8.Font = new System.Drawing.Font("Arial", 14.5F);
            this.label8.Location = new System.Drawing.Point(23, 56);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(249, 23);
            this.label8.TabIndex = 4;
            this.label8.Text = "keep           connections up";
            // 
            // listView4
            // 
            this.listView4.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader34,
            this.columnHeader35,
            this.columnHeader36,
            this.columnHeader37,
            this.columnHeader41,
            this.columnHeader38,
            this.columnHeader42,
            this.columnHeader39,
            this.columnHeader40});
            this.listView4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.listView4.FullRowSelect = true;
            this.listView4.GridLines = true;
            this.listView4.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listView4.HideSelection = false;
            this.listView4.Location = new System.Drawing.Point(0, 0);
            this.listView4.Name = "listView4";
            this.listView4.Size = new System.Drawing.Size(736, 192);
            this.listView4.TabIndex = 6;
            this.listView4.UseCompatibleStateImageBehavior = false;
            this.listView4.View = System.Windows.Forms.View.Details;
            this.listView4.MouseUp += new System.Windows.Forms.MouseEventHandler(this.listView4_MouseUp);
            // 
            // columnHeader34
            // 
            this.columnHeader34.Text = "Host";
            this.columnHeader34.Width = 120;
            // 
            // columnHeader35
            // 
            this.columnHeader35.Text = "Status";
            this.columnHeader35.Width = 70;
            // 
            // columnHeader36
            // 
            this.columnHeader36.Text = "Route";
            this.columnHeader36.Width = 42;
            // 
            // columnHeader37
            // 
            this.columnHeader37.Text = "Bandwidth (I/O) KB/s";
            this.columnHeader37.Width = 124;
            // 
            // columnHeader41
            // 
            this.columnHeader41.Text = "Name";
            this.columnHeader41.Width = 70;
            // 
            // columnHeader38
            // 
            this.columnHeader38.Text = "Mode";
            this.columnHeader38.Width = 55;
            // 
            // columnHeader42
            // 
            this.columnHeader42.Text = "Leaves";
            this.columnHeader42.Width = 50;
            // 
            // columnHeader39
            // 
            this.columnHeader39.Text = "Vendor";
            this.columnHeader39.Width = 110;
            // 
            // columnHeader40
            // 
            this.columnHeader40.Text = "C#";
            this.columnHeader40.Width = 26;
            // 
            // imageList1
            // 
            this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList1.Images.SetKeyName(0, "");
            this.imageList1.Images.SetKeyName(1, "");
            this.imageList1.Images.SetKeyName(2, "");
            this.imageList1.Images.SetKeyName(3, "");
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 1000;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // toolBarButton8
            // 
            this.toolBarButton8.Enabled = false;
            this.toolBarButton8.Name = "toolBarButton8";
            this.toolBarButton8.Text = "Connect";
            this.toolBarButton8.ToolTipText = "Connect to selected server(s)";
            // 
            // toolBarButton9
            // 
            this.toolBarButton9.Enabled = false;
            this.toolBarButton9.Name = "toolBarButton9";
            this.toolBarButton9.Text = "Disconnect";
            this.toolBarButton9.ToolTipText = "Disconnect from selected server(s)";
            // 
            // toolBarButton10
            // 
            this.toolBarButton10.Name = "toolBarButton10";
            this.toolBarButton10.Style = System.Windows.Forms.ToolBarButtonStyle.Separator;
            // 
            // toolBarButton11
            // 
            this.toolBarButton11.Name = "toolBarButton11";
            this.toolBarButton11.Text = "Add";
            this.toolBarButton11.ToolTipText = "Add a new server";
            // 
            // toolBarButton12
            // 
            this.toolBarButton12.Enabled = false;
            this.toolBarButton12.Name = "toolBarButton12";
            this.toolBarButton12.Text = "Remove";
            this.toolBarButton12.ToolTipText = "Remove the selected server(s)";
            // 
            // toolBarButton13
            // 
            this.toolBarButton13.Name = "toolBarButton13";
            this.toolBarButton13.Style = System.Windows.Forms.ToolBarButtonStyle.Separator;
            // 
            // toolBarButton14
            // 
            this.toolBarButton14.Name = "toolBarButton14";
            this.toolBarButton14.Text = "Open .wsx";
            this.toolBarButton14.ToolTipText = "Import a list of OpenNap servers";
            // 
            // columnHeader14
            // 
            this.columnHeader14.Text = "Server";
            this.columnHeader14.Width = 205;
            // 
            // columnHeader15
            // 
            this.columnHeader15.Text = "Status";
            this.columnHeader15.Width = 80;
            // 
            // columnHeader16
            // 
            this.columnHeader16.Text = "Users";
            this.columnHeader16.Width = 48;
            // 
            // columnHeader17
            // 
            this.columnHeader17.Text = "Files";
            this.columnHeader17.Width = 68;
            // 
            // columnHeader18
            // 
            this.columnHeader18.Text = "Gigabytes";
            this.columnHeader18.Width = 72;
            // 
            // columnHeader19
            // 
            this.columnHeader19.Text = "C#";
            this.columnHeader19.Width = 26;
            // 
            // elMenuItem1
            // 
            this.elMenuItem1.Index = -1;
            this.elMenuItem1.Text = "Connect";
            // 
            // elMenuItem2
            // 
            this.elMenuItem2.Index = -1;
            this.elMenuItem2.Text = "Connect";
            // 
            // contextMenu3
            // 
            this.contextMenu3.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem10,
            this.menuItem11,
            this.menuItem12,
            this.menuItem13,
            this.menuItem14,
            this.menuItem15,
            this.menuItem16});
            
            // 
            // columnHeader24
            // 
            this.columnHeader24.Text = "Host";
            this.columnHeader24.Width = 120;
            // 
            // columnHeader28
            // 
            this.columnHeader28.Text = "Status";
            this.columnHeader28.Width = 70;
            // 
            // columnHeader29
            // 
            this.columnHeader29.Text = "Route";
            this.columnHeader29.Width = 42;
            // 
            // columnHeader30
            // 
            this.columnHeader30.Text = "Bandwidth (I/O) KB/s";
            this.columnHeader30.Width = 124;
            // 
            // columnHeader31
            // 
            this.columnHeader31.Text = "Ultrapeer";
            this.columnHeader31.Width = 55;
            // 
            // columnHeader32
            // 
            this.columnHeader32.Text = "Vendor";
            this.columnHeader32.Width = 110;
            // 
            // columnHeader33
            // 
            this.columnHeader33.Text = "C#";
            this.columnHeader33.Width = 26;
            // 
            // contextMenu4
            // 
            this.contextMenu4.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem23,
            this.menuItem19,
            this.menuItem20,
            this.menuItem21,
            this.menuItem22});
            // 
            // menuItem23
            // 
            this.menuItem23.Enabled = false;
            this.menuItem23.Index = 0;
            this.menuItem23.Text = "Browse";
            this.menuItem23.Click += new System.EventHandler(this.menuItem23_Click);
            // 
            // menuItem19
            // 
            this.menuItem19.Enabled = false;
            this.menuItem19.Index = 1;
            this.menuItem19.Text = "Chat";
            this.menuItem19.Click += new System.EventHandler(this.menuItem19_Click);
            // 
            // menuItem20
            // 
            this.menuItem20.Index = 2;
            this.menuItem20.Text = "-";
            // 
            // menuItem21
            // 
            this.menuItem21.Index = 3;
            this.menuItem21.Text = "Add";
            this.menuItem21.Click += new System.EventHandler(this.menuItem21_Click);
            // 
            // menuItem22
            // 
            this.menuItem22.Enabled = false;
            this.menuItem22.Index = 4;
            this.menuItem22.Text = "Remove";
            this.menuItem22.Click += new System.EventHandler(this.menuItem22_Click);
            // 
            // Connection
            // 
            this.Controls.Add(this.tabControl1);
            this.Name = "Connection";
            this.Size = new System.Drawing.Size(880, 440);
            this.tabControl1.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            this.ResumeLayout(false);

		}
		#endregion

		

		private void tabPage3_Resize(object sender, System.EventArgs e)
		{
			groupBox3.Left = (tabPage3.ClientSize.Width / 4) - (groupBox3.Width / 2);
			groupBox4.Left = tabPage3.ClientSize.Width - groupBox3.Left - groupBox4.Width;
			groupBox3.Top = tabPage3.ClientSize.Height - groupBox3.Height;
			groupBox4.Top = groupBox3.Top;
			listView4.Height = groupBox3.Top - 25;
			listView4.Width = tabPage3.ClientSize.Width;
		}

		private void tabPage3_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
		{
			HatchBrush hbtr = new HatchBrush(HatchStyle.WideDownwardDiagonal, Color.Transparent, Color.FromArgb(40, StartApp.settings.clHomeBR));
			Pen ptr = new Pen(hbtr, 4);
			e.Graphics.DrawLine(ptr, 0, groupBox3.Top - 10, tabPage3.ClientSize.Width, groupBox3.Top - 10);
			e.Graphics.DrawLine(ptr, tabPage3.ClientSize.Width / 2, groupBox3.Top - 10, tabPage3.ClientSize.Width / 2, tabPage3.ClientSize.Height);
			hbtr.Dispose();
			ptr.Dispose();
			e.Graphics.Dispose();
		}

        /// <summary>
        /// Incoming gnutella 2 query.
        /// </summary>
        public void G2Query(string query)
        {
            if (tabControl1.SelectedIndex == 1)
            {
                listQueries2.Items.Insert(0, query);
                if (listQueries2.Items.Count > 50)
                    listQueries2.Items.RemoveAt(listQueries2.Items.Count - 1);
            }
        }
		
		public void G2NewConnection(string addr, int sockNum)
		{
			try
			{
				string[] subitems = new string[9];
				subitems[0] = addr;
				subitems[1] = "Connecting";
                if (FileScopeCore.Gnutella2.Sck.scks[sockNum].incoming)
					subitems[2] = "In";
				else
					subitems[2] = "Out";
				subitems[3] = "0.00 / 0.00";
				subitems[4] = "?";
				subitems[5] = "?";
				subitems[6] = "?";
				subitems[7] = "?";
				subitems[8] = sockNum.ToString();
				ListViewItem lvi = new ListViewItem(subitems);
				lvi.ForeColor = StartApp.settings.clHighlight3;
				listView4.Items.Add(lvi);
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("Connection G2NewConnection");
			}
		}

		public void G2JustConnected(int sockNum)
		{
			try
			{
				foreach(ListViewItem lvi in listView4.Items)
					if(lvi.SubItems[8].Text == sockNum.ToString())
					{
                        if (FileScopeCore.Gnutella2.Sck.scks[sockNum].incoming)
                            lvi.SubItems[0].Text = FileScopeCore.Gnutella2.Sck.scks[sockNum].RemoteIP();
						lvi.SubItems[1].Text = "Connected";
                        lvi.SubItems[5].Text = FileScopeCore.Gnutella2.Sck.scks[sockNum].mode.ToString();
                        lvi.SubItems[7].Text = FileScopeCore.Gnutella2.Sck.scks[sockNum].vendor;
						lvi.SubItems[8].Text = sockNum.ToString();
                        if (FileScopeCore.Gnutella2.Sck.scks[sockNum].mode == FileScopeCore.Gnutella2.G2Mode.Ultrapeer)
							lvi.ForeColor = StartApp.settings.clHighlight1;
						else
							lvi.ForeColor = StartApp.settings.clHighlight2;
						return;
					}
				System.Diagnostics.Debug.WriteLine("Connection G2JustConnected shouldn't have gotten this far");
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("Connection G2JustConnected");
			}
		}

		public void G2Update(int sockNum, string bwInOut)
		{
			try
			{
				if(tabControl1.SelectedIndex != 1)
					return;
				foreach(ListViewItem lvi in listView4.Items)
					if(lvi.SubItems[8].Text == sockNum.ToString())
					{
						if(lvi.SubItems[1].Text == "Connecting")
							lvi.SubItems[1].Text = "Handshaking";
						if(lvi.SubItems[3].Text != bwInOut)
							lvi.SubItems[3].Text = bwInOut;
                        string leavesData = FileScopeCore.Gnutella2.Sck.scks[sockNum].leaves.ToString() + @" / " + FileScopeCore.Gnutella2.Sck.scks[sockNum].maxleaves.ToString();
						if(lvi.SubItems[6].Text != leavesData)
							lvi.SubItems[6].Text = leavesData;
						return;
					}
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("Connection G2Update");
			}
		}

		public void G2JustDisconnected(int sockNum)
		{
			try
			{
				ListViewItem lvi;
				for(int x = 0; x < listView4.Items.Count; x++)
				{
					lvi = (ListViewItem)listView4.Items[x];
					if(lvi.SubItems[8].Text == sockNum.ToString())
					{
						listView4.Items.RemoveAt(x);
						return;
					}
				}
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("Connection G2JustDisconnected");
			}
		}




		/// <summary>
		/// Update the controls appropriately.
		/// </summary>
		public void UpdateUltrapeerStatus()
		{
			/*
			if(Configuration.State.GnutellaConfiguration.State.ultrapeer)
			{
				this.labelStatus1.Text = "[Ultrapeer]";
				this.label1.Enabled = true;
				this.updownConnections1.Enabled = true;
			}
			else
			{
				this.labelStatus1.Text = "[Leaf Node]";
				this.label1.Enabled = false;
				this.updownConnections1.Enabled = false;
			}
			*/
			if(Configuration.State.Gnutella2Stats.ultrapeer)
			{
				this.label7.Text = "[Ultrapeer]";
				this.label8.Enabled = true;
				this.numericUpDown1.Enabled = true;
			}
			else
			{
				this.label7.Text = "[Leaf Node]";
				this.label8.Enabled = false;
				this.numericUpDown1.Enabled = false;
			}
		}

		private void updownConnections1_ValueChanged(object sender, System.EventArgs e)
		{
			updownStuff();
		}

		private void updownConnections1_Leave(object sender, System.EventArgs e)
		{
			updownStuff();
		}

		void updownStuff()
		{
			if(Convert.ToInt32(numericUpDown1.Value) > 800)
				numericUpDown1.Value = 800;
			else if(Convert.ToInt32(numericUpDown1.Value) < 40)
				numericUpDown1.Value = 40;
            Configuration.settings.gConnectionsToKeep = (int)numericUpDown1.Value;
		}

		
		private void richMessages_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			e.Handled = true;
		}

		private void timer1_Tick(object sender, System.EventArgs e)
		{
			try
			{

				//g2
				lblg2pi.Text = "PI / sec: " + Configuration.State.Gnutella2Stats.numPI.ToString();
				Configuration.State.Gnutella2Stats.numPI = 0;
				lblg2po.Text = "PO / sec: " + Configuration.State.Gnutella2Stats.numPO.ToString();
				Configuration.State.Gnutella2Stats.numPO = 0;
				lblg2lni.Text = "LNI / sec: " + Configuration.State.Gnutella2Stats.numLNI.ToString();
				Configuration.State.Gnutella2Stats.numLNI = 0;
				lblg2khl.Text = "KHL / sec: " + Configuration.State.Gnutella2Stats.numKHL.ToString();
				Configuration.State.Gnutella2Stats.numKHL = 0;
				lblg2q2.Text = "Q2 / sec: " + Configuration.State.Gnutella2Stats.numQ2.ToString();
				Configuration.State.Gnutella2Stats.numQ2 = 0;
				lblg2qa.Text = "QA / sec: " + Configuration.State.Gnutella2Stats.numQA.ToString();
				Configuration.State.Gnutella2Stats.numQA = 0;
				lblg2qh2.Text = "QH2 / sec: " + Configuration.State.Gnutella2Stats.numQH2.ToString();
				Configuration.State.Gnutella2Stats.numQH2 = 0;
				lblg2push.Text = "PUSH / sec: " + Configuration.State.Gnutella2Stats.numPUSH.ToString();
				Configuration.State.Gnutella2Stats.numPUSH = 0;
				lblg2qht.Text = "QHT / sec: " + Configuration.State.Gnutella2Stats.numQHT.ToString();
				Configuration.State.Gnutella2Stats.numQHT = 0;
				lblg2qkr.Text = "QKR / sec: " + Configuration.State.Gnutella2Stats.numQKR.ToString();
				Configuration.State.Gnutella2Stats.numQKR = 0;
				lblg2qka.Text = "QKA / sec: " + Configuration.State.Gnutella2Stats.numQKA.ToString();
				Configuration.State.Gnutella2Stats.numQKA = 0;
			}
			catch(Exception e2)
			{
				System.Diagnostics.Debug.WriteLine("Connection timer1_Tick: " + e2.Message);
			}
		}

		private void listView4_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if(listView4.SelectedItems.Count == 0)
				listView4.HideSelection = true;
			else
				listView4.HideSelection = false;
			this.contextMenu4.MenuItems[0].Enabled = (listView4.SelectedItems.Count == 1);
			this.contextMenu4.MenuItems[1].Enabled = (listView4.SelectedItems.Count == 1);
			this.contextMenu4.MenuItems[4].Enabled = (listView4.SelectedItems.Count > 0);

			//popup menu on right click
			if(e.Button == MouseButtons.Right)
			{
				System.Drawing.Point pos = new System.Drawing.Point(e.X, e.Y);
				this.contextMenu4.Show(listView4, pos);
			}
		}

		private void numericUpDown1_Leave(object sender, System.EventArgs e)
		{
			this.updownStuff();
		}

		private void numericUpDown1_ValueChanged(object sender, System.EventArgs e)
		{
			this.updownStuff();
		}

		private void menuItem19_Click(object sender, System.EventArgs e)
		{
			//g2 chat
			if(listView4.SelectedItems.Count != 1)
				return;
            FileScopeCore.Gnutella2.Sck g2sck = FileScopeCore.Gnutella2.Sck.scks[Convert.ToInt32(listView4.SelectedItems[0].SubItems[8].Text)];
			string chatip = g2sck.address + ":" + g2sck.port.ToString();
			ChatManager.Outgoing(ref chatip);
		}

		private void menuItem23_Click(object sender, System.EventArgs e)
		{
			//g2 browse
			if(listView4.SelectedItems.Count != 1)
				return;
            FileScopeCore.Gnutella2.Sck g2sck = FileScopeCore.Gnutella2.Sck.scks[Convert.ToInt32(listView4.SelectedItems[0].SubItems[8].Text)];
            FileScopeCore.Gnutella2.Sck.OutgoingBrowseHost(g2sck.address + ":" + g2sck.port.ToString());
		}

		private void menuItem21_Click(object sender, System.EventArgs e)
		{
			//g2 add
			string newHost = InputBox.Show("What is the IP address of the host you wish to connect to?");
			if(newHost.Length > 0)
			{
				if(newHost.ToLower().IndexOf(@"http://") != -1)
                    FileScopeCore.Gnutella2.Sck.OutgoingWebCache(newHost);
				else
                    FileScopeCore.Gnutella2.Sck.Outgoing(newHost);
			}
		}

		private void menuItem22_Click(object sender, System.EventArgs e)
		{
			//g2 remove
			try
			{
				foreach(ListViewItem lvi in listView4.SelectedItems)
                    FileScopeCore.Gnutella2.Sck.scks[Convert.ToInt32(lvi.SubItems[8].Text)].Disconnect("user disconnect");
			}
			catch
			{
				System.Diagnostics.Debug.WriteLine("Connection g2 remove");
			}
		}

		public void tabControl1_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			listQueries2.Items.Clear();
		}
	}
}
