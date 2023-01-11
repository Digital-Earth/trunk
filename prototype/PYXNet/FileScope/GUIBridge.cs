// GUIBridge.cs
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
using System.Text;
using System.Windows.Forms;
using FileScopeCore;

namespace FileScope
{
    /// <summary>
    /// Class used as a bridge between the backend core and the frontend gui.
    /// Almost everything is done via Invoke so that the GUI thread takes care of everything on its own.
    /// This class is the ONLY way the core can even touch the gui.
    /// </summary>
    //CEH
    public class GUIBridge : IGUIBridge
    {
        /// <summary>
        /// Inform GUI of a new incoming QueryHit.
        /// </summary>
        public void AddQueryHit(QueryHitObject qho, QueryHitTable qht, ref string searchName)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                //we're trying to find the tabpage for this corresponding search
                foreach (SearchTabPage tabPage in StartApp.main.search.tabControl1.TabPages)
                {
                    if (searchName.Length >= 8 && searchName.Substring(0, 8) == "browse: ")
                    {
                        if (tabPage.Tag.ToString().IndexOf(searchName) != -1)
                        {
                            tabPage.BeginInvoke(new SearchTabPage.addNewItem(tabPage.AddNewItem), new object[] { qho, qht });
                            return;
                        }
                    }
                    else if (tabPage.Tag.ToString() == searchName)
                    {
                        tabPage.BeginInvoke(new SearchTabPage.addNewItem(tabPage.AddNewItem), new object[] { qho, qht });
                        return;
                    }
                }
                System.Diagnostics.Debug.WriteLine("can not find search tab " + searchName);
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge AddQueryHitTable");
            }
        }

        public void RefreshHomePageNetworks()
        {
            if (StartApp.main == null)
                return;
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.homepage.BeginInvoke(new HomePage.resetNetworksText(StartApp.main.homepage.ResetNetworksText));
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge RefreshHomePageNetworks");
            }
        }

        public void RefreshHomePageTransfers()
        {
            if (StartApp.main == null)
                return;
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.homepage.BeginInvoke(new HomePage.resetTransfersText(StartApp.main.homepage.ResetTransfersText));
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge RefreshHomePageTransfers");
            }
        }

        public void RefreshHomePageLibrary()
        {
            if (StartApp.main == null)
                return;
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.homepage.BeginInvoke(new HomePage.resetLibraryText(StartApp.main.homepage.ResetLibraryText));
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge RefreshHomePageLibrary");
            }
        }


        /// <summary>
        /// We just initiated a connection to a g2 host.
        /// </summary>
        public void G2NewConnection(string addr, int sockNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.connection.Invoke(new Connection.g2NewConnection(StartApp.main.connection.G2NewConnection), new object[] { addr, sockNum });
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("guibridge G2NewConnection " + e.Message);
            }
        }

        /// <summary>
        /// Handshake is complete and we're now connected to the g2 host.
        /// </summary>
        public void G2JustConnected(int sockNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.connection.Invoke(new Connection.g2JustConnected(StartApp.main.connection.G2JustConnected), new object[] { sockNum });
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("guibridge G2JustConnected " + e.Message);
            }
        }

        /// <summary>
        /// Update bandwidth and other stuff.
        /// </summary>
        public void G2Update(int sckNum, ref string bIn, ref string bOut)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                if (FileScope.MainDlg.selectedIndexMain != 1)
                    return;
                StartApp.main.connection.BeginInvoke(new Connection.g2Update(StartApp.main.connection.G2Update), new object[] { sckNum, bIn + @" / " + bOut });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge G2Update");
            }
        }

        /// <summary>
        /// We just disconnected from a g2 host.
        /// </summary>
        public void G2JustDisconnected(int sockNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.connection.Invoke(new Connection.g2JustDisconnected(StartApp.main.connection.G2JustDisconnected), new object[] { sockNum });
            }
            catch (System.Threading.ThreadAbortException)
            {
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge G2JustDisconnected");
            }
        }

        /// <summary>
        /// New download; update UI.
        /// </summary>
        public void NewDownload(QueryHitTable qht, int dlNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.newDownload(StartApp.main.transfers.NewDownload), new object[] { qht, dlNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge NewDownload");
            }
        }

        /// <summary>
        /// Update the download information.
        /// </summary>
        public void UpdateDownload(ref string status, ref string percent, ref string speed, int dlNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.updateDownload(StartApp.main.transfers.UpdateDownload), new object[] { status, percent, speed, dlNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge UpdateDownload");
            }
        }

        /// <summary>
        /// A download just ended; update UI.
        /// </summary>
        public void RemoveDownload(int dlNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.removeDownload(StartApp.main.transfers.RemoveDownload), new object[] { dlNum });
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("guibridge RemoveDownload: " + e.Message);
                System.Diagnostics.Debug.WriteLine(e.StackTrace);
            }
        }

        /// <summary>
        /// New upload.
        /// </summary>
        public void NewUpload(int upNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.newUpload(StartApp.main.transfers.NewUpload), new object[] { upNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge NewUpload");
            }
        }

        /// <summary>
        /// Update the UI about this upload.
        /// </summary>
        public void UpdateUpload(string filename, uint filesize, string status, string percent, string offsets, string speed, int upNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.updateUpload(StartApp.main.transfers.UpdateUpload), new object[] { filename, filesize, status, percent, offsets, speed, upNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge UpdateUpload");
            }
        }

        /// <summary>
        /// Upload just finished.
        /// </summary>
        public void RemoveUpload(int upNum, int type)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.transfers.Invoke(new Transfers.removeUpload(StartApp.main.transfers.RemoveUpload), new object[] { upNum, type });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge RemoveUpload");
            }
        }

        /// <summary>
        /// Either an incoming or outgoing chat was created.
        /// </summary>
        public void NewChat(int chatNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.chatpage.Invoke(new ChatPage.newChat(StartApp.main.chatpage.NewChat), new object[] { chatNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge NewChat");
            }
        }

        /// <summary>
        /// An outgoing chat has just completed its handshake.
        /// </summary>
        public void ConnectedChat(int chatNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.chatpage.Invoke(new ChatPage.connectedChat(StartApp.main.chatpage.ConnectedChat), new object[] { chatNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge ConnectedChat");
            }
        }

        /// <summary>
        /// We've received a chat message from our peer.
        /// </summary>
        public void NewChatData(int chatNum, string msg)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.chatpage.Invoke(new ChatPage.newChatData(StartApp.main.chatpage.NewChatData), new object[] { chatNum, msg });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge NewChatData");
            }
        }

        /// <summary>
        /// Either an incoming or outgoing chat was finished.
        /// </summary>
        public void DisconnectedChat(int chatNum)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.chatpage.Invoke(new ChatPage.disconnectedChat(StartApp.main.chatpage.DisconnectedChat), new object[] { chatNum });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge DisconnectedChat");
            }
        }


        /// <summary>
        /// Notify gui of incoming G2 search.
        /// </summary>
        public void QueryG2(ref string query)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                if (FileScope.MainDlg.selectedIndexMain == 1)
                    StartApp.main.connection.BeginInvoke(new Connection.g2Query(StartApp.main.connection.G2Query), new object[] { query });
            }
            catch (System.Threading.ThreadAbortException)
            {
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge QueryG2");
            }
        }


        /// <summary>
        /// There was a change in the shared content, modify library accordingly.
        /// </summary>
        public void ChangeShared()
        {
            if (Configuration.State.closing)
                return;
            if (StartApp.main == null)
                return;
            try
            {
                StartApp.main.library.Invoke(new Library.updateShares(StartApp.main.library.UpdateShares));
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge ChangeShared");
            }
        }

        /// <summary>
        /// New version of FileScope is available.
        /// </summary>
        public void NewVersion(string version)
        {
            if (Configuration.State.closing)
                return;
            try
            {
                //check if there is a new version out
                if (version != Configuration.version && StartApp.settings.updateNotify)
                    StartApp.main.Invoke(new MainDlg.newVersion(StartApp.main.NewVersion), new object[] { version });
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge NewVersion");
            }
        }

        /// <summary>
        /// Switched mode from ultrapeer to leafnode or vice-versa.
        /// </summary>
        public void SwitchedMode()
        {
            if (Configuration.State.closing)
                return;
            try
            {
                StartApp.main.connection.Invoke(new Connection.updateUltrapeerStatus(StartApp.main.connection.UpdateUltrapeerStatus));
            }
            catch
            {
                System.Diagnostics.Debug.WriteLine("guibridge SwitchedMode");
            }
        }
    }

}
