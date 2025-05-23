// HomePage.cs
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
using System.Data;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using FileScopeCore;

namespace FileScope
{
	/// <summary>
	/// The first tab that appears in MainDlg when FileScope starts.
	/// This is the "Home Page".
	/// </summary>
	public class HomePage : System.Windows.Forms.UserControl
	{
		private System.Windows.Forms.PictureBox pictureBox1;
		private System.Windows.Forms.LinkLabel linkFscp;
		private System.Windows.Forms.Label lblNetwork;
		private System.Windows.Forms.ImageList imageList1;
		private System.Windows.Forms.Label lblSearches;
		private System.Windows.Forms.Label lblTransfers;
		private System.Windows.Forms.Label lblLibrary;
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.LinkLabel lbl2Searches;
		private System.Windows.Forms.LinkLabel lbl2Transfers;
		private System.Windows.Forms.LinkLabel lbl2Library;
        int smallGap;
        private System.Windows.Forms.LinkLabel lbl2Network2;
		private System.Windows.Forms.Label lblConfiguration;
		private System.Windows.Forms.Label lblNB;
		private System.Windows.Forms.Label lblTB;
		private System.Windows.Forms.Label lblNBin;
		private System.Windows.Forms.Label lblNBout;
		private System.Windows.Forms.Label lblTBin;
		private System.Windows.Forms.Label lblTBout;
		private System.Windows.Forms.Timer timer1;
		private System.Windows.Forms.Label lblIP;
		private System.Windows.Forms.Label lblIP1;
		int largeGap;
        private LinkLabel lbl2Network;
    
		public delegate void resetNetworksText();
		public delegate void resetTransfersText();
		public delegate void resetLibraryText();
		int cirSize = 152;

		public HomePage()
		{
			InitializeComponent();
			Control c = (Control)this;
			Themes.SetupTheme(c);
			ResetBrushes();
			this.SetStyle(ControlStyles.UserPaint, true);
			this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
			this.imageList1.ImageStream = StartApp.main.imageList1.ImageStream;
			lblNetwork.ImageIndex = 1;
			lblSearches.ImageIndex = 0;
			lblTransfers.ImageIndex = 2;
			lblLibrary.ImageIndex = 3;
			smallGap = lbl2Network.Top - (lblNetwork.Top + lblNetwork.Height);
			largeGap = lblTransfers.Top - (lbl2Searches.Top + lbl2Searches.Height);
            //CEH
  
		}

		SolidBrush br;
		LinearGradientBrush lgb;
		Point[] ps;
		Pen pFS;
		SolidBrush pfsb;
		LinearGradientBrush lgb2;
		GraphicsPath path;
		HatchBrush hbtr;
		Pen phbtr;

		/// <summary>
		/// This can be called when themes change.
		/// </summary>
		public void ResetBrushes()
		{
			if(br != null) br.Dispose();
			br = new SolidBrush(StartApp.settings.clFormsBack);
			if(lgb != null) lgb.Dispose();
			lgb = new LinearGradientBrush(new Rectangle(0,0,127,127), StartApp.settings.clHomeTL, StartApp.settings.clFormsBack, 60);
			ps = new Point[3];
			ps[0] = new Point(0,0);
			ps[1] = new Point(346,0);
			ps[2] = new Point(0,200);
			if(pFS != null) pFS.Dispose();
			pFS = new Pen(StartApp.settings.clHomeBR, 16);
			if(pfsb != null) pfsb.Dispose();
			pfsb = new SolidBrush(pFS.Color);
			if(hbtr != null) hbtr.Dispose();
			hbtr = new HatchBrush(HatchStyle.WideDownwardDiagonal, Color.Transparent, Color.FromArgb(40, StartApp.settings.clHomeBR));
			if(phbtr != null) phbtr.Dispose();
			phbtr = new Pen(hbtr, 4);
			FscpLogoStuff();
		}

