// MainDlg.cs
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
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using FileScopeCore;

namespace FileScope
{
	/// <summary>
	/// Main window for FileScope.
	/// </summary>
	public class MainDlg : System.Windows.Forms.Form
	{
		public delegate void newVersion(string version);
		public HomePage homepage = null;
		public Connection connection = null;
		public Search search = null;
		public Transfers transfers = null;
		public Library library = null;
		public ChatPage chatpage = null;
		private System.Windows.Forms.MainMenu mainMenu1;
		public FileScope.ElTabControl tabControl1;
		public System.Windows.Forms.TabPage tabPage1;
		public System.Windows.Forms.TabPage tabPage2;
		public System.Windows.Forms.TabPage tabPage3;
		public System.Windows.Forms.TabPage tabPage5;
		public System.Windows.Forms.TabPage tabPage6;
		public System.Windows.Forms.TabPage tabPage7;
		private FileScope.ElMenuItem menuItem1;
		private FileScope.ElMenuItem menuItem2;
		private System.Windows.Forms.StatusBar statusBar1;
		public System.Windows.Forms.StatusBarPanel statusBarDownloads;
		public System.Windows.Forms.StatusBarPanel statusBarUploads;
		public System.Windows.Forms.StatusBarPanel statusBarConnection;
		public System.Windows.Forms.ImageList imageList1;
		public System.Windows.Forms.NotifyIcon trayIcon;
		private System.Windows.Forms.Timer tmrUpdate;
		private FileScope.ElMenuItem menuItem3;
		private FileScope.ElMenuItem menuItem4;
		private FileScope.ElMenuItem menuItem5;
		private FileScope.ElMenuItem menuItem6;
		private FileScope.ElMenuItem menuItem7;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.StatusBarPanel statusBarChats;
		public System.Windows.Forms.PictureBox pictureMoreInfo;
		public System.Windows.Forms.PictureBox pictureScope;
        private FileScope.ElMenuItem menuItem8;
        private FileScope.ElMenuItem menuItem11;
		private FileScope.ElMenuItem menuItem14;
		private FileScope.ElMenuItem menuItem15;
		private FileScope.ElMenuItem menuItem17;
		private FileScope.ElMenuItem menuItem18;
		private FileScope.ElMenuItem menuItem19;
		private FileScope.ElMenuItem menuItem20;
		private FileScope.ElMenuItem menuItem21;
		private FileScope.ElMenuItem menuItem22;
		private FileScope.ElMenuItem menuItem23;
		private FileScope.ElMenuItem menuItem27;
		private FileScope.ElMenuItem menuItem31;
		private FileScope.ElMenuItem menuItem25;
		private FileScope.ElMenuItem menuItem16;
		private FileScope.ElMenuItem menuItem32;
		private FileScope.ElMenuItem menuItem33;
		private System.Windows.Forms.ContextMenu contextMenu1;
		private FileScope.ElMenuItem menuItem34;
		private FileScope.ElMenuItem menuItem35;
		private FileScope.ElMenuItem menuItem36;
		private FileScope.ElMenuItem menuItem37;
		private FileScope.ElMenuItem menuItem38;
		private FileScope.ElMenuItem menuItem39;
		private FileScope.ElMenuItem menuItem24;
		private FileScope.ElMenuItem menuItem26;
		private FileScope.ElMenuItem menuItem28;
		private FileScope.ElMenuItem menuItem29;
		private FileScope.ElMenuItem menuItem30;
		private FileScope.ElMenuItem menuItem40;
		private FileScope.ElMenuItem menuItem41;
        private FileScope.ElMenuItem menuItem42;
		private FileScope.ElMenuItem menuItem46;
		private FileScope.ElMenuItem menuItem47;
        private FileScope.ElMenuItem menuItem48;
		private FileScope.ElMenuItem menuItem51;
		System.Resources.ResourceManager rm;

