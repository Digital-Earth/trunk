using System;
using System.Collections.Generic;
using System.Text;

namespace FileScopeCore
{
    public interface IGUIBridge
    {
        /// <summary>
        /// Inform GUI of a new incoming QueryHit.
        /// </summary>
        void AddQueryHit(QueryHitObject qho, QueryHitTable qht, ref string searchName);

        void RefreshHomePageNetworks();

        void RefreshHomePageTransfers();

        void RefreshHomePageLibrary();
        /// <summary>
        /// We just initiated a new gnutella connection that's not connected yet.
        /// </summary>

        /// <summary>
        /// We just initiated a connection to a g2 host.
        /// </summary>
        void G2NewConnection(string addr, int sockNum);

        /// <summary>
        /// Handshake is complete and we're now connected to the g2 host.
        /// </summary>
        void G2JustConnected(int sockNum);

        /// <summary>
        /// Update bandwidth and other stuff.
        /// </summary>
        void G2Update(int sckNum, ref string bIn, ref string bOut);

        /// <summary>
        /// We just disconnected from a g2 host.
        /// </summary>
        void G2JustDisconnected(int sockNum);

        /// <summary>
        /// New download; update UI.
        /// </summary>
        void NewDownload(QueryHitTable qht, int dlNum);

        /// <summary>
        /// Update the download information.
        /// </summary>
        void UpdateDownload(ref string status, ref string percent, ref string speed, int dlNum);

        /// <summary>
        /// A download just ended; update UI.
        /// </summary>
        void RemoveDownload(int dlNum);

        /// <summary>
        /// New upload.
        /// </summary>
        void NewUpload(int upNum);

        /// <summary>
        /// Update the UI about this upload.
        /// </summary>
        void UpdateUpload(string filename, uint filesize, string status, string percent, string offsets, string speed, int upNum);

        /// <summary>
        /// Upload just finished.
        /// </summary>
        void RemoveUpload(int upNum, int type);

        /// <summary>
        /// Either an incoming or outgoing chat was created.
        /// </summary>
        void NewChat(int chatNum);

        /// <summary>
        /// An outgoing chat has just completed its handshake.
        /// </summary>
        void ConnectedChat(int chatNum);

        /// <summary>
        /// We've received a chat message from our peer.
        /// </summary>
        void NewChatData(int chatNum, string msg);

        /// <summary>
        /// Either an incoming or outgoing chat was finished.
        /// </summary>
        void DisconnectedChat(int chatNum);

        /// <summary>
        /// Notify gui of incoming G2 search.
        /// </summary>
        void QueryG2(ref string query);

        /// <summary>
        /// There was a change in the shared content, modify library accordingly.
        /// </summary>
        void ChangeShared();

        /// <summary>
        /// New version of FileScope is available.
        /// </summary>
        void NewVersion(string version);

        /// <summary>
        /// Switched mode from ultrapeer to leafnode or vice-versa.
        /// </summary>
        void SwitchedMode();
    }

    public class NullGUIBridge : IGUIBridge
    {
        /// <summary>
        /// Inform GUI of a new incoming QueryHit.
        /// </summary>
        public void AddQueryHit(QueryHitObject qho, QueryHitTable qht, ref string searchName) { }

        public void RefreshHomePageNetworks() { }

        public void RefreshHomePageTransfers() { }

        public void RefreshHomePageLibrary() { }

        /// <summary>
        /// We just initiated a connection to a g2 host.
        /// </summary>
        public void G2NewConnection(string addr, int sockNum) { }

        /// <summary>
        /// Handshake is complete and we're now connected to the g2 host.
        /// </summary>
        public void G2JustConnected(int sockNum) { }

        /// <summary>
        /// Update bandwidth and other stuff.
        /// </summary>
        public void G2Update(int sckNum, ref string bIn, ref string bOut) { }

        /// <summary>
        /// We just disconnected from a g2 host.
        /// </summary>
        public void G2JustDisconnected(int sockNum) { }

        /// <summary>
        /// New download; update UI.
        /// </summary>
        public void NewDownload(QueryHitTable qht, int dlNum) { }

        /// <summary>
        /// Update the download information.
        /// </summary>
        public void UpdateDownload(ref string status, ref string percent, ref string speed, int dlNum) { }

        /// <summary>
        /// A download just ended; update UI.
        /// </summary>
        public void RemoveDownload(int dlNum) { }

        /// <summary>
        /// New upload.
        /// </summary>
        public void NewUpload(int upNum) { }

        /// <summary>
        /// Update the UI about this upload.
        /// </summary>
        public void UpdateUpload(string filename, uint filesize, string status, string percent, string offsets, string speed, int upNum) { }

        /// <summary>
        /// Upload just finished.
        /// </summary>
        public void RemoveUpload(int upNum, int type) { }

        /// <summary>
        /// Either an incoming or outgoing chat was created.
        /// </summary>
        public void NewChat(int chatNum) { }

        /// <summary>
        /// An outgoing chat has just completed its handshake.
        /// </summary>
        public void ConnectedChat(int chatNum) { }

        /// <summary>
        /// We've received a chat message from our peer.
        /// </summary>
        public void NewChatData(int chatNum, string msg) { }

        /// <summary>
        /// Either an incoming or outgoing chat was finished.
        /// </summary>
        public void DisconnectedChat(int chatNum) { }

        /// <summary>
        /// Notify gui of incoming G2 search.
        /// </summary>
        public void QueryG2(ref string query) { }

        /// <summary>
        /// There was a change in the shared content, modify library accordingly.
        /// </summary>
        public void ChangeShared() { }

        /// <summary>
        /// New version of FileScope is available.
        /// </summary>
        public void NewVersion(string version) { }

        /// <summary>
        /// Switched mode from ultrapeer to leafnode or vice-versa.
        /// </summary>
        public void SwitchedMode() { }
    }

}