		void FscpLogoStuff()
		{
			int dX = this.ClientSize.Width - 110;
			int dY = this.ClientSize.Height - 110;
			if(lgb2 != null) lgb2.Dispose();
			lgb2 = new LinearGradientBrush(new Point(0,dY), new Point(0,dY+118), Color.FromArgb(160, Color.White), Color.Transparent);
			if(path != null) path.Dispose();
			path = new GraphicsPath(new Point[]
								{
									new Point(dX-64, dY-24),//start
									new Point(dX-59, dY-34),//focal point 1
									new Point(dX-52, dY-54),//focal point 2
									new Point(dX-18, dY-66),//top
									new Point(dX-34, dY-47),//focal point 1
									new Point(dX-43, dY-27),//focal point 2
									new Point(dX-44, dY-8),//end
			}, new byte[]
								{
									(byte)PathPointType.Start,
									(byte)PathPointType.Bezier,
									(byte)PathPointType.Bezier,
									(byte)PathPointType.Bezier,
									(byte)PathPointType.Bezier,
									(byte)PathPointType.Bezier,
									(byte)PathPointType.Bezier,
			});
		}

        public void ResetNetworksText()
        {
            int totalConns = Configuration.State.Gnutella2Stats.lastConnectionCount;
            if (totalConns == 0)
                lbl2Network.Text = "You're currently not connected to any network (Connections Area)";
            else
            {
                int totalNetworks = 0;

                if (Configuration.State.Gnutella2Stats.lastConnectionCount > 0)
                    totalNetworks++;

                lbl2Network.Text = "You're currently connected to " + totalNetworks.ToString() + " network(s) (Connections Area)";
            }
            lbl2Network.LinkArea = new LinkArea(lbl2Network.Text.Length - 17, 16);


            //gnutella 2
            string gnutella2Stat = "Connected";
            if (Configuration.State.Gnutella2Stats.lastConnectionCount == 0)
                gnutella2Stat = FileScopeCore.Gnutella2.ConnectionManager.status;
            lbl2Network2.Text = "      Gnutella2: " + gnutella2Stat;
            if (gnutella2Stat == "Connected")
                lbl2Network2.Text += "(" + Configuration.State.Gnutella2Stats.lastConnectionCount.ToString() + ")";
            if (FileScopeCore.Gnutella2.ConnectionManager.status == "Disconnected")
            {
                lbl2Network2.Text += " --- Connect";
                lbl2Network2.LinkArea = new LinkArea(lbl2Network2.Text.Length - 7, 7);
            }
            else
            {
                lbl2Network2.Text += " --- Disconnect";
                lbl2Network2.LinkArea = new LinkArea(lbl2Network2.Text.Length - 10, 10);
            }

        }

		public void ResetSearchesText()
		{
			if(StartApp.main.search == null)
				return;
			int origHeight = lbl2Searches.Height;
			if(StartApp.main.search.tabControl1.TabCount == 0)
				lbl2Searches.Text = "There are no open searches";
			else if(StartApp.main.search.tabControl1.TabCount == 1)
				lbl2Searches.Text = "There is 1 open search";
			else
				lbl2Searches.Text = "There are " + StartApp.main.search.tabControl1.TabCount.ToString() + " open searches";
			lbl2Searches.Text += " (Search Area)";
			lbl2Searches.LinkArea = new LinkArea(lbl2Searches.Text.Length - 12, 11);

			foreach(TabPage tp in StartApp.main.search.tabControl1.TabPages)
				lbl2Searches.Text += "\n      " + tp.Text;

			lbl2Searches.Height = lbl2Searches.Font.Height * NumLines(lbl2Searches.Text) + 5;
			if(lbl2Searches.Height != origHeight)
				this.HomePage_Resize(null, null);
		}

		public void ResetTransfersText()
		{
			int origHeight = lbl2Transfers.Height;
			lbl2Transfers.Text = "You have " + Configuration.State.downloadsNow.ToString() + " download(s) and " + Configuration.State.uploadsNow.ToString() + " upload(s)";
			lbl2Transfers.Text += "\nClick here to see your transfers";
			lbl2Transfers.LinkArea = new LinkArea(lbl2Transfers.Text.Length - 32, 32);
			lbl2Transfers.Height = lbl2Transfers.Font.Height * NumLines(lbl2Transfers.Text) + 5;
			if(lbl2Transfers.Height != origHeight)
				this.HomePage_Resize(null, null);
		}

