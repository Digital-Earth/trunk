// Themes.cs
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
using System.Windows.Forms;
using System.Drawing;
using FileScopeCore;

namespace FileScope
{
    /// <summary>
    /// Responsible for dealing with different color schemes in FileScope.
    /// </summary>
    public class Themes
    {
        /// <summary>
        /// This will take any object like a form or control and theme it accordingly.
        /// </summary>
        public static void SetupTheme(Control container)
        {
            //context menus
            ContextMenu[] cmArray = null;
            if (container.GetType() == typeof(Connection))
                cmArray = ((Connection)container).cmArray;
            else if (container.GetType() == typeof(Library))
                cmArray = ((Library)container).cmArray;
            else if (container.GetType() == typeof(SearchTabPage))
                cmArray = ((SearchTabPage)container).cmArray;
            else if (container.GetType() == typeof(Transfers))
                cmArray = ((Transfers)container).cmArray;
            if (cmArray != null)
                foreach (ContextMenu cm in cmArray)
                    foreach (ElMenuItem emi in cm.MenuItems)
                    {
                        if (StartApp.settings.theme != "Default Colors")
                            emi.SwitchOwner();
                        else if (StartApp.settings.theme == "Default Colors")
                            emi.SwitchDefault();
                    }
            //main menus
            if (container.GetType() == typeof(MainDlg))
            {
                MainDlg md = (MainDlg)container;
                foreach (ElMenuItem emi in md.Menu.MenuItems)
                {
                    if (StartApp.settings.theme != "Default Colors")
                        emi.SwitchOwner();
                    else if (StartApp.settings.theme == "Default Colors")
                        emi.SwitchDefault();
                    foreach (ElMenuItem emi2 in emi.MenuItems)
                    {
                        if (StartApp.settings.theme != "Default Colors")
                            emi2.SwitchOwner();
                        else if (StartApp.settings.theme == "Default Colors")
                            emi2.SwitchDefault();
                    }
                }
            }
            container.BackColor = StartApp.settings.clFormsBack;
            foreach (Control c in container.Controls)
            {
                if (c.GetType() == typeof(Label) || c.GetType() == typeof(LinkLabel))
                {
                    c.ForeColor = StartApp.settings.clLabelFore;
                }
                else if (c.GetType() == typeof(Button))
                {
                    c.BackColor = StartApp.settings.clButtonBack;
                    c.ForeColor = StartApp.settings.clButtonFore;
                }
                else if (c.GetType() == typeof(TextBox) || c.GetType() == typeof(ComboBox) || c.GetType() == typeof(NumericUpDown))
                {
                    c.BackColor = StartApp.settings.clTextBoxBack;
                    c.ForeColor = StartApp.settings.clTextBoxFore;
                }
                else if (c.GetType() == typeof(CheckBox) || c.GetType() == typeof(RadioButton))
                {
                    c.ForeColor = StartApp.settings.clCheckBoxFore;
                }
                else if (c.GetType() == typeof(Splitter))
                {
                    c.BackColor = StartApp.settings.clLabelFore2;
                }
                else if (c.GetType() == typeof(GroupBox))
                {
                    Control c4 = (Control)c;
                    SetupTheme(c4);
                    c.BackColor = StartApp.settings.clGroupBoxBack;
                    c.ForeColor = StartApp.settings.clGroupBoxFore;
                }
                else if (c.GetType() == typeof(ListBox) || c.GetType() == typeof(TreeView) || c.GetType() == typeof(ListView))
                {
                    //exception
                    if (c.GetType() == typeof(ListBox))
                        if (((ListBox)c).Name == "listQueries" || ((ListBox)c).Name == "listQueries2")
                        {
                            c.BackColor = StartApp.settings.clGroupBoxBack;
                            c.ForeColor = StartApp.settings.clLabelFore;
                            continue;
                        }
                    c.BackColor = StartApp.settings.clListBoxBack;
                    c.ForeColor = StartApp.settings.clListBoxFore;
                    if (c.GetType() == typeof(ListView))
                    {
                        ListView lv = (ListView)c;
                        lv.GridLines = StartApp.settings.clGridLines;
                    }
                }
                else if (c.GetType() == typeof(RichTextBox))
                {
                    c.BackColor = StartApp.settings.clRichTextBoxBack;
                }
                else if (c.GetType() == typeof(Panel))
                {
                    Control c3 = (Control)c;
                    SetupTheme(c3);
                }
                else if (c.GetType() == typeof(ElTabControl))
                {
                    if (StartApp.settings.theme != "Default Colors")
                        ((ElTabControl)c).SwitchOwner(true);
                    else if (StartApp.settings.theme == "Default Colors")
                        ((ElTabControl)c).SwitchDefault(true);
                    TabControl c2 = (TabControl)c;
                    foreach (TabPage tp in c2.TabPages)
                    {
                        Control elTp = (Control)tp;
                        SetupTheme(elTp);
                    }
                }
            }
        }

