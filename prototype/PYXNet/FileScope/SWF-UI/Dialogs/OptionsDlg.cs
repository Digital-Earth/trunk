// OptionsDlg.cs
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
using FileScopeCore;

namespace FileScope
{
	/// <summary>
	/// Options window.
	/// </summary>
	public class OptionsDlg : System.Windows.Forms.Form
	{
		private System.Windows.Forms.TreeView treeView1;
		private FileScope.ElTabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.TabPage tabPage5;
		private System.Windows.Forms.TabPage tabPage6;
		private System.Windows.Forms.TabPage tabPage7;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.CheckBox chkUpdates;
		private System.Windows.Forms.CheckBox chkOnTop;
		private System.Windows.Forms.CheckBox chkSwitchTransfers;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.RadioButton radioMin1;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.RadioButton radioMin2;
		private System.Windows.Forms.RadioButton radioClose2;
		private System.Windows.Forms.RadioButton radioClose1;
		private System.Windows.Forms.GroupBox groupBox17;
		private System.Windows.Forms.TextBox textBoxPort;
		private System.Windows.Forms.Label label2343;
		private System.Windows.Forms.CheckBox chkFirewall;
		private System.Windows.Forms.GroupBox groupBox22;
		private System.Windows.Forms.RadioButton radioT3;
		private System.Windows.Forms.RadioButton radioT1;
		private System.Windows.Forms.RadioButton radioCable;
        private System.Windows.Forms.RadioButton radioDialup;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.Button buttonBrowse;
		private System.Windows.Forms.TextBox textDownload;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.CheckBox chkFileAlert;
		private System.Windows.Forms.CheckBox chkClearUp;
		private System.Windows.Forms.CheckBox chkClearDl;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.ListBox listShares;
		private System.Windows.Forms.Button button10;
		private System.Windows.Forms.Button button9;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.CheckBox chkChats;
		private System.Windows.Forms.GroupBox groupBox5;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button button3;
		private System.Windows.Forms.Button button4;
        private System.Windows.Forms.ListBox listBoxChats;
		private System.Windows.Forms.CheckBox chkCancelDLAlert;
		private System.Windows.Forms.GroupBox groupBox7;
		private System.Windows.Forms.Button button7;
		private System.Windows.Forms.Button button8;
        private System.Windows.Forms.ListBox listBoxWebCache2;
        private TabPage tabPage3;
        private CheckBox chkAutoGnutella2;
        private CheckBox chkAllowUltrapeer;
		private System.ComponentModel.Container components = null;

