// FileObject.cs
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
using System.Runtime.Serialization.Formatters.Binary;

namespace FileScopeCore
{
    /// <summary>
    /// Used in fileList to store all necessary information about each file.
    /// </summary>
    public class FileObject
    {
        //full file path and name
        public string location;

        //store a copy of the filename in lowercase
        public string lcaseFileName;

        //size of file in bytes
        public uint b;

        //file index
        public int fileIndex;

        //md4 hash value for the file
        public byte[] md4;

        //sha1 hash value for the file
        public string sha1;		//in base32
        public byte[] sha1bytes;

        //temporary variable
        public int tempOne;
    }

    /// <summary>
    /// Used in lastFileSet to store all necessary information used to recover
    /// previously generated hash values.
    /// </summary>
    [Serializable]
    public class FileObject2
    {
        public uint bytes;
        public byte[] md4;
        public string sha1;
        public byte[] sha1bytes;
    }
}