		public MainDlg()
		{
			InitializeComponent();
			MainWindowSizeSettings();
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

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainDlg));
            this.mainMenu1 = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new FileScope.ElMenuItem();
            this.menuItem47 = new FileScope.ElMenuItem();
            this.menuItem46 = new FileScope.ElMenuItem();
            this.menuItem2 = new FileScope.ElMenuItem();
            this.menuItem3 = new FileScope.ElMenuItem();
            this.menuItem4 = new FileScope.ElMenuItem();
            this.menuItem5 = new FileScope.ElMenuItem();
            this.menuItem6 = new FileScope.ElMenuItem();
            this.menuItem7 = new FileScope.ElMenuItem();
            this.menuItem8 = new FileScope.ElMenuItem();
            this.menuItem48 = new FileScope.ElMenuItem();
            this.menuItem11 = new FileScope.ElMenuItem();
            this.menuItem40 = new FileScope.ElMenuItem();
            this.menuItem41 = new FileScope.ElMenuItem();
            this.menuItem30 = new FileScope.ElMenuItem();
            this.menuItem42 = new FileScope.ElMenuItem();
            this.menuItem14 = new FileScope.ElMenuItem();
            this.menuItem15 = new FileScope.ElMenuItem();
            this.menuItem25 = new FileScope.ElMenuItem();
            this.menuItem29 = new FileScope.ElMenuItem();
            this.menuItem27 = new FileScope.ElMenuItem();
            this.menuItem32 = new FileScope.ElMenuItem();
            this.menuItem26 = new FileScope.ElMenuItem();
            this.menuItem16 = new FileScope.ElMenuItem();
            this.menuItem31 = new FileScope.ElMenuItem();
            this.menuItem33 = new FileScope.ElMenuItem();
            this.menuItem24 = new FileScope.ElMenuItem();
            this.menuItem28 = new FileScope.ElMenuItem();
            this.menuItem17 = new FileScope.ElMenuItem();
            this.menuItem18 = new FileScope.ElMenuItem();
            this.menuItem19 = new FileScope.ElMenuItem();
            this.menuItem20 = new FileScope.ElMenuItem();
            this.menuItem21 = new FileScope.ElMenuItem();
            this.menuItem22 = new FileScope.ElMenuItem();
            this.menuItem51 = new FileScope.ElMenuItem();
            this.menuItem23 = new FileScope.ElMenuItem();
            this.tabControl1 = new FileScope.ElTabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.statusBar1 = new System.Windows.Forms.StatusBar();
            this.statusBarConnection = new System.Windows.Forms.StatusBarPanel();
            this.statusBarDownloads = new System.Windows.Forms.StatusBarPanel();
            this.statusBarUploads = new System.Windows.Forms.StatusBarPanel();
            this.statusBarChats = new System.Windows.Forms.StatusBarPanel();
            this.trayIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.contextMenu1 = new System.Windows.Forms.ContextMenu();
            this.menuItem34 = new FileScope.ElMenuItem();
            this.menuItem39 = new FileScope.ElMenuItem();
            this.menuItem38 = new FileScope.ElMenuItem();
            this.menuItem37 = new FileScope.ElMenuItem();
            this.menuItem35 = new FileScope.ElMenuItem();
            this.menuItem36 = new FileScope.ElMenuItem();
            this.tmrUpdate = new System.Windows.Forms.Timer(this.components);
            this.pictureMoreInfo = new System.Windows.Forms.PictureBox();
            this.pictureScope = new System.Windows.Forms.PictureBox();
            this.tabControl1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarConnection)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarDownloads)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarUploads)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarChats)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureMoreInfo)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureScope)).BeginInit();
            this.SuspendLayout();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem8,
            this.menuItem14,
            this.menuItem17});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem47,
            this.menuItem46,
            this.menuItem2,
            this.menuItem3,
            this.menuItem4,
            this.menuItem5,
            this.menuItem6,
            this.menuItem7});
            this.menuItem1.Text = "File";
            // 
            // menuItem47
            // 
            this.menuItem47.Index = 0;
            this.menuItem47.Text = "Browse host...";
            this.menuItem47.Click += new System.EventHandler(this.menuItem47_Click);
            // 
            // menuItem46
            // 
            this.menuItem46.Index = 1;
            this.menuItem46.Text = "Chat with host...";
            this.menuItem46.Click += new System.EventHandler(this.menuItem46_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 2;
            this.menuItem2.Text = "Options...";
            this.menuItem2.Click += new System.EventHandler(this.menuItem2_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 3;
            this.menuItem3.Text = "-";
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 4;
            this.menuItem4.Text = "Minimize";
            this.menuItem4.Click += new System.EventHandler(this.menuItem4_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 5;
            this.menuItem5.Text = "Send to tray";
            this.menuItem5.Click += new System.EventHandler(this.menuItem5_Click);
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 6;
            this.menuItem6.Text = "-";
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 7;
            this.menuItem7.Text = "Exit";
            this.menuItem7.Click += new System.EventHandler(this.menuItem7_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 1;
            this.menuItem8.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem48,
            this.menuItem11,
            this.menuItem40,
            this.menuItem41,
            this.menuItem30,
            this.menuItem42});
            this.menuItem8.Text = "Network";
            // 
            // menuItem48
            // 
            this.menuItem48.Index = 0;
            this.menuItem48.Text = "-";
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 1;
            this.menuItem11.Text = "-";
            // 
            // menuItem40
            // 
            this.menuItem40.Index = 2;
            this.menuItem40.Text = "Connect Gnutella2";
            this.menuItem40.Click += new System.EventHandler(this.menuItem40_Click);
            // 
            // menuItem41
            // 
            this.menuItem41.Index = 3;
            this.menuItem41.Text = "Disconnect Gnutella2";
            this.menuItem41.Click += new System.EventHandler(this.menuItem41_Click);
            // 
            // menuItem30
            // 
            this.menuItem30.Index = 4;
            this.menuItem30.Text = "-";
            // 
            // menuItem42
            // 
            this.menuItem42.Index = 5;
            this.menuItem42.Text = "-";
            // 
            // menuItem14
            // 
            this.menuItem14.Index = 2;
            this.menuItem14.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem15,
            this.menuItem25,
            this.menuItem29,
            this.menuItem27,
            this.menuItem32,
            this.menuItem26,
            this.menuItem16,
            this.menuItem31,
            this.menuItem33,
            this.menuItem24,
            this.menuItem28});
            this.menuItem14.Text = "Themes";
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 0;
            this.menuItem15.Text = "Default Colors";
            this.menuItem15.Click += new System.EventHandler(this.menuItem15_Click);
            // 
            // menuItem25
            // 
            this.menuItem25.Index = 1;
            this.menuItem25.Text = "-";
            // 
            // menuItem29
            // 
            this.menuItem29.Index = 2;
            this.menuItem29.Text = "Blueish";
            this.menuItem29.Click += new System.EventHandler(this.menuItem29_Click);
            // 
            // menuItem27
            // 
            this.menuItem27.Index = 3;
            this.menuItem27.Text = "Cherry";
            this.menuItem27.Click += new System.EventHandler(this.menuItem27_Click);
            // 
            // menuItem32
            // 
            this.menuItem32.Index = 4;
            this.menuItem32.Text = "Dungeon";
            this.menuItem32.Click += new System.EventHandler(this.menuItem32_Click);
            // 
            // menuItem26
            // 
            this.menuItem26.Index = 5;
            this.menuItem26.Text = "FileScope";
            this.menuItem26.Click += new System.EventHandler(this.menuItem26_Click);
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 6;
            this.menuItem16.Text = "Forest";
            this.menuItem16.Click += new System.EventHandler(this.menuItem16_Click);
            // 
            // menuItem31
            // 
            this.menuItem31.Index = 7;
            this.menuItem31.Text = "Pleasant";
            this.menuItem31.Click += new System.EventHandler(this.menuItem31_Click);
            // 
            // menuItem33
            // 
            this.menuItem33.Index = 8;
            this.menuItem33.Text = "Starburst";
            this.menuItem33.Click += new System.EventHandler(this.menuItem33_Click);
            // 
            // menuItem24
            // 
            this.menuItem24.Index = 9;
            this.menuItem24.Text = "UniGray";
            this.menuItem24.Click += new System.EventHandler(this.menuItem24_Click);
            // 
            // menuItem28
            // 
            this.menuItem28.Index = 10;
            this.menuItem28.Text = "WinterFresh";
            this.menuItem28.Click += new System.EventHandler(this.menuItem28_Click);
            // 
            // menuItem17
            // 
            this.menuItem17.Index = 3;
            this.menuItem17.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem18,
            this.menuItem19,
            this.menuItem20,
            this.menuItem21,
            this.menuItem22,
            this.menuItem51,
            this.menuItem23});
            this.menuItem17.Text = "Help";
            // 
            // menuItem18
            // 
            this.menuItem18.Index = 0;
            this.menuItem18.Text = "FileScope.com";
            this.menuItem18.Click += new System.EventHandler(this.menuItem18_Click);
            // 
            // menuItem19
            // 
            this.menuItem19.Index = 1;
            this.menuItem19.Text = "-";
            // 
            // menuItem20
            // 
            this.menuItem20.Index = 2;
            this.menuItem20.Text = "User Guide";
            this.menuItem20.Click += new System.EventHandler(this.menuItem20_Click);
            // 
            // menuItem21
            // 
            this.menuItem21.Index = 3;
            this.menuItem21.Text = "FAQs";
            this.menuItem21.Click += new System.EventHandler(this.menuItem21_Click);
            // 
            // menuItem22
            // 
            this.menuItem22.Index = 4;
            this.menuItem22.Text = "-";
            // 
            // menuItem51
            // 
            this.menuItem51.Index = 5;
            this.menuItem51.Text = "Submit Bugs...";
            this.menuItem51.Click += new System.EventHandler(this.menuItem51_Click);
            // 
            // menuItem23
            // 
            this.menuItem23.Index = 6;
            this.menuItem23.Text = "About FileScope...";
            this.menuItem23.Click += new System.EventHandler(this.menuItem23_Click);
            // 
            // tabControl1
            // 
            this.tabControl1.Appearance = System.Windows.Forms.TabAppearance.FlatButtons;
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage5);
            this.tabControl1.Controls.Add(this.tabPage6);
            this.tabControl1.Controls.Add(this.tabPage7);
            this.tabControl1.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tabControl1.HotTrack = true;
            this.tabControl1.ImageList = this.imageList1;
            this.tabControl1.ImeMode = System.Windows.Forms.ImeMode.NoControl;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(576, 282);
            this.tabControl1.TabIndex = 0;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
            // 
            // tabPage1
            // 
            this.tabPage1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage1.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(2)));
            this.tabPage1.Location = new System.Drawing.Point(4, 28);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(568, 250);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Home";
            // 
            // tabPage2
            // 
            this.tabPage2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage2.ImageIndex = 1;
            this.tabPage2.Location = new System.Drawing.Point(4, 28);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Size = new System.Drawing.Size(568, 250);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Connections";
            // 
            // tabPage3
            // 
            this.tabPage3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage3.ImageIndex = 0;
            this.tabPage3.Location = new System.Drawing.Point(4, 28);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(568, 250);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Search";
            // 
            // tabPage5
            // 
            this.tabPage5.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage5.ImageIndex = 2;
            this.tabPage5.Location = new System.Drawing.Point(4, 28);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Size = new System.Drawing.Size(568, 250);
            this.tabPage5.TabIndex = 4;
            this.tabPage5.Text = "Transfers";
            // 
            // tabPage6
            // 
            this.tabPage6.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage6.ImageIndex = 3;
            this.tabPage6.Location = new System.Drawing.Point(4, 28);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Size = new System.Drawing.Size(568, 250);
            this.tabPage6.TabIndex = 5;
            this.tabPage6.Text = "Library";
            // 
            // tabPage7
            // 
            this.tabPage7.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.tabPage7.ImageIndex = 4;
            this.tabPage7.Location = new System.Drawing.Point(4, 28);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Size = new System.Drawing.Size(568, 250);
            this.tabPage7.TabIndex = 6;
            this.tabPage7.Text = "Chat";
            // 
            // imageList1
            // 
            this.imageList1.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList1.ImageStream")));
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            this.imageList1.Images.SetKeyName(0, "");
            this.imageList1.Images.SetKeyName(1, "");
            this.imageList1.Images.SetKeyName(2, "");
            this.imageList1.Images.SetKeyName(3, "");
            this.imageList1.Images.SetKeyName(4, "");
            // 
            // statusBar1
            // 
            this.statusBar1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.statusBar1.Location = new System.Drawing.Point(0, 509);
            this.statusBar1.Name = "statusBar1";
            this.statusBar1.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
            this.statusBarConnection,
            this.statusBarDownloads,
            this.statusBarUploads,
            this.statusBarChats});
            this.statusBar1.ShowPanels = true;
            this.statusBar1.Size = new System.Drawing.Size(682, 20);
            this.statusBar1.TabIndex = 1;
            // 
            // statusBarConnection
            // 
            this.statusBarConnection.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Spring;
            this.statusBarConnection.Icon = ((System.Drawing.Icon)(resources.GetObject("statusBarConnection.Icon")));
            this.statusBarConnection.Name = "statusBarConnection";
            this.statusBarConnection.Text = "Status: Disconnected";
            this.statusBarConnection.Width = 465;
            // 
            // statusBarDownloads
            // 
            this.statusBarDownloads.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarDownloads.Icon = ((System.Drawing.Icon)(resources.GetObject("statusBarDownloads.Icon")));
            this.statusBarDownloads.Name = "statusBarDownloads";
            this.statusBarDownloads.Text = "Downloads:";
            this.statusBarDownloads.Width = 94;
            // 
            // statusBarUploads
            // 
            this.statusBarUploads.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarUploads.Name = "statusBarUploads";
            this.statusBarUploads.Text = "Uploads: ";
            this.statusBarUploads.Width = 59;
            // 
            // statusBarChats
            // 
            this.statusBarChats.AutoSize = System.Windows.Forms.StatusBarPanelAutoSize.Contents;
            this.statusBarChats.Name = "statusBarChats";
            this.statusBarChats.Text = "Chats:";
            this.statusBarChats.Width = 47;
            // 
            // trayIcon
            // 
            this.trayIcon.ContextMenu = this.contextMenu1;
            this.trayIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("trayIcon.Icon")));
            this.trayIcon.Text = "FileScope";
            this.trayIcon.Visible = true;
            this.trayIcon.MouseUp += new System.Windows.Forms.MouseEventHandler(this.trayIcon_MouseUp);
            this.trayIcon.MouseDown += new System.Windows.Forms.MouseEventHandler(this.trayIcon_MouseDown);
            // 
            // contextMenu1
            // 
            this.contextMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem34,
            this.menuItem39,
            this.menuItem38,
            this.menuItem37,
            this.menuItem35,
            this.menuItem36});
            // 
            // menuItem34
            // 
            this.menuItem34.Index = 0;
            this.menuItem34.Text = "Restore";
            this.menuItem34.Click += new System.EventHandler(this.menuItem34_Click);
            // 
            // menuItem39
            // 
            this.menuItem39.Index = 1;
            this.menuItem39.Text = "-";
            // 
            // menuItem38
            // 
            this.menuItem38.Index = 2;
            this.menuItem38.Text = "Options...";
            this.menuItem38.Click += new System.EventHandler(this.menuItem38_Click);
            // 
            // menuItem37
            // 
            this.menuItem37.Index = 3;
            this.menuItem37.Text = "About...";
            this.menuItem37.Click += new System.EventHandler(this.menuItem37_Click);
            // 
            // menuItem35
            // 
            this.menuItem35.Index = 4;
            this.menuItem35.Text = "-";
            // 
            // menuItem36
            // 
            this.menuItem36.Index = 5;
            this.menuItem36.Text = "Exit";
            this.menuItem36.Click += new System.EventHandler(this.menuItem36_Click);
            // 
            // tmrUpdate
            // 
            this.tmrUpdate.Enabled = true;
            this.tmrUpdate.Interval = 500;
            this.tmrUpdate.Tick += new System.EventHandler(this.tmrUpdate_Tick);
            // 
            // pictureMoreInfo
            // 
            this.pictureMoreInfo.Image = ((System.Drawing.Image)(resources.GetObject("pictureMoreInfo.Image")));
            this.pictureMoreInfo.Location = new System.Drawing.Point(66, 287);
            this.pictureMoreInfo.Name = "pictureMoreInfo";
            this.pictureMoreInfo.Size = new System.Drawing.Size(60, 60);
            this.pictureMoreInfo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureMoreInfo.TabIndex = 2;
            this.pictureMoreInfo.TabStop = false;
            this.pictureMoreInfo.Visible = false;
            // 
            // pictureScope
            // 
            this.pictureScope.Image = ((System.Drawing.Image)(resources.GetObject("pictureScope.Image")));
            this.pictureScope.Location = new System.Drawing.Point(160, 285);
            this.pictureScope.Name = "pictureScope";
            this.pictureScope.Size = new System.Drawing.Size(83, 83);
            this.pictureScope.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureScope.TabIndex = 3;
            this.pictureScope.TabStop = false;
            this.pictureScope.Visible = false;
            // 
            // MainDlg
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(682, 529);
            this.Controls.Add(this.pictureScope);
            this.Controls.Add(this.pictureMoreInfo);
            this.Controls.Add(this.statusBar1);
            this.Controls.Add(this.tabControl1);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu1;
            this.MinimumSize = new System.Drawing.Size(655, 556);
            this.Name = "MainDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "FileScope";
            this.Closed += new System.EventHandler(this.main_Closed);
            this.Resize += new System.EventHandler(this.MainDlg_Resize);
            this.Closing += new System.ComponentModel.CancelEventHandler(this.MainDlg_Closing);
            this.LocationChanged += new System.EventHandler(this.MainDlg_LocationChanged);
            this.Load += new System.EventHandler(this.main_Load);
            this.tabControl1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.statusBarConnection)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarDownloads)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarUploads)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarChats)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureMoreInfo)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictureScope)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		private void main_Load(object sender, System.EventArgs e)
		{
			Control c = (Control)this;
			Themes.SetupTheme(c);

            //setup all tab pages
			//home page
			homepage = new HomePage();
			homepage.Dock = DockStyle.Fill;
			tabPage1.Controls.Add(homepage);
			//connection page
			connection = new Connection();
			connection.Dock = DockStyle.Fill;
			tabPage2.Controls.Add(connection);
			//search page
			search = new Search();
			search.Dock = DockStyle.Fill;
			tabPage3.Controls.Add(search);
			//transfers page
			transfers = new Transfers();
			transfers.Dock = DockStyle.Fill;
			tabPage5.Controls.Add(transfers);
			//library page
			library = new Library();
			library.Dock = DockStyle.Fill;
			tabPage6.Controls.Add(library);
			//chat page
			chatpage = new ChatPage();
			chatpage.Dock = DockStyle.Fill;
			tabPage7.Controls.Add(chatpage);

			//reset menus
			UpdateThemeMenu();

			//take care of status bar icons
			rm = new System.Resources.ResourceManager(typeof(MainDlg));
			statusBarDownloads.Icon = null;
			this.imageList1.Images.Add((System.Drawing.Icon)(rm.GetObject("trayIcon.Icon")));
			tabPage1.ImageIndex = 5;

			//networks
           
            if (StartApp.settings.autoGnutella2)
                FileScopeCore.Gnutella2.StartStop.Start();
            
			//start listening
			Listener.Start();

			//check the version
			VersionChecker.Start();

			//see if we can recover all previous downloads
			DownloadManager.RecoverAllDownloads();

			//activate me
			if(StartApp.settings.mainMax)
				this.WindowState = FormWindowState.Maximized;
			this.Activate();
			if(StartApp.settings.alwaysOnTop)
				this.TopMost = true;
			tabControl1.SelectedIndex = 0;

			//homepage stuff
			StartApp.main.homepage.ResetNetworksText();
			StartApp.main.homepage.ResetSearchesText();
			StartApp.main.homepage.ResetTransfersText();
			StartApp.main.homepage.ResetLibraryText();

#if DEBUG
			this.Text += "   --DEBUG";
#endif

			//let everyone know
			Configuration.State.opened = true;

            //CEH help debugging
            string mymode = "Leaf";
            if (Configuration.State.Gnutella2Stats.ultrapeer)
                mymode="UltraPeer";
            this.Text = this.Text + " [" + Configuration.settings.ipAddress.ToString() + ":" + Configuration.settings.port.ToString() + " | " + mymode + "]";
            //CEH added for testing
            //Configuration.gnutella2WebCache.Add("http://www.exagon.org/~crazypops/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://glad04.no-ip.com/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://trancefusionnetwork.org.uk/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://gwcrab2.sarcastro.com:8002/");
            //Configuration.gnutella2WebCache.Add("http://atomo64.webcindario.com/gwc/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://www.student.ru.nl/markjansen/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://gwc.wodi.org/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://www.ciscllc.com/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://pokerface.bishopston.net:3559");
            //Configuration.gnutella2WebCache.Add("http://rssfed23.angeltowns.com/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://g2cache.webcindario.com/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://www.exoticthumbs.biz/g2/bazooka.php");
            //Configuration.gnutella2WebCache.Add("http://twopi.no-ip.org/g2/bazooka.php");

            
            //CEH commented out code below was not done by me

//Gnutella2.G2Data.SetupFuncTable();
//Gnutella2.Sck.scks[Gnutella2.Sck.GetSck()].Reset("192.168.1.105");
	//		Configuration.gnutella2WebCache.Add("http://www20.brinkster.com/dgc2/lynnx.asp");
	//		Configuration.gnutella2WebCache.Add("http://user1.7host.com/dgwc2/lynnx.asp");
	//		Configuration.gnutella2WebCache.Add("http://gwebcache2.jonatkins.com/cgi-bin/gwebcache.cgi");
	//		Configuration.gnutella2WebCache.Add("http://www.newsability.com/Services/GWC2/lynn.asp");
	//		Configuration.gnutella2WebCache.Add("http://g2cache.theg2.net/gwcache/lynnx.asp");
	//		Configuration.gnutella2WebCache.Add("http://gwebcache5.jonatkins.com/cgi-bin/perlgcache.cgi");
		}

		private void MainWindowSizeSettings()
		{
			if(StartApp.settings.mainHeight > 10 && StartApp.settings.mainWidth > 10)
				this.ClientSize = new Size(StartApp.settings.mainWidth, StartApp.settings.mainHeight);
		}

		private void main_Closed(object sender, System.EventArgs e)
		{
			//we just closed the main window; exit program
			StartApp.ExitApp();
		}

		private void menuItem2_Click(object sender, System.EventArgs e)
		{
			//options menu item
			OptionsDlg od = new OptionsDlg();
			od.ShowDialog();
		}

		private void MainDlg_Resize(object sender, System.EventArgs e)
		{
			tabControl1.Width = this.ClientSize.Width;
			tabControl1.Height = this.ClientSize.Height - (tabControl1.Top + statusBar1.Height);
			if(this.WindowState == FormWindowState.Normal && !Configuration.State.closing)
			{
				StartApp.settings.mainHeight = this.ClientSize.Height;
				StartApp.settings.mainWidth = this.ClientSize.Width;
			}
		}

		private void tmrUpdate_Tick(object sender, System.EventArgs e)
		{
			
				statusBarConnection.Icon = (Icon)rm.GetObject("statusBarDownloads.Icon");
                statusBarConnection.Text = "Status: Connected" + " (G2: " + Configuration.State.Gnutella2Stats.lastConnectionCount.ToString(); 
			
			statusBarDownloads.Text = "Downloads: " + Configuration.State.downloadsNow.ToString();
			statusBarUploads.Text = "Uploads: " + Configuration.State.uploadsNow.ToString();
			statusBarChats.Text = "Chats: " + Configuration.State.numChats.ToString();
		}

		private void menuItem4_Click(object sender, System.EventArgs e)
		{
			//minimize menu item
			totray = false;
			this.WindowState = FormWindowState.Minimized;
		}

		private void menuItem7_Click(object sender, System.EventArgs e)
		{
			//exit menu item
			StartApp.ExitApp();
		}

		private void menuItem5_Click(object sender, System.EventArgs e)
		{
			//send to tray menu item
			this.WindowState = FormWindowState.Minimized;
			this.Hide();
		}

		//keep track of whether this form is maximized or not
		bool maximized = false;
		//special flag
		bool hide = false;
		//special minimize flag
		bool totray = true;

		private void trayIcon_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if(this.WindowState == FormWindowState.Minimized)
				this.contextMenu1.MenuItems[0].Text = "Restore";
			else
				this.contextMenu1.MenuItems[0].Text = "Send to tray";
		}

		private void trayIcon_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			switch(e.Button)
			{
				case MouseButtons.Left:
					if(this.Visible == false)
					{
						this.Visible = true;
						if(!maximized)
							this.WindowState = FormWindowState.Normal;
						else
							this.WindowState = FormWindowState.Maximized;
						this.Activate();
					}
					else
					{
						hide = true;
						this.WindowState = FormWindowState.Minimized;
					}
					break;
			}
		}

		/// <summary>
		/// Popup a NewVersionDlg on this UI thread.
		/// </summary>
		public void NewVersion(string version)
		{
			NewVersionDlg nvd = new NewVersionDlg(version);
			nvd.ShowDialog();
		}

		private void MainDlg_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			if(!StartApp.settings.closeNormal)
			{
				e.Cancel = true;
				hide = true;
				this.WindowState = FormWindowState.Minimized;
			}
		}

		private void MainDlg_LocationChanged(object sender, System.EventArgs e)
		{
			if(!Configuration.State.opened)
				return;
			if(this.WindowState == FormWindowState.Normal)
			{
				maximized = false;
				this.statusBar1.SizingGrip = true;
				StartApp.settings.mainMax = false;
			}
			else if(this.WindowState == FormWindowState.Maximized)
			{
				maximized = true;
				this.statusBar1.SizingGrip = false;
				StartApp.settings.mainMax = true;
			}
			else if(this.WindowState == FormWindowState.Minimized)
			{
				if((!StartApp.settings.minimizeNormal || hide) && totray)
				{
					hide = false;
					this.Hide();
				}
				else if(!totray)
					totray = true;
			}
		}

		

		
		private void menuItem40_Click(object sender, System.EventArgs e)
		{
            if (!FileScopeCore.Gnutella2.StartStop.enabled)
                FileScopeCore.Gnutella2.StartStop.Start();
		}

		private void menuItem41_Click(object sender, System.EventArgs e)
		{
            if (FileScopeCore.Gnutella2.StartStop.enabled)
                FileScopeCore.Gnutella2.StartStop.Stop();
		}


		void UpdateThemeMenu()
		{
			menuItem15.Checked = (StartApp.settings.theme == menuItem15.Text);
			menuItem27.Checked = (StartApp.settings.theme == menuItem27.Text);
			menuItem31.Checked = (StartApp.settings.theme == menuItem31.Text);
			menuItem32.Checked = (StartApp.settings.theme == menuItem32.Text);
			menuItem16.Checked = (StartApp.settings.theme == menuItem16.Text);
			menuItem33.Checked = (StartApp.settings.theme == menuItem33.Text);
			menuItem24.Checked = (StartApp.settings.theme == menuItem24.Text);
			menuItem26.Checked = (StartApp.settings.theme == menuItem26.Text);
			menuItem28.Checked = (StartApp.settings.theme == menuItem28.Text);
			menuItem29.Checked = (StartApp.settings.theme == menuItem29.Text);
		}

		void ApplyTheme()
		{
			Control homepag = (Control)homepage;
			Control connectio = (Control)connection;
			Control searc = (Control)search;
			Control transfer = (Control)transfers;
			Control librar = (Control)library;
			Control chatpag = (Control)chatpage;
			Themes.SetupTheme((Control)this);
			Themes.SetupTheme(homepag);
			Themes.SetupTheme(connectio);
			Themes.SetupTheme(searc);
			Themes.SetupTheme(transfer);
			Themes.SetupTheme(librar);
			Themes.SetupTheme(chatpag);
			homepage.ResetBrushes();
		}

		void menuThemeSwitch(string themeName)
		{
			Themes.SetColors(themeName);
			UpdateThemeMenu();
			ApplyTheme();
            LoadSaveSettings.SaveSettings();
		}

		private void menuItem15_Click(object sender, System.EventArgs e)
		{
System.Diagnostics.Debug.WriteLine(string.Format("{0:#,###}", GC.GetTotalMemory(true)) + " bytes");
			menuThemeSwitch(menuItem15.Text);
		}

		private void menuItem27_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem27.Text);
		}

		private void menuItem31_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem31.Text);
		}

		private void menuItem32_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem32.Text);
		}

		private void menuItem16_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem16.Text);
		}

		private void menuItem33_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem33.Text);
		}

		private void menuItem24_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem24.Text);
		}

		private void menuItem28_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem28.Text);
		}

		private void menuItem26_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem26.Text);
		}

		private void menuItem29_Click(object sender, System.EventArgs e)
		{
			menuThemeSwitch(menuItem29.Text);
		}

		private void menuItem18_Click(object sender, System.EventArgs e)
		{
			Utils.SpawnLink("http://www.filescope.com");
		}

		private void menuItem20_Click(object sender, System.EventArgs e)
		{
			Utils.SpawnLink("http://www.filescope.com/manual.htm");
		}

		private void menuItem21_Click(object sender, System.EventArgs e)
		{
			Utils.SpawnLink("http://www.filescope.com/faq.htm");
		}

		private void menuItem23_Click(object sender, System.EventArgs e)
		{
			//about
			AboutDlg ad = new AboutDlg();
			ad.ShowDialog();
		}

		private void menuItem34_Click(object sender, System.EventArgs e)
		{
			//restore/send to tray
			if(this.Visible == false)
			{
				this.Visible = true;
				if(!maximized)
					this.WindowState = FormWindowState.Normal;
				else
					this.WindowState = FormWindowState.Maximized;
				this.Activate();
			}
			else
			{
				hide = true;
				this.WindowState = FormWindowState.Minimized;
			}
		}

		private void menuItem38_Click(object sender, System.EventArgs e)
		{
			//options
			OptionsDlg od = new OptionsDlg();
			od.StartPosition = FormStartPosition.CenterScreen;
			od.ShowDialog();
		}

		private void menuItem37_Click(object sender, System.EventArgs e)
		{
			//about
			AboutDlg ad = new AboutDlg();
			ad.StartPosition = FormStartPosition.CenterScreen;
			ad.ShowDialog();
		}

		private void menuItem36_Click(object sender, System.EventArgs e)
		{
			//exit
			StartApp.ExitApp();
		}

		private void menuItem45_Click(object sender, System.EventArgs e)
		{
			//download url
			DownloadUrl du = new DownloadUrl();
			du.StartPosition = FormStartPosition.CenterScreen;
			du.Show();
		}

		private void menuItem47_Click(object sender, System.EventArgs e)
		{
			//browse host
			string browseHost = InputBox.Show("Enter the IP Address of a Gnutella 2 host that you wish to browse");
			if(browseHost.Length > 0)
				search.BrowseHost(browseHost);
		}

		private void menuItem46_Click(object sender, System.EventArgs e)
		{
			//chat with host
			string chatHost = InputBox.Show("Enter the IP Address of a Gnutella (1 or 2) host that you wish to chat with");
			if(chatHost.Length > 0)
				ChatManager.Outgoing(ref chatHost);
		}

		

		private void menuItem51_Click(object sender, System.EventArgs e)
		{
			MessageBox.Show("If you feel that something is not working properly in FileScope and you couldn't find any kind of explanation in the FAQs section of FileScope.com, please feel free to email your observations to mz392@nyu.edu");
		}

		public static int selectedIndexMain = 0;

		private void tabControl1_SelectedIndexChanged(object sender, System.EventArgs e)
		{
			if(connection != null)
				connection.tabControl1_SelectedIndexChanged(null, null);
			selectedIndexMain = tabControl1.SelectedIndex;
		}
	}
}