		public OptionsDlg()
		{
			InitializeComponent();
			Control c = (Control)this;
			Themes.SetupTheme(c);
			if(StartApp.settings.alwaysOnTop)
				this.TopMost = true;
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
            System.Windows.Forms.TreeNode treeNode1 = new System.Windows.Forms.TreeNode("Interface");
            System.Windows.Forms.TreeNode treeNode2 = new System.Windows.Forms.TreeNode("Network");
            System.Windows.Forms.TreeNode treeNode3 = new System.Windows.Forms.TreeNode("Connection");
            System.Windows.Forms.TreeNode treeNode4 = new System.Windows.Forms.TreeNode("Transfers");
            System.Windows.Forms.TreeNode treeNode5 = new System.Windows.Forms.TreeNode("Sharing");
            System.Windows.Forms.TreeNode treeNode6 = new System.Windows.Forms.TreeNode("Chat");
            System.Windows.Forms.TreeNode treeNode7 = new System.Windows.Forms.TreeNode("WebCaches");
            this.treeView1 = new System.Windows.Forms.TreeView();
            this.tabControl1 = new FileScope.ElTabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.radioClose2 = new System.Windows.Forms.RadioButton();
            this.radioClose1 = new System.Windows.Forms.RadioButton();
            this.label2 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.radioMin2 = new System.Windows.Forms.RadioButton();
            this.radioMin1 = new System.Windows.Forms.RadioButton();
            this.label1 = new System.Windows.Forms.Label();
            this.chkSwitchTransfers = new System.Windows.Forms.CheckBox();
            this.chkOnTop = new System.Windows.Forms.CheckBox();
            this.chkUpdates = new System.Windows.Forms.CheckBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.groupBox17 = new System.Windows.Forms.GroupBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label2343 = new System.Windows.Forms.Label();
            this.chkFirewall = new System.Windows.Forms.CheckBox();
            this.groupBox22 = new System.Windows.Forms.GroupBox();
            this.radioT3 = new System.Windows.Forms.RadioButton();
            this.radioT1 = new System.Windows.Forms.RadioButton();
            this.radioCable = new System.Windows.Forms.RadioButton();
            this.radioDialup = new System.Windows.Forms.RadioButton();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.chkCancelDLAlert = new System.Windows.Forms.CheckBox();
            this.chkClearUp = new System.Windows.Forms.CheckBox();
            this.chkClearDl = new System.Windows.Forms.CheckBox();
            this.chkFileAlert = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.buttonBrowse = new System.Windows.Forms.Button();
            this.textDownload = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.listShares = new System.Windows.Forms.ListBox();
            this.button10 = new System.Windows.Forms.Button();
            this.button9 = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.button3 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.listBoxChats = new System.Windows.Forms.ListBox();
            this.label3 = new System.Windows.Forms.Label();
            this.chkChats = new System.Windows.Forms.CheckBox();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.button7 = new System.Windows.Forms.Button();
            this.button8 = new System.Windows.Forms.Button();
            this.listBoxWebCache2 = new System.Windows.Forms.ListBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.chkAllowUltrapeer = new System.Windows.Forms.CheckBox();
            this.chkAutoGnutella2 = new System.Windows.Forms.CheckBox();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.groupBox17.SuspendLayout();
            this.groupBox22.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.groupBox7.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeView1
            // 
            this.treeView1.HotTracking = true;
            this.treeView1.Location = new System.Drawing.Point(0, 0);
            this.treeView1.Name = "treeView1";
            treeNode1.Name = "";
            treeNode1.Text = "Interface";
            treeNode2.Name = "";
            treeNode2.Text = "Network";
            treeNode3.Name = "";
            treeNode3.Text = "Connection";
            treeNode4.Name = "";
            treeNode4.Text = "Transfers";
            treeNode5.Name = "";
            treeNode5.Text = "Sharing";
            treeNode6.Name = "";
            treeNode6.Text = "Chat";
            treeNode7.Name = "";
            treeNode7.Text = "WebCaches";
            this.treeView1.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
            treeNode1,
            treeNode2,
            treeNode3,
            treeNode4,
            treeNode5,
            treeNode6,
            treeNode7});
            this.treeView1.Scrollable = false;
            this.treeView1.Size = new System.Drawing.Size(104, 344);
            this.treeView1.TabIndex = 0;
            this.treeView1.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView1_AfterSelect);
            // 
            // tabControl1
            // 
            this.tabControl1.Appearance = System.Windows.Forms.TabAppearance.FlatButtons;
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage4);
            this.tabControl1.Controls.Add(this.tabPage5);
            this.tabControl1.Controls.Add(this.tabPage6);
            this.tabControl1.Controls.Add(this.tabPage7);
            this.tabControl1.Location = new System.Drawing.Point(104, -24);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(400, 320);
            this.tabControl1.TabIndex = 1;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBox2);
            this.tabPage1.Controls.Add(this.groupBox1);
            this.tabPage1.Controls.Add(this.chkSwitchTransfers);
            this.tabPage1.Controls.Add(this.chkOnTop);
            this.tabPage1.Controls.Add(this.chkUpdates);
            this.tabPage1.Location = new System.Drawing.Point(4, 25);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Size = new System.Drawing.Size(392, 291);
            this.tabPage1.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.radioClose2);
            this.groupBox2.Controls.Add(this.radioClose1);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.groupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox2.Location = new System.Drawing.Point(32, 192);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(328, 80);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Closing";
            // 
            // radioClose2
            // 
            this.radioClose2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioClose2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioClose2.Location = new System.Drawing.Point(200, 48);
            this.radioClose2.Name = "radioClose2";
            this.radioClose2.Size = new System.Drawing.Size(88, 16);
            this.radioClose2.TabIndex = 2;
            this.radioClose2.Text = "Exit";
            // 
            // radioClose1
            // 
            this.radioClose1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioClose1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioClose1.Location = new System.Drawing.Point(200, 32);
            this.radioClose1.Name = "radioClose1";
            this.radioClose1.Size = new System.Drawing.Size(88, 16);
            this.radioClose1.TabIndex = 1;
            this.radioClose1.Text = "Send to tray";
            // 
            // label2
            // 
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label2.Location = new System.Drawing.Point(16, 32);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(144, 32);
            this.label2.TabIndex = 0;
            this.label2.Text = "When user tries to close the program, it should:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.radioMin2);
            this.groupBox1.Controls.Add(this.radioMin1);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox1.Location = new System.Drawing.Point(32, 96);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(328, 80);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Minimizing";
            // 
            // radioMin2
            // 
            this.radioMin2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioMin2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioMin2.Location = new System.Drawing.Point(184, 48);
            this.radioMin2.Name = "radioMin2";
            this.radioMin2.Size = new System.Drawing.Size(88, 16);
            this.radioMin2.TabIndex = 2;
            this.radioMin2.Text = "Send to tray";
            // 
            // radioMin1
            // 
            this.radioMin1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioMin1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioMin1.Location = new System.Drawing.Point(184, 32);
            this.radioMin1.Name = "radioMin1";
            this.radioMin1.Size = new System.Drawing.Size(120, 16);
            this.radioMin1.TabIndex = 1;
            this.radioMin1.Text = "Minimize to taskbar";
            // 
            // label1
            // 
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label1.Location = new System.Drawing.Point(16, 32);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(144, 32);
            this.label1.TabIndex = 0;
            this.label1.Text = "When user tries to minimize the program, it should:";
            // 
            // chkSwitchTransfers
            // 
            this.chkSwitchTransfers.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkSwitchTransfers.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkSwitchTransfers.Location = new System.Drawing.Point(72, 64);
            this.chkSwitchTransfers.Name = "chkSwitchTransfers";
            this.chkSwitchTransfers.Size = new System.Drawing.Size(248, 16);
            this.chkSwitchTransfers.TabIndex = 2;
            this.chkSwitchTransfers.Text = "Switch to transfers page on new downloads";
            // 
            // chkOnTop
            // 
            this.chkOnTop.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkOnTop.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkOnTop.Location = new System.Drawing.Point(72, 40);
            this.chkOnTop.Name = "chkOnTop";
            this.chkOnTop.Size = new System.Drawing.Size(248, 16);
            this.chkOnTop.TabIndex = 1;
            this.chkOnTop.Text = "FileScope always on top of all other windows";
            // 
            // chkUpdates
            // 
            this.chkUpdates.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkUpdates.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkUpdates.Location = new System.Drawing.Point(72, 16);
            this.chkUpdates.Name = "chkUpdates";
            this.chkUpdates.Size = new System.Drawing.Size(208, 16);
            this.chkUpdates.TabIndex = 0;
            this.chkUpdates.Text = "Notify me of new FileScope updates";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.groupBox17);
            this.tabPage2.Controls.Add(this.groupBox22);
            this.tabPage2.Location = new System.Drawing.Point(4, 25);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Size = new System.Drawing.Size(392, 291);
            this.tabPage2.TabIndex = 1;
            // 
            // groupBox17
            // 
            this.groupBox17.Controls.Add(this.textBoxPort);
            this.groupBox17.Controls.Add(this.label2343);
            this.groupBox17.Controls.Add(this.chkFirewall);
            this.groupBox17.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox17.Location = new System.Drawing.Point(40, 152);
            this.groupBox17.Name = "groupBox17";
            this.groupBox17.Size = new System.Drawing.Size(312, 88);
            this.groupBox17.TabIndex = 12;
            this.groupBox17.TabStop = false;
            this.groupBox17.Text = "Incoming Connections";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.textBoxPort.Location = new System.Drawing.Point(170, 61);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(46, 20);
            this.textBoxPort.TabIndex = 2;
            this.textBoxPort.Text = "6346";
            // 
            // label2343
            // 
            this.label2343.AutoSize = true;
            this.label2343.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label2343.Location = new System.Drawing.Point(32, 64);
            this.label2343.Name = "label2343";
            this.label2343.Size = new System.Drawing.Size(135, 13);
            this.label2343.TabIndex = 1;
            this.label2343.Text = "Incoming connections port:";
            // 
            // chkFirewall
            // 
            this.chkFirewall.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkFirewall.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkFirewall.Location = new System.Drawing.Point(32, 24);
            this.chkFirewall.Name = "chkFirewall";
            this.chkFirewall.Size = new System.Drawing.Size(256, 32);
            this.chkFirewall.TabIndex = 0;
            this.chkFirewall.Text = "I know that I am behind a firewall and I cannot accept incoming connections";
            // 
            // groupBox22
            // 
            this.groupBox22.Controls.Add(this.radioT3);
            this.groupBox22.Controls.Add(this.radioT1);
            this.groupBox22.Controls.Add(this.radioCable);
            this.groupBox22.Controls.Add(this.radioDialup);
            this.groupBox22.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox22.Location = new System.Drawing.Point(40, 16);
            this.groupBox22.Name = "groupBox22";
            this.groupBox22.Size = new System.Drawing.Size(312, 120);
            this.groupBox22.TabIndex = 11;
            this.groupBox22.TabStop = false;
            this.groupBox22.Text = "Connection Type";
            // 
            // radioT3
            // 
            this.radioT3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioT3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioT3.Location = new System.Drawing.Point(96, 96);
            this.radioT3.Name = "radioT3";
            this.radioT3.Size = new System.Drawing.Size(120, 16);
            this.radioT3.TabIndex = 4;
            this.radioT3.Text = "T3 Line or higher";
            // 
            // radioT1
            // 
            this.radioT1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioT1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioT1.Location = new System.Drawing.Point(96, 72);
            this.radioT1.Name = "radioT1";
            this.radioT1.Size = new System.Drawing.Size(120, 16);
            this.radioT1.TabIndex = 3;
            this.radioT1.Text = "T1 Line";
            // 
            // radioCable
            // 
            this.radioCable.Checked = true;
            this.radioCable.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioCable.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioCable.Location = new System.Drawing.Point(96, 48);
            this.radioCable.Name = "radioCable";
            this.radioCable.Size = new System.Drawing.Size(120, 16);
            this.radioCable.TabIndex = 1;
            this.radioCable.TabStop = true;
            this.radioCable.Text = "Cable/DSL Modem";
            // 
            // radioDialup
            // 
            this.radioDialup.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.radioDialup.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.radioDialup.Location = new System.Drawing.Point(96, 24);
            this.radioDialup.Name = "radioDialup";
            this.radioDialup.Size = new System.Drawing.Size(120, 16);
            this.radioDialup.TabIndex = 0;
            this.radioDialup.Text = "Dialup Modem";
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.chkCancelDLAlert);
            this.tabPage4.Controls.Add(this.chkClearUp);
            this.tabPage4.Controls.Add(this.chkClearDl);
            this.tabPage4.Controls.Add(this.chkFileAlert);
            this.tabPage4.Controls.Add(this.groupBox3);
            this.tabPage4.Location = new System.Drawing.Point(4, 25);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Size = new System.Drawing.Size(392, 291);
            this.tabPage4.TabIndex = 3;
            // 
            // chkCancelDLAlert
            // 
            this.chkCancelDLAlert.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkCancelDLAlert.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkCancelDLAlert.Location = new System.Drawing.Point(64, 112);
            this.chkCancelDLAlert.Name = "chkCancelDLAlert";
            this.chkCancelDLAlert.Size = new System.Drawing.Size(264, 16);
            this.chkCancelDLAlert.TabIndex = 14;
            this.chkCancelDLAlert.Text = "Alert me when attempting to cancel downloads";
            // 
            // chkClearUp
            // 
            this.chkClearUp.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkClearUp.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkClearUp.Location = new System.Drawing.Point(64, 184);
            this.chkClearUp.Name = "chkClearUp";
            this.chkClearUp.Size = new System.Drawing.Size(248, 16);
            this.chkClearUp.TabIndex = 13;
            this.chkClearUp.Text = "Automatically clear completed uploads";
            // 
            // chkClearDl
            // 
            this.chkClearDl.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkClearDl.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkClearDl.Location = new System.Drawing.Point(64, 160);
            this.chkClearDl.Name = "chkClearDl";
            this.chkClearDl.Size = new System.Drawing.Size(248, 16);
            this.chkClearDl.TabIndex = 12;
            this.chkClearDl.Text = "Automatically clear completed downloads";
            // 
            // chkFileAlert
            // 
            this.chkFileAlert.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkFileAlert.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkFileAlert.Location = new System.Drawing.Point(64, 136);
            this.chkFileAlert.Name = "chkFileAlert";
            this.chkFileAlert.Size = new System.Drawing.Size(264, 16);
            this.chkFileAlert.TabIndex = 11;
            this.chkFileAlert.Text = "Alert me when downloading dangerous file types";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.buttonBrowse);
            this.groupBox3.Controls.Add(this.textDownload);
            this.groupBox3.Controls.Add(this.label5);
            this.groupBox3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox3.Location = new System.Drawing.Point(8, 8);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(376, 88);
            this.groupBox3.TabIndex = 10;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Download Directory";
            // 
            // buttonBrowse
            // 
            this.buttonBrowse.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.buttonBrowse.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.buttonBrowse.Location = new System.Drawing.Point(304, 48);
            this.buttonBrowse.Name = "buttonBrowse";
            this.buttonBrowse.Size = new System.Drawing.Size(56, 20);
            this.buttonBrowse.TabIndex = 2;
            this.buttonBrowse.Text = "Browse";
            this.buttonBrowse.Click += new System.EventHandler(this.buttonBrowse_Click);
            // 
            // textDownload
            // 
            this.textDownload.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.textDownload.Location = new System.Drawing.Point(16, 48);
            this.textDownload.Name = "textDownload";
            this.textDownload.Size = new System.Drawing.Size(288, 20);
            this.textDownload.TabIndex = 1;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label5.Location = new System.Drawing.Point(16, 32);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(230, 13);
            this.label5.TabIndex = 0;
            this.label5.Text = "Directory where downloaded files will be saved:";
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.groupBox4);
            this.tabPage5.Location = new System.Drawing.Point(4, 25);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Size = new System.Drawing.Size(392, 291);
            this.tabPage5.TabIndex = 4;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.listShares);
            this.groupBox4.Controls.Add(this.button10);
            this.groupBox4.Controls.Add(this.button9);
            this.groupBox4.Controls.Add(this.label6);
            this.groupBox4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox4.Location = new System.Drawing.Point(8, 8);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(376, 278);
            this.groupBox4.TabIndex = 11;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Shared Directories";
            // 
            // listShares
            // 
            this.listShares.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.listShares.Location = new System.Drawing.Point(8, 72);
            this.listShares.Name = "listShares";
            this.listShares.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.listShares.Size = new System.Drawing.Size(360, 199);
            this.listShares.TabIndex = 4;
            // 
            // button10
            // 
            this.button10.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button10.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.button10.Location = new System.Drawing.Point(192, 44);
            this.button10.Name = "button10";
            this.button10.Size = new System.Drawing.Size(144, 20);
            this.button10.TabIndex = 3;
            this.button10.Text = "Remove selected folders";
            this.button10.Click += new System.EventHandler(this.button10_Click);
            // 
            // button9
            // 
            this.button9.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button9.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.button9.Location = new System.Drawing.Point(40, 44);
            this.button9.Name = "button9";
            this.button9.Size = new System.Drawing.Size(136, 20);
            this.button9.TabIndex = 2;
            this.button9.Text = "Add a folder to share";
            this.button9.Click += new System.EventHandler(this.button9_Click);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label6.Location = new System.Drawing.Point(48, 22);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(267, 13);
            this.label6.TabIndex = 0;
            this.label6.Text = "What folders do you want to contribute to the network?";
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.groupBox5);
            this.tabPage6.Controls.Add(this.chkChats);
            this.tabPage6.Location = new System.Drawing.Point(4, 25);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Size = new System.Drawing.Size(392, 291);
            this.tabPage6.TabIndex = 5;
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.button3);
            this.groupBox5.Controls.Add(this.button4);
            this.groupBox5.Controls.Add(this.listBoxChats);
            this.groupBox5.Controls.Add(this.label3);
            this.groupBox5.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox5.Location = new System.Drawing.Point(72, 32);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(248, 256);
            this.groupBox5.TabIndex = 1;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Blocked Chat Hosts";
            // 
            // button3
            // 
            this.button3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button3.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button3.Location = new System.Drawing.Point(128, 224);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(88, 24);
            this.button3.TabIndex = 5;
            this.button3.Text = "Remove";
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button4
            // 
            this.button4.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button4.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button4.Location = new System.Drawing.Point(32, 224);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(88, 24);
            this.button4.TabIndex = 4;
            this.button4.Text = "Add";
            this.button4.Click += new System.EventHandler(this.button4_Click);
            // 
            // listBoxChats
            // 
            this.listBoxChats.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.listBoxChats.Location = new System.Drawing.Point(40, 48);
            this.listBoxChats.Name = "listBoxChats";
            this.listBoxChats.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.listBoxChats.Size = new System.Drawing.Size(168, 173);
            this.listBoxChats.TabIndex = 1;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.label3.Location = new System.Drawing.Point(16, 24);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(210, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Prevent these hosts from chatting with you:";
            // 
            // chkChats
            // 
            this.chkChats.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkChats.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkChats.Location = new System.Drawing.Point(128, 8);
            this.chkChats.Name = "chkChats";
            this.chkChats.Size = new System.Drawing.Size(128, 16);
            this.chkChats.TabIndex = 0;
            this.chkChats.Text = "Allow incoming chats";
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.groupBox7);
            this.tabPage7.Location = new System.Drawing.Point(4, 25);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Size = new System.Drawing.Size(392, 291);
            this.tabPage7.TabIndex = 6;
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.button7);
            this.groupBox7.Controls.Add(this.button8);
            this.groupBox7.Controls.Add(this.listBoxWebCache2);
            this.groupBox7.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.groupBox7.Location = new System.Drawing.Point(8, 7);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(376, 136);
            this.groupBox7.TabIndex = 3;
            this.groupBox7.TabStop = false;
            this.groupBox7.Text = "Gnutella 2 WebCache Servers";
            // 
            // button7
            // 
            this.button7.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button7.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button7.Location = new System.Drawing.Point(192, 104);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(88, 24);
            this.button7.TabIndex = 5;
            this.button7.Text = "Remove";
            this.button7.Click += new System.EventHandler(this.button7_Click);
            // 
            // button8
            // 
            this.button8.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button8.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button8.Location = new System.Drawing.Point(96, 104);
            this.button8.Name = "button8";
            this.button8.Size = new System.Drawing.Size(88, 24);
            this.button8.TabIndex = 4;
            this.button8.Text = "Add";
            this.button8.Click += new System.EventHandler(this.button8_Click);
            // 
            // listBoxWebCache2
            // 
            this.listBoxWebCache2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.listBoxWebCache2.Location = new System.Drawing.Point(16, 16);
            this.listBoxWebCache2.Name = "listBoxWebCache2";
            this.listBoxWebCache2.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.listBoxWebCache2.Size = new System.Drawing.Size(344, 82);
            this.listBoxWebCache2.TabIndex = 1;
            // 
            // button1
            // 
            this.button1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button1.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button1.Location = new System.Drawing.Point(161, 304);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(88, 32);
            this.button1.TabIndex = 2;
            this.button1.Text = "OK";
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button2.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.button2.Location = new System.Drawing.Point(257, 304);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(88, 32);
            this.button2.TabIndex = 3;
            this.button2.Text = "Cancel";
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // chkAllowUltrapeer
            // 
            this.chkAllowUltrapeer.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkAllowUltrapeer.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkAllowUltrapeer.Location = new System.Drawing.Point(40, 24);
            this.chkAllowUltrapeer.Name = "chkAllowUltrapeer";
            this.chkAllowUltrapeer.Size = new System.Drawing.Size(240, 16);
            this.chkAllowUltrapeer.TabIndex = 3;
            this.chkAllowUltrapeer.Text = "Allow FileScope to function as an ultrapeer";
            // 
            // chkAutoGnutella2
            // 
            this.chkAutoGnutella2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.chkAutoGnutella2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
            this.chkAutoGnutella2.Location = new System.Drawing.Point(40, 52);
            this.chkAutoGnutella2.Name = "chkAutoGnutella2";
            this.chkAutoGnutella2.Size = new System.Drawing.Size(304, 16);
            this.chkAutoGnutella2.TabIndex = 7;
            this.chkAutoGnutella2.Text = "Automatically connect to Gnutella2 on FileScope startup";
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.chkAutoGnutella2);
            this.tabPage3.Controls.Add(this.chkAllowUltrapeer);
            this.tabPage3.Location = new System.Drawing.Point(4, 25);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Size = new System.Drawing.Size(392, 291);
            this.tabPage3.TabIndex = 2;
            // 
            // OptionsDlg
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(506, 344);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.treeView1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "OptionsDlg";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Options";
            this.Load += new System.EventHandler(this.OptionsDlg_Load);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.groupBox17.ResumeLayout(false);
            this.groupBox17.PerformLayout();
            this.groupBox22.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.tabPage5.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.tabPage6.ResumeLayout(false);
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.tabPage7.ResumeLayout(false);
            this.groupBox7.ResumeLayout(false);
            this.tabPage3.ResumeLayout(false);
            this.ResumeLayout(false);

		}
		#endregion

		private void button2_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void treeView1_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e)
		{
			switch(e.Node.Text)
			{
				case "Interface":
					tabControl1.SelectedIndex = 0;
					break;
				case "Network":
					tabControl1.SelectedIndex = 1;
					break;
				case "Connection":
					tabControl1.SelectedIndex = 2;
					break;
				case "Transfers":
					tabControl1.SelectedIndex = 3;
					break;
				case "Sharing":
					tabControl1.SelectedIndex = 4;
					break;
				case "Chat":
					tabControl1.SelectedIndex = 5;
					break;
				case "WebCaches":
					tabControl1.SelectedIndex = 6;
					break;
			}
		}

		private void OptionsDlg_Load(object sender, System.EventArgs e)
		{
			//Interface
			chkUpdates.Checked = StartApp.settings.updateNotify;
			chkOnTop.Checked = StartApp.settings.alwaysOnTop;
			chkSwitchTransfers.Checked = StartApp.settings.switchTransfers;
			if(StartApp.settings.minimizeNormal)
				radioMin1.Checked = true;
			else
				radioMin2.Checked = true;
			if(StartApp.settings.closeNormal)
				radioClose2.Checked = true;
			else
				radioClose1.Checked = true;

			//Network
            switch (Configuration.settings.connectionType)
			{
				case EnumConnectionType.dialUp:
					radioDialup.Checked = true;
					break;
				case EnumConnectionType.cable:
					radioCable.Checked = true;
					break;
				case EnumConnectionType.t1:
					radioT1.Checked = true;
					break;
				case EnumConnectionType.t3:
					radioT3.Checked = true;
					break;
			}
            chkFirewall.Checked = Configuration.settings.firewall;
            textBoxPort.Text = Configuration.settings.port.ToString();

			//Connection
			chkAllowUltrapeer.Checked = Configuration.settings.allowUltrapeer;
            chkAutoGnutella2.Checked = StartApp.settings.autoGnutella2;

			//Transfers
            textDownload.Text = Configuration.settings.dlDirectory;
			chkFileAlert.Checked = StartApp.settings.fileAlert;
			chkClearDl.Checked = StartApp.settings.clearDl;
			chkClearUp.Checked = StartApp.settings.clearUp;
			chkCancelDLAlert.Checked = StartApp.settings.cancelDLAlert;

			//Sharing
			lock(Configuration.shareList)
			{
				foreach(object shareFolder in Configuration.shareList)
					listShares.Items.Add(shareFolder);
			}

			//Chat
            chkChats.Checked = Configuration.settings.allowChats;
			lock(Configuration.blockedChatHosts)
			{
				foreach(object blockedHost in Configuration.blockedChatHosts)
					listBoxChats.Items.Add(blockedHost);
			}

			//WebCaches
			
			lock(Configuration.gnutella2WebCache)
			{
				foreach(object webCache in Configuration.gnutella2WebCache)
					listBoxWebCache2.Items.Add(webCache);
			}
		}

		/// <summary>
		/// The ok button was pressed; we save all settings.
		/// </summary>
		private void button1_Click(object sender, System.EventArgs e)
		{
			//we need to check some stuff first
			try
			{
				int portNum = Convert.ToInt32(textBoxPort.Text);
				if(portNum < 10 || portNum > 65530)
				{
					MessageBox.Show("Invalid port number!");
					return;
				}
			}
			catch
			{
				MessageBox.Show("Invalid port number!");
				return;
			}
			if(!System.IO.Directory.Exists(textDownload.Text))
			{
				MessageBox.Show("The download directory does not exist!");
				return;
			}

			//if we got this far, everything is ready to save

			//Interface
			StartApp.settings.updateNotify = chkUpdates.Checked;
			StartApp.settings.alwaysOnTop = chkOnTop.Checked;
			StartApp.settings.switchTransfers = chkSwitchTransfers.Checked;
			StartApp.settings.minimizeNormal = radioMin1.Checked;
			StartApp.settings.closeNormal = radioClose2.Checked;

			//Network
			if(radioDialup.Checked)
                Configuration.settings.connectionType = EnumConnectionType.dialUp;
			else if(radioCable.Checked)
                Configuration.settings.connectionType = EnumConnectionType.cable;
			else if(radioT1.Checked)
                Configuration.settings.connectionType = EnumConnectionType.t1;
			else
                Configuration.settings.connectionType = EnumConnectionType.t3;
            Configuration.settings.firewall = chkFirewall.Checked;
            if (Convert.ToInt32(textBoxPort.Text) != Configuration.settings.port)
			{
				Listener.Abort();
                Configuration.settings.port = Convert.ToInt32(textBoxPort.Text);
				Listener.Start();
			}

			//Connection
            Configuration.settings.allowUltrapeer = chkAllowUltrapeer.Checked;
            StartApp.settings.autoGnutella2 = chkAutoGnutella2.Checked;

			//Transfers
            Configuration.settings.dlDirectory = textDownload.Text;
			StartApp.settings.fileAlert = chkFileAlert.Checked;
			StartApp.settings.clearDl = chkClearDl.Checked;
			StartApp.settings.clearUp = chkClearUp.Checked;
			StartApp.settings.cancelDLAlert = chkCancelDLAlert.Checked;

			//Sharing
			lock(Configuration.shareList)
			{
				Configuration.shareList.Clear();
				foreach(object shareFolder in listShares.Items)
					Configuration.shareList.Add(shareFolder);
			}
			Configuration.UpdateShares();

			//Chat
            Configuration.settings.allowChats = chkChats.Checked;
			lock(Configuration.blockedChatHosts)
			{
				Configuration.blockedChatHosts.Clear();
				foreach(object blockedHost in listBoxChats.Items)
					Configuration.blockedChatHosts.Add(blockedHost);
			}

			//WebCaches
			
			lock(Configuration.gnutella2WebCache)
			{
				Configuration.gnutella2WebCache.Clear();
				foreach(object webCache in listBoxWebCache2.Items)
					Configuration.gnutella2WebCache.Add(webCache);
			}

			if(StartApp.settings.alwaysOnTop)
				StartApp.main.TopMost = true;
			else
				StartApp.main.TopMost = false;

			//see if we need to switch mode
			if(Configuration.State.Gnutella2Stats.ultrapeer)
                if (Configuration.settings.firewall || !Configuration.settings.allowUltrapeer)
                    FileScopeCore.Gnutella2.ConnectionManager.SwitchMode();

			//close options dialog
			this.Close();
		}

		private void buttonBrowse_Click(object sender, System.EventArgs e)
		{
			//browse for download folder
			BrowseForFolder pickFolder = new BrowseForFolder();
			string strDownload = pickFolder.Show("Pick your download directory");
			if(strDownload.Length > 0)
				textDownload.Text = strDownload;
		}

		private void button9_Click(object sender, System.EventArgs e)
		{
			//browse for shared folder
			BrowseForFolder pickFolder = new BrowseForFolder();
			string strDir = pickFolder.Show("Pick a folder to share");
			if(strDir.Length > 0)
				listShares.Items.Add(strDir);
		}

		private void button10_Click(object sender, System.EventArgs e)
		{
			//remove selected shares
			for(int count = listShares.SelectedIndices.Count - 1; count >= 0; count--)
				listShares.Items.RemoveAt(listShares.SelectedIndices[count]);
		}

		private void button4_Click(object sender, System.EventArgs e)
		{
			//add blocked chat host
			string block = InputBox.Show("What is the IP Address of the host you would like to block?");
			if(block == "")
				return;
			listBoxChats.Items.Add(block);
		}

		private void button3_Click(object sender, System.EventArgs e)
		{
			//remove blocked chat hosts
			for(int count = listBoxChats.SelectedIndices.Count - 1; count >= 0; count--)
				listBoxChats.Items.RemoveAt(listBoxChats.SelectedIndices[count]);
		}



		private void button8_Click(object sender, System.EventArgs e)
		{
			//add webcache server
			string server = InputBox.Show("What is the URL of the server?");
			if(server == "")
				return;
			listBoxWebCache2.Items.Add(server);
		}

		private void button7_Click(object sender, System.EventArgs e)
		{
			//remove webcache servers
			for(int count = listBoxWebCache2.SelectedIndices.Count - 1; count >= 0; count--)
				listBoxWebCache2.Items.RemoveAt(listBoxWebCache2.SelectedIndices[count]);
		}
	}
}