        /// <summary>
        /// Setup a color scheme for FileScope.
        /// </summary>
        public static void SetColors(string kind)
        {
            StartApp.settings.theme = kind;
            switch (kind)
            {
                case "Default Colors":
                    StartApp.settings.clButtonBack = Color.FromKnownColor(KnownColor.Control);
                    StartApp.settings.clButtonFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 145, 55);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clFormsBack = Color.FromKnownColor(KnownColor.Control);
                    StartApp.settings.clMenuBox = Color.Green;
                    StartApp.settings.clMenuHighlight1 = Color.Green;
                    StartApp.settings.clMenuHighlight2 = Color.Green;
                    StartApp.settings.clMenuBorder = Color.Green;
                    StartApp.settings.clGroupBoxBack = Color.FromKnownColor(KnownColor.Control);
                    StartApp.settings.clGroupBoxFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 0, 255);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 255, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(255, 155, 55);
                    StartApp.settings.clHighlight4 = Color.FromArgb(255, 0, 0);
                    StartApp.settings.clLabelFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clLabelFore2 = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clListBoxBack = Color.FromKnownColor(KnownColor.Window);
                    StartApp.settings.clListBoxFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(84, 103, 117);
                    StartApp.settings.clTextBoxBack = Color.FromKnownColor(KnownColor.Window);
                    StartApp.settings.clTextBoxFore = Color.FromKnownColor(KnownColor.ControlText);
                    StartApp.settings.clHomeBR = Color.FromArgb(80, Color.Gray);
                    StartApp.settings.clHomeTL = Color.Orange;
                    StartApp.settings.clGridLines = true;
                    break;
                case "Blueish":
                    StartApp.settings.clButtonBack = Color.LightBlue;
                    StartApp.settings.clButtonFore = Color.Black;
                    StartApp.settings.clChatHeader = Color.Black;
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.MidnightBlue;
                    StartApp.settings.clFormsBack = Color.SteelBlue;
                    StartApp.settings.clMenuBox = Color.MidnightBlue;
                    StartApp.settings.clMenuHighlight1 = Color.FromArgb(40, Color.Blue);
                    StartApp.settings.clMenuHighlight2 = Color.CornflowerBlue;
                    StartApp.settings.clMenuBorder = Color.White;
                    StartApp.settings.clGroupBoxBack = Color.DeepSkyBlue;
                    StartApp.settings.clGroupBoxFore = Color.MidnightBlue;
                    StartApp.settings.clHighlight1 = Color.FromArgb(255, 255, 255);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 0, 200);
                    StartApp.settings.clHighlight3 = Color.FromArgb(120, 0, 0);
                    StartApp.settings.clHighlight4 = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clLabelFore = Color.MidnightBlue;
                    StartApp.settings.clLabelFore2 = Color.WhiteSmoke;
                    StartApp.settings.clListBoxBack = Color.DodgerBlue;
                    StartApp.settings.clListBoxFore = Color.Black;
                    StartApp.settings.clRichTextBoxBack = Color.LightSteelBlue;
                    StartApp.settings.clTextBoxBack = Color.MediumBlue;
                    StartApp.settings.clTextBoxFore = Color.LightSkyBlue;
                    StartApp.settings.clHomeBR = Color.MidnightBlue;
                    StartApp.settings.clHomeTL = Color.PowderBlue;
                    StartApp.settings.clGridLines = false;
                    break;
                case "Cherry":
                    StartApp.settings.clButtonBack = Color.FromArgb(252, 83, 83);
                    StartApp.settings.clButtonFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 255, 0);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(25, 255, 0);
                    StartApp.settings.clFormsBack = Color.FromArgb(170, 100, 100);
                    StartApp.settings.clMenuBox = Color.FromArgb(220, 104, 130);
                    StartApp.settings.clMenuHighlight1 = Color.Pink;
                    StartApp.settings.clMenuHighlight2 = Color.FromArgb(220, 104, 130);
                    StartApp.settings.clMenuBorder = Color.Purple;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(160, 52, 89);
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(255, 250, 0);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 0, 255);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 255, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(255, 255, 0);
                    StartApp.settings.clHighlight4 = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clLabelFore = Color.FromArgb(64, 128, 255);
                    StartApp.settings.clLabelFore2 = Color.DarkGray;
                    StartApp.settings.clListBoxBack = Color.FromArgb(220, 104, 130);
                    StartApp.settings.clListBoxFore = Color.FromArgb(255, 255, 255);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(220, 104, 130);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(188, 255, 188);
                    StartApp.settings.clTextBoxFore = Color.FromArgb(165, 24, 24);
                    StartApp.settings.clHomeBR = Color.FromArgb(200, 130, 130);
                    StartApp.settings.clHomeTL = Color.FromArgb(160, 52, 89);
                    StartApp.settings.clGridLines = true;
                    break;
                case "Dungeon":
                    StartApp.settings.clButtonBack = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clButtonFore = Color.FromArgb(233, 220, 107);
                    StartApp.settings.clChatHeader = Color.FromArgb(127, 255, 252);
                    StartApp.settings.clChatPeer = Color.FromArgb(40, 140, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 80, 80);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(120, 171, 255);
                    StartApp.settings.clFormsBack = Color.FromArgb(52, 62, 51);
                    StartApp.settings.clMenuBox = Color.FromArgb(65, 89, 65);
                    StartApp.settings.clMenuHighlight1 = Color.FromArgb(20, Color.White);
                    StartApp.settings.clMenuHighlight2 = Color.FromArgb(50, Color.White);
                    StartApp.settings.clMenuBorder = Color.White;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(0, 11, 21);
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(248, 128, 0);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 255, 72);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 200, 56);
                    StartApp.settings.clHighlight3 = Color.FromArgb(0, 138, 39);
                    StartApp.settings.clHighlight4 = Color.FromArgb(0, 0, 16);
                    StartApp.settings.clLabelFore = Color.Silver;
                    StartApp.settings.clLabelFore2 = Color.White;
                    StartApp.settings.clListBoxBack = Color.FromArgb(53, 89, 137);
                    StartApp.settings.clListBoxFore = Color.FromArgb(114, 253, 124);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(84, 103, 117);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(53, 89, 137);
                    StartApp.settings.clTextBoxFore = Color.FromArgb(114, 253, 124);
                    StartApp.settings.clHomeBR = Color.FromArgb(60, Color.Yellow);
                    StartApp.settings.clHomeTL = Color.WhiteSmoke;
                    StartApp.settings.clGridLines = false;
                    break;
                case "FileScope":
                    StartApp.settings.clButtonBack = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clButtonFore = Color.Orange;
                    StartApp.settings.clChatHeader = Color.Green;
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.Orange;
                    StartApp.settings.clFormsBack = Color.FromArgb(80, 80, 80);
                    StartApp.settings.clMenuBox = Color.FromArgb(66, 66, 66);
                    StartApp.settings.clMenuHighlight1 = Color.Orange;
                    StartApp.settings.clMenuHighlight2 = Color.FromArgb(120, Color.Orange);
                    StartApp.settings.clMenuBorder = Color.White;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(70, 70, 70);
                    StartApp.settings.clGroupBoxFore = Color.Orange;
                    StartApp.settings.clHighlight1 = Color.FromArgb(250, 180, 40);
                    StartApp.settings.clHighlight2 = Color.FromArgb(220, 120, 40);
                    StartApp.settings.clHighlight3 = Color.FromArgb(160, 80, 40);
                    StartApp.settings.clHighlight4 = Color.FromArgb(120, 40, 40);
                    StartApp.settings.clLabelFore = Color.Black;
                    StartApp.settings.clLabelFore2 = Color.LightGreen;
                    StartApp.settings.clListBoxBack = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clListBoxFore = Color.Gray;
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(30, 30, 30);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clTextBoxFore = Color.Orange;
                    StartApp.settings.clHomeBR = Color.FromArgb(120, Color.Orange);
                    StartApp.settings.clHomeTL = Color.Black;
                    StartApp.settings.clGridLines = false;
                    break;
                case "Forest":
                    StartApp.settings.clButtonBack = Color.FromArgb(37, 147, 60);
                    StartApp.settings.clButtonFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 145, 55);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clFormsBack = Color.FromArgb(37, 147, 60);
                    StartApp.settings.clMenuBox = Color.FromArgb(57, 167, 80);
                    StartApp.settings.clMenuHighlight1 = Color.ForestGreen;
                    StartApp.settings.clMenuHighlight2 = Color.DarkGreen;
                    StartApp.settings.clMenuBorder = Color.White;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(37, 123, 60);
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 0, 255);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 255, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(255, 155, 55);
                    StartApp.settings.clHighlight4 = Color.FromArgb(255, 0, 0);
                    StartApp.settings.clLabelFore = Color.FromArgb(255, 255, 23);
                    StartApp.settings.clLabelFore2 = Color.LightBlue;
                    StartApp.settings.clListBoxBack = Color.FromArgb(85, 54, 0);
                    StartApp.settings.clListBoxFore = Color.FromArgb(127, 101, 94);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(37, 189, 60);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(85, 54, 0);
                    StartApp.settings.clTextBoxFore = Color.FromArgb(227, 201, 14);
                    StartApp.settings.clHomeBR = Color.Black;
                    StartApp.settings.clHomeTL = Color.Black;
                    StartApp.settings.clGridLines = false;
                    break;
                case "Pleasant":
                    StartApp.settings.clButtonBack = Color.FromArgb(45, 205, 30);
                    StartApp.settings.clButtonFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 145, 55);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clFormsBack = Color.FromArgb(110, 110, 110);
                    StartApp.settings.clMenuBox = Color.FromArgb(140, 140, 140);
                    StartApp.settings.clMenuHighlight1 = Color.Yellow;
                    StartApp.settings.clMenuHighlight2 = Color.Orange;
                    StartApp.settings.clMenuBorder = Color.Green;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(100, 100, 100);
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(255, 250, 0);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 0, 255);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 255, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(255, 155, 55);
                    StartApp.settings.clHighlight4 = Color.FromArgb(255, 0, 0);
                    StartApp.settings.clLabelFore = Color.FromArgb(160, 200, 240);
                    StartApp.settings.clLabelFore2 = Color.Black;
                    StartApp.settings.clListBoxBack = Color.FromArgb(100, 195, 250);
                    StartApp.settings.clListBoxFore = Color.FromArgb(10, 40, 10);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(84, 103, 117);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(45, 205, 30);
                    StartApp.settings.clTextBoxFore = Color.FromArgb(10, 40, 10);
                    StartApp.settings.clHomeBR = Color.FromArgb(140, 100, 195, 250);
                    StartApp.settings.clHomeTL = Color.FromArgb(100, 195, 250);
                    StartApp.settings.clGridLines = false;
                    break;
                case "Starburst":
                    StartApp.settings.clFormsBack = Color.FromArgb(84, 131, 82);
                    StartApp.settings.clMenuBox = Color.FromArgb(104, 145, 117);
                    StartApp.settings.clMenuHighlight1 = Color.FromArgb(120, Color.Yellow);
                    StartApp.settings.clMenuHighlight2 = Color.CadetBlue;
                    StartApp.settings.clMenuBorder = Color.Orange;
                    StartApp.settings.clListBoxBack = Color.FromArgb(84, 131, 193);
                    StartApp.settings.clListBoxFore = Color.WhiteSmoke;
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(64, 111, 62);
                    StartApp.settings.clGroupBoxFore = Color.Yellow;
                    StartApp.settings.clTextBoxBack = Color.FromArgb(84, 131, 193);
                    StartApp.settings.clTextBoxFore = Color.Black;
                    StartApp.settings.clButtonBack = Color.FromArgb(44, 140, 207);
                    StartApp.settings.clButtonFore = Color.FromArgb(250, 220, 0);
                    StartApp.settings.clLabelFore = Color.WhiteSmoke;
                    StartApp.settings.clLabelFore2 = Color.Black;
                    StartApp.settings.clCheckBoxFore = Color.WhiteSmoke;
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(94, 90, 94);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 145, 55);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clHighlight1 = Color.FromArgb(255, 255, 180);
                    StartApp.settings.clHighlight2 = Color.FromArgb(255, 255, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(200, 200, 0);
                    StartApp.settings.clHighlight4 = Color.FromArgb(50, 50, 0);
                    StartApp.settings.clHomeBR = Color.FromArgb(84, 131, 193);
                    StartApp.settings.clHomeTL = Color.Yellow;
                    StartApp.settings.clGridLines = true;
                    break;
                case "UniGray":
                    StartApp.settings.clFormsBack = Color.FromArgb(120, 120, 120);
                    StartApp.settings.clMenuBox = Color.FromArgb(130, 150, 150);
                    StartApp.settings.clMenuHighlight1 = Color.Silver;
                    StartApp.settings.clMenuHighlight2 = Color.DarkGray;
                    StartApp.settings.clMenuBorder = Color.WhiteSmoke;
                    StartApp.settings.clListBoxBack = Color.FromArgb(85, 85, 85);
                    StartApp.settings.clListBoxFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clGroupBoxBack = Color.FromArgb(85, 85, 85);
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clTextBoxBack = Color.FromArgb(85, 85, 85);
                    StartApp.settings.clTextBoxFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clButtonBack = Color.FromArgb(85, 85, 85);
                    StartApp.settings.clButtonFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clLabelFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clLabelFore2 = Color.FromArgb(185, 185, 185);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(205, 205, 205);
                    StartApp.settings.clRichTextBoxBack = Color.FromArgb(85, 85, 85);
                    StartApp.settings.clChatHeader = Color.FromArgb(255, 145, 55);
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clHighlight1 = Color.White;
                    StartApp.settings.clHighlight2 = Color.LightGray;
                    StartApp.settings.clHighlight3 = Color.FromArgb(97, 149, 88);
                    StartApp.settings.clHighlight4 = Color.FromArgb(0, 0, 0);
                    StartApp.settings.clHomeBR = Color.WhiteSmoke;
                    StartApp.settings.clHomeTL = Color.WhiteSmoke;
                    StartApp.settings.clGridLines = true;
                    break;
                case "WinterFresh":
                    StartApp.settings.clButtonBack = Color.LightGreen;
                    StartApp.settings.clButtonFore = Color.Black;
                    StartApp.settings.clChatHeader = Color.Green;
                    StartApp.settings.clChatPeer = Color.FromArgb(0, 90, 255);
                    StartApp.settings.clChatYou = Color.FromArgb(255, 40, 40);
                    StartApp.settings.clCheckBoxFore = Color.FromArgb(3, 22, 67);
                    StartApp.settings.clFormsBack = Color.WhiteSmoke;
                    StartApp.settings.clMenuBox = Color.ForestGreen;
                    StartApp.settings.clMenuHighlight1 = Color.LightGreen;
                    StartApp.settings.clMenuHighlight2 = Color.DarkGreen;
                    StartApp.settings.clMenuBorder = Color.White;
                    StartApp.settings.clGroupBoxBack = Color.LightGray;
                    StartApp.settings.clGroupBoxFore = Color.FromArgb(3, 22, 67);
                    StartApp.settings.clHighlight1 = Color.FromArgb(0, 250, 0);
                    StartApp.settings.clHighlight2 = Color.FromArgb(0, 180, 0);
                    StartApp.settings.clHighlight3 = Color.FromArgb(0, 110, 0);
                    StartApp.settings.clHighlight4 = Color.FromArgb(0, 40, 0);
                    StartApp.settings.clLabelFore = Color.FromArgb(3, 22, 67);
                    StartApp.settings.clLabelFore2 = Color.Black;
                    StartApp.settings.clListBoxBack = Color.White;
                    StartApp.settings.clListBoxFore = Color.DarkGreen;
                    StartApp.settings.clRichTextBoxBack = Color.White;
                    StartApp.settings.clTextBoxBack = Color.White;
                    StartApp.settings.clTextBoxFore = Color.Green;
                    StartApp.settings.clHomeBR = Color.Green;
                    StartApp.settings.clHomeTL = Color.Green;
                    StartApp.settings.clGridLines = true;
                    break;
            }
        }
    }
}
