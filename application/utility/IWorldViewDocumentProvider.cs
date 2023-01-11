using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// this interface maybe used by class that would like to open a Document in worldview.
    /// 
    /// this is kind of dependency injection
    /// </summary>
    public interface IWorldViewDocumentProvider
    {
        void OpenDocument(string path);
    }
}