		public void ResetLibraryText()
		{
			lbl2Library.Text = "You're sharing " + Configuration.State.filesShared.ToString() + " file(s) (" + Utils.Assemble((uint)Configuration.State.kbShared, " KB") + ") in your library";
			lbl2Library.LinkArea = new LinkArea(lbl2Library.Text.Length - 12, 12);
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(HomePage));
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.linkFscp = new System.Windows.Forms.LinkLabel();
            this.lblNetwork = new System.Windows.Forms.Label();
            this.imageList1 = new System.Windows.Forms.ImageList(this.components);
            this.lblSearches = new System.Windows.Forms.Label();
            this.lblTransfers = new System.Windows.Forms.Label();
            this.lblLibrary = new System.Windows.Forms.Label();
            this.lbl2Searches = new System.Windows.Forms.LinkLabel();
            this.lbl2Transfers = new System.Windows.Forms.LinkLabel();
            this.lbl2Library = new System.Windows.Forms.LinkLabel();
            this.lbl2Network2 = new System.Windows.Forms.LinkLabel();
            this.lblConfiguration = new System.Windows.Forms.Label();
            this.lblNB = new System.Windows.Forms.Label();
            this.lblTB = new System.Windows.Forms.Label();
            this.lblNBin = new System.Windows.Forms.Label();
            this.lblNBout = new System.Windows.Forms.Label();
            this.lblTBin = new System.Windows.Forms.Label();
            this.lblTBout = new System.Windows.Forms.Label();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.lblIP = new System.Windows.Forms.Label();
            this.lblIP1 = new System.Windows.Forms.Label();
            this.lbl2Network = new System.Windows.Forms.LinkLabel();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.Location = new System.Drawing.Point(0, 0);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(472, 87);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.Visible = false;
            // 
            // linkFscp
            // 
            this.linkFscp.AutoSize = true;
            this.linkFscp.DisabledLinkColor = System.Drawing.Color.Blue;
            this.linkFscp.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.linkFscp.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.linkFscp.LinkColor = System.Drawing.Color.Blue;
            this.linkFscp.Location = new System.Drawing.Point(8, 424);
            this.linkFscp.Name = "linkFscp";
            this.linkFscp.Size = new System.Drawing.Size(95, 16);
            this.linkFscp.TabIndex = 1;
            this.linkFscp.TabStop = true;
            this.linkFscp.Text = "FileScope.com";
            this.linkFscp.VisitedLinkColor = System.Drawing.Color.Blue;
            this.linkFscp.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkFscp_LinkClicked);
            // 
            // lblNetwork
            // 
            this.lblNetwork.AutoSize = true;
            this.lblNetwork.BackColor = System.Drawing.Color.Transparent;
            this.lblNetwork.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblNetwork.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblNetwork.ImageList = this.imageList1;
            this.lblNetwork.Location = new System.Drawing.Point(16, 104);
            this.lblNetwork.Name = "lblNetwork";
            this.lblNetwork.Size = new System.Drawing.Size(121, 24);
            this.lblNetwork.TabIndex = 2;
            this.lblNetwork.Text = "   Networks";
            // 
            // imageList1
            // 
            this.imageList1.ColorDepth = System.Windows.Forms.ColorDepth.Depth16Bit;
            this.imageList1.ImageSize = new System.Drawing.Size(16, 16);
            this.imageList1.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // lblSearches
            // 
            this.lblSearches.AutoSize = true;
            this.lblSearches.BackColor = System.Drawing.Color.Transparent;
            this.lblSearches.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblSearches.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblSearches.ImageList = this.imageList1;
            this.lblSearches.Location = new System.Drawing.Point(16, 232);
            this.lblSearches.Name = "lblSearches";
            this.lblSearches.Size = new System.Drawing.Size(123, 24);
            this.lblSearches.TabIndex = 3;
            this.lblSearches.Text = "   Searches";
            // 
            // lblTransfers
            // 
            this.lblTransfers.AutoSize = true;
            this.lblTransfers.BackColor = System.Drawing.Color.Transparent;
            this.lblTransfers.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTransfers.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblTransfers.ImageList = this.imageList1;
            this.lblTransfers.Location = new System.Drawing.Point(16, 296);
            this.lblTransfers.Name = "lblTransfers";
            this.lblTransfers.Size = new System.Drawing.Size(125, 24);
            this.lblTransfers.TabIndex = 4;
            this.lblTransfers.Text = "   Transfers";
            // 
            // lblLibrary
            // 
            this.lblLibrary.AutoSize = true;
            this.lblLibrary.BackColor = System.Drawing.Color.Transparent;
            this.lblLibrary.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblLibrary.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.lblLibrary.ImageList = this.imageList1;
            this.lblLibrary.Location = new System.Drawing.Point(16, 360);
            this.lblLibrary.Name = "lblLibrary";
            this.lblLibrary.Size = new System.Drawing.Size(98, 24);
            this.lblLibrary.TabIndex = 5;
            this.lblLibrary.Text = "   Library";
            // 
            // lbl2Searches
            // 
            this.lbl2Searches.BackColor = System.Drawing.Color.Transparent;
            this.lbl2Searches.DisabledLinkColor = System.Drawing.Color.Blue;
            this.lbl2Searches.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbl2Searches.LinkArea = new System.Windows.Forms.LinkArea(0, 0);
            this.lbl2Searches.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.lbl2Searches.LinkColor = System.Drawing.Color.Blue;
            this.lbl2Searches.Location = new System.Drawing.Point(56, 264);
            this.lbl2Searches.Name = "lbl2Searches";
            this.lbl2Searches.Size = new System.Drawing.Size(192, 16);
            this.lbl2Searches.TabIndex = 7;
            this.lbl2Searches.Text = "--";
            this.lbl2Searches.VisitedLinkColor = System.Drawing.Color.Blue;
            this.lbl2Searches.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lbl2Searches_LinkClicked);
            // 
            // lbl2Transfers
            // 
            this.lbl2Transfers.BackColor = System.Drawing.Color.Transparent;
            this.lbl2Transfers.DisabledLinkColor = System.Drawing.Color.Blue;
            this.lbl2Transfers.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbl2Transfers.LinkArea = new System.Windows.Forms.LinkArea(0, 0);
            this.lbl2Transfers.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.lbl2Transfers.LinkColor = System.Drawing.Color.Blue;
            this.lbl2Transfers.Location = new System.Drawing.Point(56, 328);
            this.lbl2Transfers.Name = "lbl2Transfers";
            this.lbl2Transfers.Size = new System.Drawing.Size(192, 16);
            this.lbl2Transfers.TabIndex = 8;
            this.lbl2Transfers.Text = "--";
            this.lbl2Transfers.VisitedLinkColor = System.Drawing.Color.Blue;
            this.lbl2Transfers.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lbl2Transfers_LinkClicked);
            // 
            // lbl2Library
            // 
            this.lbl2Library.BackColor = System.Drawing.Color.Transparent;
            this.lbl2Library.DisabledLinkColor = System.Drawing.Color.Blue;
            this.lbl2Library.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbl2Library.LinkArea = new System.Windows.Forms.LinkArea(0, 0);
            this.lbl2Library.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.lbl2Library.LinkColor = System.Drawing.Color.Blue;
            this.lbl2Library.Location = new System.Drawing.Point(56, 392);
            this.lbl2Library.Name = "lbl2Library";
            this.lbl2Library.Size = new System.Drawing.Size(192, 16);
            this.lbl2Library.TabIndex = 9;
            this.lbl2Library.Text = "--";
            this.lbl2Library.VisitedLinkColor = System.Drawing.Color.Blue;
            this.lbl2Library.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lbl2Library_LinkClicked);
            // 
            // lbl2Network2
            // 
            this.lbl2Network2.BackColor = System.Drawing.Color.Transparent;
            this.lbl2Network2.DisabledLinkColor = System.Drawing.Color.Blue;
            this.lbl2Network2.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbl2Network2.LinkArea = new System.Windows.Forms.LinkArea(0, 0);
            this.lbl2Network2.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.lbl2Network2.LinkColor = System.Drawing.Color.Blue;
            this.lbl2Network2.Location = new System.Drawing.Point(56, 168);
            this.lbl2Network2.Name = "lbl2Network2";
            this.lbl2Network2.Size = new System.Drawing.Size(192, 16);
            this.lbl2Network2.TabIndex = 10;
            this.lbl2Network2.Text = "--";
            this.lbl2Network2.VisitedLinkColor = System.Drawing.Color.Blue;
            this.lbl2Network2.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.lbl2Network_LinkClicked);
            // 
            // lblConfiguration
            // 
            this.lblConfiguration.Location = new System.Drawing.Point(0, 0);
            this.lblConfiguration.Name = "lblConfiguration";
            this.lblConfiguration.Size = new System.Drawing.Size(100, 23);
            this.lblConfiguration.TabIndex = 24;
            // 
            // lblNB
            // 
            this.lblNB.AutoSize = true;
            this.lblNB.BackColor = System.Drawing.Color.Transparent;
            this.lblNB.Font = new System.Drawing.Font("Arial", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblNB.Location = new System.Drawing.Point(584, 40);
            this.lblNB.Name = "lblNB";
            this.lblNB.Size = new System.Drawing.Size(170, 22);
            this.lblNB.TabIndex = 15;
            this.lblNB.Text = "Network Bandwidth";
            // 
            // lblTB
            // 
            this.lblTB.AutoSize = true;
            this.lblTB.BackColor = System.Drawing.Color.Transparent;
            this.lblTB.Font = new System.Drawing.Font("Arial", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTB.Location = new System.Drawing.Point(584, 120);
            this.lblTB.Name = "lblTB";
            this.lblTB.Size = new System.Drawing.Size(183, 22);
            this.lblTB.TabIndex = 16;
            this.lblTB.Text = "Transfers Bandwidth";
            // 
            // lblNBin
            // 
            this.lblNBin.AutoSize = true;
            this.lblNBin.BackColor = System.Drawing.Color.Transparent;
            this.lblNBin.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblNBin.Location = new System.Drawing.Point(640, 64);
            this.lblNBin.Name = "lblNBin";
            this.lblNBin.Size = new System.Drawing.Size(55, 18);
            this.lblNBin.TabIndex = 17;
            this.lblNBin.Text = "0 KB/s";
            // 
            // lblNBout
            // 
            this.lblNBout.AutoSize = true;
            this.lblNBout.BackColor = System.Drawing.Color.Transparent;
            this.lblNBout.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblNBout.Location = new System.Drawing.Point(640, 88);
            this.lblNBout.Name = "lblNBout";
            this.lblNBout.Size = new System.Drawing.Size(55, 18);
            this.lblNBout.TabIndex = 18;
            this.lblNBout.Text = "0 KB/s";
            // 
            // lblTBin
            // 
            this.lblTBin.AutoSize = true;
            this.lblTBin.BackColor = System.Drawing.Color.Transparent;
            this.lblTBin.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTBin.Location = new System.Drawing.Point(640, 144);
            this.lblTBin.Name = "lblTBin";
            this.lblTBin.Size = new System.Drawing.Size(55, 18);
            this.lblTBin.TabIndex = 19;
            this.lblTBin.Text = "0 KB/s";
            // 
            // lblTBout
            // 
            this.lblTBout.AutoSize = true;
            this.lblTBout.BackColor = System.Drawing.Color.Transparent;
            this.lblTBout.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTBout.Location = new System.Drawing.Point(640, 168);
            this.lblTBout.Name = "lblTBout";
            this.lblTBout.Size = new System.Drawing.Size(55, 18);
            this.lblTBout.TabIndex = 20;
            this.lblTBout.Text = "0 KB/s";
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 1000;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // lblIP
            // 
            this.lblIP.AutoSize = true;
            this.lblIP.BackColor = System.Drawing.Color.Transparent;
            this.lblIP.Font = new System.Drawing.Font("Arial", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblIP.Location = new System.Drawing.Point(592, 200);
            this.lblIP.Name = "lblIP";
            this.lblIP.Size = new System.Drawing.Size(73, 22);
            this.lblIP.TabIndex = 21;
            this.lblIP.Text = "Your IP";
            // 
            // lblIP1
            // 
            this.lblIP1.AutoSize = true;
            this.lblIP1.BackColor = System.Drawing.Color.Transparent;
            this.lblIP1.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblIP1.Location = new System.Drawing.Point(603, 222);
            this.lblIP1.Name = "lblIP1";
            this.lblIP1.Size = new System.Drawing.Size(74, 18);
            this.lblIP1.TabIndex = 22;
            this.lblIP1.Text = "127.0.0.1";
            // 
            // lbl2Network
            // 
            this.lbl2Network.BackColor = System.Drawing.Color.Transparent;
            this.lbl2Network.DisabledLinkColor = System.Drawing.Color.Blue;
            this.lbl2Network.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lbl2Network.LinkArea = new System.Windows.Forms.LinkArea(0, 0);
            this.lbl2Network.LinkBehavior = System.Windows.Forms.LinkBehavior.HoverUnderline;
            this.lbl2Network.LinkColor = System.Drawing.Color.Blue;
            this.lbl2Network.Location = new System.Drawing.Point(56, 146);
            this.lbl2Network.Name = "lbl2Network";
            this.lbl2Network.Size = new System.Drawing.Size(192, 16);
            this.lbl2Network.TabIndex = 23;
            this.lbl2Network.Text = "--";
            this.lbl2Network.VisitedLinkColor = System.Drawing.Color.Blue;
            // 
            // HomePage
            // 
            this.Controls.Add(this.lbl2Network);
            this.Controls.Add(this.lblIP1);
            this.Controls.Add(this.lblIP);
            this.Controls.Add(this.lblTBout);
            this.Controls.Add(this.lblTBin);
            this.Controls.Add(this.lblNBout);
            this.Controls.Add(this.lblNBin);
            this.Controls.Add(this.lblTB);
            this.Controls.Add(this.lblNB);
            this.Controls.Add(this.lblConfiguration);
            this.Controls.Add(this.lbl2Network2);
            this.Controls.Add(this.lbl2Library);
            this.Controls.Add(this.lbl2Transfers);
            this.Controls.Add(this.lbl2Searches);
            this.Controls.Add(this.lblLibrary);
            this.Controls.Add(this.lblTransfers);
            this.Controls.Add(this.lblSearches);
            this.Controls.Add(this.lblNetwork);
            this.Controls.Add(this.linkFscp);
            this.Controls.Add(this.pictureBox1);
            this.Name = "HomePage";
            this.Size = new System.Drawing.Size(784, 448);
            this.Resize += new System.EventHandler(this.HomePage_Resize);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

		}
		#endregion

		protected override void OnPaintBackground(System.Windows.Forms.PaintEventArgs e)
		{
			e.Graphics.FillRectangle(br, e.ClipRectangle);
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			//linear gradient from the top-left corner
			e.Graphics.FillPolygon(lgb, ps);

			//top-left corner logo
			e.Graphics.DrawImage(pictureBox1.Image, 0, 0, pictureBox1.Width, pictureBox1.Height);

			//filescope logo at the bottom-right
			int dX = this.ClientSize.Width - 110;
			int dY = this.ClientSize.Height - 110;
			e.Graphics.DrawLine(pFS, dX-100, dY, dX+100, dY);
			e.Graphics.DrawLine(pFS, dX, dY-100, dX, dY+100);
			e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
			e.Graphics.DrawEllipse(pFS, dX-(cirSize/2), dY-(cirSize/2), cirSize, cirSize);
			e.Graphics.FillPie(pfsb, dX-cirSize/2, dY-cirSize/2, cirSize, cirSize, 0, 90);
			e.Graphics.FillPie(pfsb, dX-cirSize/2, dY-cirSize/2, cirSize, cirSize, 180, 90);
			e.Graphics.FillPath(lgb2, path);

			//top-right corner stats
			e.Graphics.DrawLine(phbtr, this.ClientSize.Width - 186, 0, this.ClientSize.Width - 186, 250);
			e.Graphics.DrawLine(phbtr, this.ClientSize.Width - 186, 250, this.ClientSize.Width, 250);

			//some other stuff
			this.lblLibrary.ForeColor = StartApp.settings.clLabelFore2;
			this.lblNetwork.ForeColor = StartApp.settings.clLabelFore2;
			this.lblSearches.ForeColor = StartApp.settings.clLabelFore2;
			this.lblTransfers.ForeColor = StartApp.settings.clLabelFore2;
			this.lblIP.ForeColor = StartApp.settings.clLabelFore2;
			this.lblNB.ForeColor = StartApp.settings.clLabelFore2;
			this.lblTB.ForeColor = StartApp.settings.clLabelFore2;
			this.lblConfiguration.ForeColor = StartApp.settings.clLabelFore2;
		}

		private void HomePage_Resize(object sender, System.EventArgs e)
		{
			FscpLogoStuff();

			this.linkFscp.Top = this.ClientSize.Height - linkFscp.Height - linkFscp.Left;
			
			lblSearches.Top = lbl2Network2.Top + lbl2Network2.Height + largeGap;
			lbl2Searches.Top = lblSearches.Top + lblSearches.Height + smallGap;
			lblTransfers.Top = lbl2Searches.Top + lbl2Searches.Height + largeGap;
			lbl2Transfers.Top = lblTransfers.Top + lblTransfers.Height + smallGap;
			lblLibrary.Top = lbl2Transfers.Top + lbl2Transfers.Height + largeGap;
			lbl2Library.Top = lblLibrary.Top + lblLibrary.Height + smallGap;

			lbl2Network.Width = this.Width - lbl2Network.Left;
			lbl2Network2.Width = this.Width - lbl2Network2.Left;
			lbl2Searches.Width = this.Width - lbl2Searches.Left;
			lbl2Transfers.Width = this.Width - lbl2Transfers.Left;
			lbl2Library.Width = this.Width - lbl2Library.Left;

			lblConfiguration.Left = this.Width - lblConfiguration.Width - 60;
			lblNB.Left = this.Width - 182;
			lblNBin.Left = this.Width - 166;
			lblNBout.Left = lblNBin.Left;
			lblTB.Left = lblNB.Left;
			lblTBin.Left = lblNBin.Left;
			lblTBout.Left = lblNBin.Left;
			lblIP.Left = lblNB.Left;
			lblIP1.Left = lblNBin.Left;
		}

		/// <summary>
		/// Simply returns the line count in labelText.
		/// </summary>
		int NumLines(string labelText)
		{
			int count = 1;
			foreach(char chr in labelText.ToCharArray())
				if(chr == '\n')
					count++;
			return count;
		}

		private void linkFscp_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			Utils.SpawnLink("http://www.filescope.com");
		}

		private void lbl2Network_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			Label lbl2net = (Label)sender;
			bool connect = lbl2net.Text.EndsWith("Connect");
			if(lbl2net.Text.IndexOf("Gnutella2") != -1)
			{
				if(connect)
                    FileScopeCore.Gnutella2.StartStop.Start();
				else
                    FileScopeCore.Gnutella2.StartStop.Stop();
			}
			
		}

		private void lbl2Network_LinkClicked_1(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			StartApp.main.tabControl1.SelectedIndex = 1;
		}

		private void lbl2Searches_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			StartApp.main.tabControl1.SelectedIndex = 2;
		}

		private void lbl2Transfers_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			StartApp.main.tabControl1.SelectedIndex = 3;
		}

		private void lbl2Library_LinkClicked(object sender, System.Windows.Forms.LinkLabelLinkClickedEventArgs e)
		{
			StartApp.main.tabControl1.SelectedIndex = 4;
		}

		private void timer1_Tick(object sender, System.EventArgs e)
		{
			try
			{
				double kb;
				kb = (double)Configuration.State.inNetworkBandwidth / (double)1024;
				Configuration.State.inNetworkBandwidth = 0;
				kb = Math.Round(kb, 2);
				lblNBin.Text = "In: " + kb.ToString() + " KB/s";
				kb = (double)Configuration.State.outNetworkBandwidth / (double)1024;
				Configuration.State.outNetworkBandwidth = 0;
				kb = Math.Round(kb, 2);
				lblNBout.Text = "Out: " + kb.ToString() + " KB/s";
				kb = (double)Configuration.State.inTransferBandwidth / (double)1024;
				Configuration.State.inTransferBandwidth = 0;
				kb = Math.Round(kb, 2);
				lblTBin.Text = "In: " + kb.ToString() + " KB/s";
				kb = (double)Configuration.State.outTransferBandwidth / (double)1024;
				Configuration.State.outTransferBandwidth = 0;
				kb = Math.Round(kb, 2);
				lblTBout.Text = "Out: " + kb.ToString() + " KB/s";
                lblIP1.Text = Configuration.settings.ipAddress.ToString() + ":" + Configuration.settings.port.ToString();
			}
			catch(Exception e2)
			{
				System.Diagnostics.Debug.WriteLine("HomePage timer1_Tick " + e2.Message);
			}
		}
	}
}
