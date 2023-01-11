using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;
using System.Xml.Serialization;
using System.Xml;
using FileScopeCore;

namespace FileScope
{
    //CEH i took this out of Stats and made a new class for GUI settings
    [Serializable]
    public class Settings
    {
        public volatile bool autoGnutella2;					//automatically connect to gnutella2 on startup
        public volatile bool updateNotify;					//notify user of new updates
        public volatile bool switchTransfers;				//switch to transfers page on new download
        public volatile bool closeNormal;					//exit program / send to tray
        public volatile bool minimizeNormal;				//minimize program / send to tray
        public volatile bool alwaysOnTop;					//window always on top
        public volatile bool fileAlert;						//alert when downloading dangerous file types
        public volatile bool clearDl;						//clear completed downloads
        public volatile bool clearUp;						//clear completed uploads
        public volatile bool cancelDLAlert;					//alert when attempting to cancel downloads?
        public volatile int mainWidth;						//width of main window
        public volatile int mainHeight;						//height of main window
        public volatile float transSplitPerc;				//% of the transfers control the splitter is at
        public volatile bool mainMax;						//whether the main window should be maximized
        public volatile string theme;						//current color scheme
        public Color clFormsBack;							//backcolor for forms, tabpages, etc.
        public Color clLabelFore;							//forecolor for labels, linklabels
        public Color clLabelFore2;							//another forecolor that's good with the backcolor
        public Color clButtonBack;							//backcolor for command buttons
        public Color clButtonFore;							//forecolor for command buttons
        public Color clTextBoxBack;							//backcolor for textboxes, combos, updowns
        public Color clTextBoxFore;							//forecolor for textboxes, combos, updowns
        public Color clCheckBoxFore;						//forecolor for checkboxes and radiobuttons
        public Color clGroupBoxBack;						//backcolor for groupboxes
        public Color clGroupBoxFore;						//forecolor for groupboxes
        public Color clListBoxBack;							//backcolor for listboxes, list/tree views
        public Color clListBoxFore;							//forecolor for listboxes, list/tree views
        public Color clRichTextBoxBack;						//backcolor for richtextboxes
        public Color clChatHeader;							//header for chats
        public Color clChatYou;								//color of your text
        public Color clChatPeer;							//color of a peer's text
        public Color clHighlight1;							//general purpose highlight color
        public Color clHighlight2;							//general purpose highlight color
        public Color clHighlight3;							//general purpose highlight color
        public Color clHighlight4;							//general purpose highlight color
        public Color clMenuHighlight1;						//hovering menu color
        public Color clMenuHighlight2;						//selected menu color
        public Color clMenuBox;								//menu box color
        public Color clMenuBorder;							//menu hovering border color
        public Color clHomeTL;								//the color at the top-left of the homepage
        public Color clHomeBR;								//the color of the bottom-right logo in the homepage
        public bool clGridLines;							//show gridlines?
    }


    public class LoadSaveSettings
    {
        public static void LoadSettings()
        {
            if (File.Exists(Utils.GetCurrentPath("settings.fscp")))
            {
                FileStream fStream = new FileStream(Utils.GetCurrentPath("settings.fscp"), FileMode.Open, FileAccess.Read);
                try
                {
                    BinaryFormatter crip = new BinaryFormatter();
                    StartApp.settings = (Settings)crip.Deserialize(fStream);
                    fStream.Close();
                }
                catch
                {
                    fStream.Close();
                    System.Diagnostics.Debug.WriteLine("LoadSettings");
                    //try { File.Delete(Utils.GetCurrentPath("settings.fscp")); }
                    //catch { System.Diagnostics.Debug.WriteLine("Deleting settings.fscp"); }
                    System.Windows.Forms.Application.Exit();
                }
            }
        }

        public static void SaveSettings()
        {
            BinaryFormatter crip = new BinaryFormatter();
            FileStream fStream = new FileStream(Utils.GetCurrentPath("settings.fscp"), FileMode.Create, FileAccess.Write);
            crip.Serialize(fStream, StartApp.settings);
            fStream.Close();
        }
    }
}
